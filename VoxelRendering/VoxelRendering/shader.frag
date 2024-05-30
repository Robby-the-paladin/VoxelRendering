#version 430 core
#pragma optionNV(unroll none)
#define M_PI 3.1415926535897932384626433832795
#define COLOR_SATURATION 0.0
out vec4 FragColor;

in vec4 gl_FragCoord; // координаты фрагмента

const float ColorSaturation = 0.0;
// Fog stats
// Linear fog decresases lineary from start to cam render distance, is used to cut of not vivsible range
const float LinearFogSize = 250;
// Hyperbolic fog decreases as FogDistacnce / dist ^ FogPower, creates more beautiful result than linear one
const float HyperbolicFogDistacnce = 10;
const float HyperbolicFogPower = 0.5;
const int scenes_number = 2;
const vec3 FogColor = vec3(0,0.5,0.6);
// Sky light
const vec3 LightDir = normalize(vec3(2,1,1));

struct Node {
   int children[8]; // 4 * 8 = 32 byte
   int terminal_empty_texture_using[4]; // 16 byte
   //int texture_nums; // 4 byte
   vec4 color_refl; // 16 byte
};

struct Camera {
    vec2 resolution; // разрешение экрана
    vec3 pos; // положние камеры в координтах мира
    vec3 dir; // вектор направлени€ камеры
    float render_distance; // дальность прорисовки
    float viewing_angle; // угол обзора
    // float tilt_angle; // угол наклона камеры (не используетс€ default 90)
};

uniform sampler2DArray voxelTexture;

layout(std430, binding = 3) buffer tree_buffer
{
    Node tree[];
};

layout(std430, binding = 6) buffer offsets_buffer
{
    int offsets[];
};

layout(std430, binding = 10) buffer borders_buffer
{
    vec4 borders[];
};

layout(std430, binding = 12) buffer grid_buffer
{
    vec4 grid_buf[];
};

uniform Camera cam;

bool belongs(vec3 l, vec3 r, vec3 point) {
    float eps = 0.0001;
    vec3 new_l = vec3(min(l.x, r.x), min(l.y, r.y), min(l.z, r.z));
    vec3 new_r = vec3(max(l.x, r.x), max(l.y, r.y), max(l.z, r.z));
    l = new_l;
    r = new_r;
    return (point.x + eps >= l.x && point.y + eps >= l.y && point.z + eps >= l.z) &&
    (point.x <= r.x + eps && point.y <= r.y + eps && point.z <= r.z + eps);
}

vec3[3] line_plane_intersections(vec3 beg, vec3 end, float x, float y, float z) {
    vec3 p = end - beg;
    if (abs(p.x) < 0.000001 || abs(p.x) < 0.000001 || abs(p.x) < 0.000001) {
        FragColor = vec4(0, 0, 1, 1);
    }
    float ansxfory = (y - beg.y) * p.x / p.y + beg.x;
    float anszfory = (y - beg.y) * p.z / p.y + beg.z;
    float ansxforz = (z - beg.z) * p.x / p.z + beg.x;
    float ansyforz = (z - beg.z) * p.y / p.z + beg.y;
    float ansyforx = (x - beg.x) * p.y / p.x + beg.y;
    float anszforx = (x - beg.x) * p.z / p.x + beg.z; 
    vec3 ansx = vec3(x, ansyforx, anszforx);
    vec3 ansy = vec3(ansxfory, y, anszfory);
    vec3 ansz = vec3(ansxforz, ansyforz, z);
    vec3 ans[3];
    ans[0] = ansx;
    ans[1] = ansy;
    ans[2] = ansz;
    return ans;
}

vec3[5] sort(vec3[5] arr, vec3 sort_point, vec3 beg, vec3 end) {
    int i = 0;
    bool t = true;
    while (t) {
        t = false;
        for (int j = 0; j <= 3 - i; j++) {
            if (belongs(beg, end, arr[j + 1]) && 
            (!belongs(beg, end, arr[j]) || (distance(arr[j], sort_point) > distance(arr[j + 1], sort_point)))) {
                vec3 add = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = add;
                t = true;
            }
        }
        i = i + 1;
    }
    return arr;
}

struct Raycasting_request {
    vec3 beg;
    vec3 end;
    int node_num;
    vec3 l;
    vec3 r;
};

struct Raycasting_response {
    int node_num;
    vec3 point;
};

const int MAX_STACK_SIZE = 32;

vec3 cubic_selection(vec3 beg, vec3 end, int offset_num) {
    vec3 new_beg = end + (end - beg);
    vec3 ap[3] = line_plane_intersections(beg, end, borders[offset_num * 2].x, borders[offset_num * 2].y, borders[offset_num * 2].z);
    for (int i = 0; i < 3; i++) {
        if (belongs(borders[offsets[offset_num] * 2].rgb, borders[offset_num * 2 + 1].rgb, ap[i]) && belongs(beg, end, ap[i])) {
            if (distance(beg, new_beg) > distance(beg, ap[i])) {
                new_beg = ap[i];
            }
        }
    }
    vec3 ap2[3] = line_plane_intersections(beg, end, borders[offset_num * 2 + 1].x, borders[offset_num * 2 + 1].y, borders[offset_num * 2 + 1].z);
    for (int i = 0; i < 3; i++) {
        if (belongs(borders[offsets[offset_num] * 2].rgb, borders[offset_num * 2 + 1].rgb, ap2[i]) && belongs(beg, end, ap2[i])) {
            if (distance(beg, new_beg) > distance(beg, ap2[i])) {
                new_beg = ap2[i];
            }
        }
    }
    if (!belongs(beg, end, new_beg)) {
        return beg;
    }
    return new_beg;
}

struct Raylaunching_response {
    int node_num;
    vec3 point;
    vec3 normal;
    vec4 texture_color;
};

vec3 build_normal(vec3 point, vec3 l, vec3 r) {
    vec3 center = (l + r) / 2;
    vec3 normal = point - center;
    vec3 normal_abs = abs(normal);
    if (normal_abs.x < normal_abs.y || normal_abs.x < normal_abs.z) normal.x = 0;
    if (normal_abs.y < normal_abs.x || normal_abs.y < normal_abs.z) normal.y = 0;
    if (normal_abs.z < normal_abs.x || normal_abs.z < normal_abs.y) normal.z = 0;
    return normalize(normal);
}

vec4 get_texture_color(int node_num, vec3 point, vec3 l, vec3 r, int offset_num) {
    vec4 ans = vec4(0, 0, 0, 0);
    if (tree[node_num + offsets[offset_num]].terminal_empty_texture_using[2] == 0) {
        return ans;
    }
    vec3 center = (l + r) / 2;
    vec3 normal = point - center;
    vec3 normal_abs = abs(normal);
    point -= l;
    ivec3 size = textureSize(voxelTexture, 0);

    int layer = tree[node_num + offsets[offset_num]].terminal_empty_texture_using[2] - 1;
    if (normal_abs.x > normal_abs.y && normal_abs.x > normal_abs.z) {
        ans = texture(voxelTexture, vec3(point.yz, layer));
    }
    if (normal_abs.y > normal_abs.x && normal_abs.y > normal_abs.z) {
        ans = texture(voxelTexture, vec3(point.xz, layer));
    }
    if (normal_abs.z > normal_abs.x && normal_abs.z > normal_abs.y) {
        ans = texture(voxelTexture, vec3(point.xy, layer));
    }
    return ans;
}

vec3 trace_calc(vec3 beg, vec3 end, vec3 point) {
    vec3 delta = normalize(end - beg);
    if (point.x != 0.5)
        return beg + abs(point.x - beg.x) * delta;  //  орректировка позиции вдоль оси x
    if (point.y != 0.5)
        return beg + abs(point.y - beg.y) * delta;  //  орректировка позиции вдоль оси y
    if (point.z != 0.5)
        return beg + abs(point.z - beg.z) * delta;  //  орректировка позиции вдоль оси z
    return end;
}

Raylaunching_response trace_grid(int node_num, int offset, vec3 beg, vec3 end, vec3 grid_size, vec3 grid_beg) {
    beg = beg - grid_beg;
    end = end - grid_beg;
        
    vec3 delta = end - beg;
    vec3 ray_step = sign(delta);
    
    vec3 curx = trace_calc(beg, end, vec3((ray_step.x > 0.0 ? ceil(beg.x) : floor(beg.x)), 0.5, 0.5));
    vec3 cury = trace_calc(beg, end, vec3(0.5, (ray_step.y > 0.0 ? ceil(beg.y) : floor(beg.y)), 0.5));
    vec3 curz = trace_calc(beg, end, vec3(0.5, 0.5, (ray_step.z > 0.0 ? ceil(beg.z) : floor(beg.z))));

    vec3 arr[5] = {curx, cury, curz, beg, end};
    arr = sort(arr, beg, beg, end);

    while (arr[2] != end) {
        if (arr[2] == curx) {
            vec3 coords = floor(curx - grid_beg);
            int grid_ind = int(coords.x + coords.y * grid_size.x + coords.z * grid_size.x * grid_size.y);
            if (coords.x >= 0 && coords.y >= 0 && coords.x >= 0 && grid_buf[grid_ind + offset].a != 0.0) {
                return Raylaunching_response(node_num, curx + grid_beg, build_normal(curx, vec3(0, 0, 0), grid_size), vec4(grid_buf[grid_ind + offset].rgb, 1.0));
            }
            curx = trace_calc(beg, end, vec3(curx.x + 1, 0.5, 0.5));
        }
        if (arr[2] == cury) {
            vec3 coords = floor(cury - grid_beg);
            int grid_ind = int(coords.x + coords.y * grid_size.x + coords.z * grid_size.x * grid_size.y);
            if (coords.x >= 0 && coords.y >= 0 && coords.x >= 0 && grid_buf[grid_ind + offset].a != 0.0) {
                return Raylaunching_response(node_num, cury + grid_beg, build_normal(cury, vec3(0, 0, 0), grid_size), vec4(grid_buf[grid_ind + offset].rgb, 1.0));
            }
            cury = trace_calc(beg, end, vec3(0.5, cury.y + 1, 0.5));
        }
        if (arr[2] == curz) {
            vec3 coords = floor(curz - grid_beg);
            int grid_ind = int(coords.x + coords.y * grid_size.x + coords.z * grid_size.x * grid_size.y);
            if (coords.x >= 0 && coords.y >= 0 && coords.x >= 0 && grid_buf[grid_ind + offset].a != 0.0) {
                return Raylaunching_response(node_num, curz + grid_beg, build_normal(curz, vec3(0, 0, 0), grid_size), vec4(grid_buf[grid_ind + offset].rgb, 1.0));
            }
            curz = trace_calc(beg, end, vec3(0.5, 0.5, curz.z + 1));
        }
        arr = vec3[5](curx, cury, curz, beg, end);
        arr = sort(arr, beg, beg, end);
    }
    return Raylaunching_response(-1, end, vec3(0,0,0), vec4(0, 0, 0, 0));
}

Raylaunching_response raylaunching(vec3 beg, vec3 end, int offset_num) {
    vec3 trve_end = end;

    Raycasting_request raycasting_requests[MAX_STACK_SIZE];
    int top_num = -1;
    top_num++;
    if (!belongs(borders[offsets[offset_num] * 2 ].rgb, borders[offset_num * 2 + 1].rgb, beg)) {
         beg = cubic_selection(beg, end, offset_num);
         if (!belongs(borders[offsets[offset_num] * 2 ].rgb, borders[offset_num * 2 + 1].rgb, beg)) {
            return Raylaunching_response(-1, end, vec3(0,0,0), vec4(0, 0, 0, 0));
         }
    }
    if (!belongs(borders[offsets[offset_num] * 2 ].rgb, borders[offset_num * 2 + 1].rgb, end)) {
         end = cubic_selection(end, beg, offset_num);
         if (!belongs(borders[offsets[offset_num] * 2 ].rgb, borders[offset_num * 2 + 1].rgb, end)) {
            return Raylaunching_response(-1, end, vec3(0,0,0), vec4(0, 0, 0, 0));
         }
    }
    raycasting_requests[top_num] = Raycasting_request(beg, end, 0, borders[offset_num * 2].rgb, borders[offset_num * 2 + 1].rgb);
    while(top_num != -1) {
        Raycasting_request req = raycasting_requests[top_num];
        top_num--;
        beg = req.beg;
        end = req.end;
        int node_num = req.node_num;
        vec3 l = req.l;
        vec3 r = req.r;
        if (!belongs(l, r, beg) || !belongs(l, r, end)) {
            FragColor = vec4(0, 1, 0, 0);
            continue;
        }
        if (tree[node_num + offsets[offset_num]].terminal_empty_texture_using[0] != 0) {
            if (tree[node_num + offsets[offset_num]].terminal_empty_texture_using[3] != -1) {
                return Raylaunching_response(-2, trve_end, vec3(0,0,0), vec4(1, 0, 0, 1));
                int offset = tree[node_num + offsets[offset_num]].terminal_empty_texture_using[3];
                Raylaunching_response res = trace_grid(node_num, offset, beg, end, r - l, l);
                if (res.node_num != -1)
                    return res;
                FragColor = vec4(0, 1, 0, 0);
                continue;
            }
            if (tree[node_num + offsets[offset_num]].terminal_empty_texture_using[1] != 0) { // если пуста€ вершина возращать -1 в node_num
                FragColor = vec4(0, 1, 0, 0);
                continue;
            }
            else { // иначе номер node_num и beg в point
                return Raylaunching_response(node_num, beg, build_normal(beg, l, r), get_texture_color(node_num, beg, l, r, offset_num));
            }
        }
        vec3 ap[3] = line_plane_intersections(beg, end, int((l.x + r.x) / 2), int((l.y + r.y) / 2), int((l.z + r.z) / 2));
        vec3 p[5];
        for (int i = 0; i < 3; i++) {
            p[i] = ap[i];
        }
        p[3] = beg;
        p[4] = end;
        p = sort(p, beg, beg, end);
        for (int t = 4; t > 0; t--) {
            if (belongs(beg, end, p[t])) {
                //FragColor += vec4(0.1, 0.1, 0.1, 1);
                vec3 p1 = p[t];
                vec3 p2 = p[t - 1];
                vec3 mp = (p1 + p2) / 2;
                float eps = 0.001;
                p1 += normalize(mp - p1) * eps;
                p2 += normalize(mp - p2) * eps;
                for (int i = 0; i < 2; i++) {
		            for (int j = 0; j < 2; j++) {
			            for (int k = 0; k < 2; k++) {
                            int new_num = tree[node_num + offsets[offset_num]].children[i * 4 + j * 2 + k];
                            vec3 add = vec3(i * (r.x - l.x) / 2.0, j * (r.y - l.y) / 2.0, k * (r.z - l.z) / 2.0);
					        vec3 newl = l + add;
					        vec3 newr = newl + ((r - l) / 2.0);
                            if (belongs(newl, newr, p1) && belongs(newl, newr, p2)) {
                                if ((tree[new_num  + offsets[offset_num]].terminal_empty_texture_using[0] == 0) || (tree[new_num  + offsets[offset_num]].terminal_empty_texture_using[1] == 0)) {
                                    //FragColor += vec4(0.1, 0.1, 0.1, 1);
                                    top_num++;
                                    if (top_num >= MAX_STACK_SIZE) {
                                        FragColor = vec4(1, 1, 1, 0);
                                        top_num--;
                                        continue;
                                    } else {
                                        raycasting_requests[top_num] = Raycasting_request(p[t-1], p[t], new_num, newl, newr);
                                    }
                                }
                            }
			            }
                    }
		        }
	        }   
        }
    }
    return Raylaunching_response(-1, trve_end, vec3(0,0,0), vec4(0, 0, 0, 0));
}

bool grid(vec3 p) {
    float eps = 0.03;
    int k = 0;
    k += (p.x - floor(p.x + eps / 2) < eps) ? 1 : 0;
    k += (p.y - floor(p.y + eps / 2) < eps) ? 1 : 0;
    k += (p.z - floor(p.z + eps / 2) < eps) ? 1 : 0; 
    return (k >= 2);
}

// Cubic saturation; x from 0 to 1
float saturate(float x) {
    float t = 2.*x-1.;
    return (3.*t-t*t*t)/4. + 0.5;
}

// Post-processing func (color saturation)
vec3 post_proc(vec3 col) {
    return col * (1. - COLOR_SATURATION) + 
        vec3(
            saturate(col.x),
            saturate(col.y),
            saturate(col.z)
        ) * COLOR_SATURATION;
}

void main() {
    vec3 cam_dir = normalize(cam.dir);
    // начало трассируемого отрезка
    vec3 beg = cam.pos; // позици€ камеры

    // конец трассируемого отрезка
    vec2 coords = gl_FragCoord.xy - vec2(0.5, 0.5) * cam.resolution; // положение пиксел€ на экране (центр экрана (0, 0))

    vec3 old_x = normalize(cross(cam.dir, vec3(0, 0, 1))); // единичный вектор Ox дл€ экрана в пространстве (x в старом базисе)
    vec3 old_z = cam_dir; // единичный вектор Oz дл€ экрана в пространстве (z в старом базисе)
    vec3 old_y = normalize(cross(old_x, old_z)); // единичный вектор Oy дл€ экрана в пространстве (y в старом базисе)
    vec3 old_O = cam.pos; // центр кооринат старого —  в новом базисе

    float cam_dist = tan((M_PI  - cam.viewing_angle) / 2.0) * cam.resolution.x / 2; // рассто€ние от наблюдател€ до экрана
    vec3 old_point = vec3(coords, cam_dist); // положение точки в прострастве, через которую пройдЄт луч, в старом базисе

    // точка в прострастве, через которую пройдЄт луч
    vec3 point =  old_x * old_point.xxx + old_y * old_point.yyy + old_z * old_point.zzz + old_O;

    

    vec3 end = normalize(point - beg) * cam.render_distance + beg; // конец трассируемого отрезка с учЄтом дальности прорисовки

    // номер вершины в которую попал луч (или -1) и точка в которую попал луч (или конец отрезка)

//    FragColor = vec4(1, 0, 0, 1);
//    if (debug == 1) {
//        FragColor = vec4(0, 1, 0, 1);
//    }
    FragColor = vec4(0, 0, 0, 1.0);
    Raylaunching_response ans = raylaunching(beg, end, 0);
    int scene = 0;
    for (int i = 1; i < scenes_number; i++) {
        Raylaunching_response scene_ans = raylaunching(beg, ans.point, i);
        if (scene_ans.node_num != -1) {
            ans = scene_ans;
            scene = i;
        }
    }

    if (ans.node_num != -2) {    
        if (ans.node_num != -1) {
            FragColor = vec4(tree[ans.node_num + offsets[scene]].color_refl.xyz, 1.0);
            if (tree[ans.node_num + offsets[scene]].terminal_empty_texture_using[2] != 0) {
                FragColor = ans.texture_color;
            }
        }
                                                                                                                                                                                     
        // Light
        FragColor *= max(0, dot(ans.normal, normalize(cam.pos - ans.point)));
        // Grid rendering
        //    if (grid(ans.point)) {
        //            FragColor = vec4(1, 1, 1, 1.0);
        //    }

        // Fog
        float lin_fog_k = (cam.render_distance - LinearFogSize - distance(cam.pos, ans.point)) / LinearFogSize;
        lin_fog_k = max(0., min(1., lin_fog_k)); // Clip between 0 and 1
        float hyp_fog_k = HyperbolicFogDistacnce / pow(distance(cam.pos, ans.point), HyperbolicFogPower);
        hyp_fog_k = max(0., min(1., hyp_fog_k)); // Clip between 0 and 1
        float fog_k = lin_fog_k * hyp_fog_k;
        FragColor = FragColor * (fog_k) + vec4(FogColor, 1) * (1 - fog_k); // Mix fog & color
    

        // Saturation
        FragColor = vec4(post_proc(FragColor.xyz), 1.);
    }
    else
        FragColor = ans.texture_color;
}
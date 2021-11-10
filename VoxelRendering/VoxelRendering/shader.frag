#version 430 core
out vec4 FragColor;

in vec4 gl_FragCoord; // координаты фрагмента

in vec2 gl_PointCoord; // координаты точки на экране

struct Node {
   int children[8]; // 4 * 8 = 32 byte
   int terminal_empty_align2[4]; // 16 byte
   vec4 color_refl; // 16 byte
};

struct Camera {
    vec2 resolution; // разрешение экрана
    vec3 pos; // положние камеры в координтах мира
    vec3 dir; // вектор направления камеры
    float render_distance; // дальность прорисовки
    float viewing_angle; // угол обзора
    // float tilt_angle; // угол наклона камеры (не используется default 90)
};

layout(std430, binding = 3) buffer tree_buffer
{
    Node tree[];
};
uniform vec3 treer;
uniform vec3 treel;

uniform Camera cam;

bool belongs(vec3 l, vec3 r, vec3 point) {
    float eps = 0.0000001;
    return (point.x + eps >= l.x && point.y + eps >= l.y && point.z + eps >= l.z) &&
    (point.x <= r.x + eps && point.y <= r.y + eps && point.z <= r.z + eps);
}

vec3[3] line_plane_intersections(vec3 beg, vec3 end, float x, float y, float z) {
    vec3 p = end - beg;
    float ansxfory = (y - beg.y) * p.x / p.y + beg.x;
    float ansxforz = (z - beg.z) * p.x / p.z + beg.x;
    float ansyforx = (x - beg.x) * p.y / p.x + beg.y; 
    float ansyforz = (z - beg.z) * p.y / p.z + beg.y;
    float anszforx = (x - beg.x) * p.z / p.x + beg.z; 
    float anszfory = (y - beg.y) * p.z / p.y + beg.z;
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
            if ((belongs(beg, end, arr[j + 1]) && !belongs(beg, end, arr[j]))
            || (distance(arr[j], sort_point) > distance(arr[j + 1], sort_point))) {
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

Raycasting_request raycasting_requests[10];
int top_num = -1;
int debug = 0;

Raycasting_response raycasting(vec3 beg, vec3 end, int node_num, vec3 l, vec3 r) {
    debug++;
    if (!belongs(l, r, beg) || !belongs(l, r, end)) {
        return Raycasting_response(-1, end);
    }
    if (tree[node_num].terminal_empty_align2[0] != 0) {
        if (tree[node_num].terminal_empty_align2[1] != 0) { // если пустая вершина возращать -1 в node_num
            return Raycasting_response(-1, end);
        }
        else { // иначе номер node_num и beg в point
            return Raycasting_response(node_num, beg);
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
            for (int i = 0; i < 2; i++) {
		        for (int j = 0; j < 2; j++) {
			        for (int k = 0; k < 2; k++) {
                        int new_num = tree[node_num].children[i * 4 + j * 2 + k];
                        vec3 add = vec3(i * (r.x - l.x) / 2, j * (r.y - l.y) / 2, k * (r.z - l.z) / 2);
					    vec3 newl = l + add;
					    vec3 newr = ((r + l) / 2) + add;
                        if (belongs(newl, newr, p[t]) && belongs(newl, newr, p[t - 1])) {
                            top_num++;
                            raycasting_requests[top_num] = Raycasting_request(p[t - 1], p[t], new_num, newl, newr);
                        }
			        }
                }
		    }
	    }   
    }
    return Raycasting_response(-1, end);
}

vec3 cubic_selection(vec3 beg, vec3 end) {
    vec3 new_beg = end + (end - beg);
    vec3 ap[3] = line_plane_intersections(beg, end, treel.x, treel.y, treel.z);
    for (int i = 0; i < 3; i++) {
        if (belongs(treel, treer, ap[i]) && belongs(beg, end, ap[i])) {
            if (distance(beg, new_beg) > distance(beg, ap[i])) {
                new_beg = ap[i];
            }
        }
    }
    vec3 ap2[3] = line_plane_intersections(beg, end, treer.x, treer.y, treer.z);
    for (int i = 0; i < 3; i++) {
        if (belongs(treel, treer, ap2[i]) && belongs(beg, end, ap2[i])) {
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

Raycasting_response raylaunching(vec3 beg, vec3 end) {
    top_num++;
    if (!belongs(treel, treer, beg)) {
         beg = cubic_selection(beg, end);
         if (!belongs(treel, treer, beg)) {
            FragColor = vec4(1, 1, 1, 0);
            return Raycasting_response(-1, end);
         }
    }
    if (!belongs(treel, treer, end)) {
         end = cubic_selection(end, beg);
         if (!belongs(treel, treer, end)) {
            FragColor = vec4(0, 0.5, 1, 0);
            return Raycasting_response(-1, end);
         }
    }
    raycasting_requests[top_num] = Raycasting_request(beg, end, 0, treel, treer);
    while(top_num != -1) {
        Raycasting_request req = raycasting_requests[top_num];
        top_num--;
        Raycasting_response ans = raycasting(req.beg, req.end, req.node_num, req.l, req.r);
        if (ans.node_num != -1) {
            return ans;
        }
    }
    return Raycasting_response(-1, end);
}

bool grid(vec3 p) {
    float eps = 0.000001; 
    return (p.x - float(int(p.x + eps / 2)) < eps ||
        p.y - float(int(p.y + eps / 2)) < eps ||
        p.z - float(int(p.z + eps / 2)) < eps); 
}

void main() {
    vec3 cam_dir = normalize(cam.dir);
    // начало трассируемого отрезка
    vec3 beg = cam.pos; // позиция камеры

    // конец трассируемого отрезка
    vec2 coords = gl_FragCoord.xy - vec2(0.5, 0.5) * cam.resolution; // положение пикселя на экране (центр экрана (0, 0))

    vec3 old_x = normalize(cross(cam.dir, vec3(0, 0, 1))); // единичный вектор Ox для экрана в пространстве (x в старом базисе)
    vec3 old_z = cam_dir; // единичный вектор Oz для экрана в пространстве (z в старом базисе)
    vec3 old_y = normalize(cross(old_x, old_z)); // единичный вектор Oy для экрана в пространстве (y в старом базисе)
    vec3 old_O = cam.pos; // центр кооринат старого СК в новом базисе

    float cam_dist = tan(cam.viewing_angle) * cam.resolution.x / 2; // расстояние от наблюдателя до экрана
    vec3 old_point = vec3(coords, cam_dist); // положение точки в прострастве, через которую пройдёт луч, в старом базисе

    // точка в прострастве, через которую пройдёт луч
    vec3 point =  old_x * old_point.xxx + old_y * old_point.yyy + old_z * old_point.zzz + old_O;

    

    vec3 end = normalize(point - beg) * cam.render_distance + beg; // конец трассируемого отрезка с учётом дальности прорисовки

    // номер вершины в которую попал луч (или -1) и точка в которую попал луч (или конец отрезка)
    vec3 raydir = normalize(point - beg);

    FragColor = vec4(abs(raydir), 1);

//    FragColor = vec4(1, 0, 0, 1);
//    if (debug == 1) {
//        FragColor = vec4(0, 1, 0, 1);
//    }
    Raycasting_response ans = raylaunching(beg, end);
    if (ans.node_num != -1) {
       FragColor = vec4(tree[ans.node_num].color_refl.xyz, 1.0);
//       if (grid(ans.point)) {
//            FragColor = vec4(1, 1, 1, 1.0);
//       }
    } else {
       FragColor = vec4(0, 0, 0, 1.0);
    }
}
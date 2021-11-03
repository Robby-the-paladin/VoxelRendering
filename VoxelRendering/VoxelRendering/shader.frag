#version 330 core
out vec4 FragColor;

in vec3 ourColor;

in vec4 gl_FragCoord; // координаты фрагмента

in vec2 gl_PointCoord; // координаты точки на экране

struct Voxel {
    vec3 color;
    float reflection_k;
};

struct Node {
   int children[8];
   bool terminal;
   Voxel voxel;
   
};

uniform Node tree[1000];
uniform vec3 treer;
uniform vec3 treel;

bool belongs(vec3 l, vec3 r, vec3 point) {
    float eps = 0.0000001;
    return !(!(point.x + eps >= l.x && point.y + eps >= l.y && point.z + eps >= l.z) ||
    !(point.x <= r.x + eps && point.y <= r.y + eps && point.z <= r.z + eps));
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

int raycasting(vec3 beg, vec3 end, int node_num, vec3 l, vec3 r) {
    if (tree[node_num].terminal) {
        // если пустая вершина возращать -1 в node_num
        // иначе номер node_num и beg в point
    }
    vec3 ap[3] = line_plane_intersections(beg, end, int((l.x + r.x) / 2), int((l.y + r.y) / 2), int((l.z + r.z) / 2));
    vec3 p[5];
    for (int i = 0; i < 3; i++) {
        p[i] = ap[i];
    }
    p[3] = beg;
    p[4] = end;
    p = sort(p, beg, beg, end);
    for (int t = 0; t < 4; t++) {
        if (belongs(beg, end, p[t + 1])) {
            for (int i = 0; i < 2; i++) {
		        for (int j = 0; j < 2; j++) {
			        for (int k = 0; k < 2; k++) {
                        int new_num = tree[node_num].children[i * 4 + j * 2 + k];
                        vec3 add = vec3(i * (r.x - l.x) / 2, j * (r.y - l.y) / 2, k * (r.z - l.z) / 2);
					    vec3 newl = l + add;
					    vec3 newr = ((r + l) / 2) + add;
			        }
                }
		    }
	    }   
    }
}

void main() {
    vec2 xy = gl_PointCoord.xy;
    vec4 solidRed = vec4(0.4,0.4,0.0,1.0); //Теперь он стал чёрным
    if(xy.x > 0.5){
        solidRed = vec4(ourColor, 1.0f);
    }
    FragColor = solidRed;
}
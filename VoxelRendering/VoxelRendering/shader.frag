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
    return !(!(point.x >= l.x && point.y >= l.y && point.z >= l.z) ||
    !(point.x < r.x && point.y < r.y && point.z < r.z));
}

int raycasting(vec3 beg, vec3 end, int node_num, vec3 l, vec3 r) {
    if (!belongs(l, r, beg) && !belongs(l, r, end) && !belongs(beg, end, l)) {
        return -1;
    }
    if (tree[node_num].terminal) {
        return node_num;
    }
}

void main()
{
    vec2 xy = gl_PointCoord.xy;
    vec4 solidRed = vec4(0.4,0.4,0.0,1.0); //Теперь он стал чёрным
    if(xy.x > 0.5){
        solidRed = vec4(ourColor, 1.0f);
    }
    FragColor = solidRed;
}
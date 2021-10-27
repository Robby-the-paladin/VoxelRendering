#version 330 core
out vec4 FragColor;

in vec3 ourColor;

in vec4 gl_FragCoord; // координаты фрагмента

in vec2 gl_PointCoord; // координаты точки на экране !!WARNING!! X И Y ИНВЕРТИРОВАННЫ

void main()
{
    vec2 xy = gl_PointCoord.xy;
    vec4 solidRed = vec4(0,0.0,0.0,1.0); //Теперь он стал чёрным
    if(xy.x > 0.5){
        solidRed = vec4(ourColor, 1.0f);
    }
    FragColor = solidRed;
}
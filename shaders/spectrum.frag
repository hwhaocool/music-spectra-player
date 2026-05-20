
// 霓虹 Shader

#version 330 core

out vec4 FragColor;

uniform float time;

void main()
{
    vec2 uv = gl_FragCoord.xy / vec2(1280.0, 720.0);

    vec3 col = vec3(0.0);

    col.r = 0.5 + 0.5 * sin(time + uv.x * 8.0);
    col.g = 0.5 + 0.5 * sin(time + uv.y * 8.0);
    col.b = 0.5 + 0.5 * sin(time);

    FragColor = vec4(col, 1.0);
}
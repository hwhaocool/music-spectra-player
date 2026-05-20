#version 330 core

out vec4 FragColor;

uniform float time;

void main()
{
    vec2 uv = gl_FragCoord.xy / vec2(1280.0, 720.0);

    vec3 color = vec3(
        0.5 + 0.5 * sin(time + uv.x * 10.0),
        0.5 + 0.5 * sin(time + uv.y * 10.0),
        0.5 + 0.5 * sin(time)
    );

    FragColor = vec4(color, 1.0);
}
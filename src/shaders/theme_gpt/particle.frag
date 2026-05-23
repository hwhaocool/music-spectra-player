#version 430 core
// gpt

in vec4 vColor;

out vec4 FragColor;

void main()
{
    vec2 uv = gl_PointCoord - 0.5;

    float d = length(uv);

    float alpha = smoothstep(0.5, 0.02, d);

    float glow = exp(-d * d * 18.0);

    vec3 color = vColor.rgb * (1.0 + glow * 2.5);

    FragColor = vec4(color, alpha * vColor.a);
}
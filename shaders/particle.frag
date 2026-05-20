//粒子片段

#version 430 core

in vec4 vColor;
out vec4 FragColor;

void main()
{
    float d     = length(gl_PointCoord - 0.5);
    float alpha = smoothstep(0.5, 0.08, d);
    float glow  = exp(-d * d * 10.0) * 0.6;

    FragColor = vec4(vColor.rgb + glow, vColor.a * alpha);
}

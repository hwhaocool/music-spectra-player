#version 430 core

out vec2 vUV;

// 全屏三角形
void main()
{
    // 3 个顶点覆盖整个屏幕
    vec2 p = vec2(float((gl_VertexID & 1) << 2),
                  float((gl_VertexID & 2) << 1)) - 1.0;
    vUV = (p + 1.0) * 0.5;
    gl_Position = vec4(p, 0.0, 1.0);
}


// 粒子顶点
#version 430 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 2) in float aSize;

uniform mat4 uProjection;

out vec4 vColor;

void main()
{
    gl_Position  = uProjection * vec4(aPos, 0.0, 1.0);
    gl_PointSize = aSize;
    vColor       = aColor;
}

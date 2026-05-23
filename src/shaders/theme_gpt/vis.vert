#version 430 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in float aAngle;
layout(location = 2) in float aMagnitude;

uniform mat4  uProjection;
uniform float uRadius;
uniform float uBarMaxHeight;
uniform float uBarWidth;

uniform float uTime;
uniform float uPulseStrength;

out vec2  vUV;
out float vMagnitude;
out float vNormAngle;

// 可视化顶点 Shader
void main()
{
    vUV        = aPos;
    vMagnitude = aMagnitude;
    vNormAngle = aAngle / (2.0 * 3.14159265);

    // 微弱角度扭曲
    float warp = sin(aAngle * 6.0 + uTime * 2.5) * 0.015 * aMagnitude;

    // 条形在圆上的角度分布
    float ang  = aAngle + (aPos.x - 0.5) * uBarWidth + warp;

    // 脉冲动画
    float pulse = sin(uTime * 4.0 + aAngle * 8.0) * uPulseStrength;
    float height =  aMagnitude *  (1.0 + pulse);

    // 极坐标定位
    float rad  = uRadius + aPos.y * height * uBarMaxHeight;

    //vRadialPos = rad;

    vec2  pos  = vec2(cos(ang), sin(ang)) * rad;

    // 投影输出
    gl_Position = uProjection * vec4(pos, 0.0, 1.0);
}
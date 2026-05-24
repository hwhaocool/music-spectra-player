#version 430 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in float aAngle;
layout(location = 2) in float aMagnitude;

uniform mat4  uProjection;
uniform float uRadius;
uniform float uBarMaxHeight;
uniform float uBarWidth;
uniform float uTime;

out vec2  vUV;
out float vMagnitude;
out float vNormAngle;
out float vIsEar;

float catOutline(float angle)
{
    float PI  = 3.14159265;
    float TAU = 2.0 * PI;

    float r = 1.0;

    // 左耳 ~50°、右耳 ~130°
    float earC1 = 0.87;
    float earC2 = 2.27;
    float earW  = 0.38;
    float earH  = 0.65;

    float d1 = abs(angle - earC1);
    d1 = min(d1, TAU - d1);
    if (d1 < earW) {
        float t = d1 / earW;
        float ear = (1.0 - t) * earH;
        ear *= smoothstep(0.0, 0.22, t);
        r += ear;
    }

    float d2 = abs(angle - earC2);
    d2 = min(d2, TAU - d2);
    if (d2 < earW) {
        float t = d2 / earW;
        float ear = (1.0 - t) * earH;
        ear *= smoothstep(0.0, 0.22, t);
        r += ear;
    }

    // 脸颊两侧略微加宽
    float cheek = cos(angle) * cos(angle);
    r += cheek * 0.07;

    return r;
}

void main()
{
    vUV        = aPos;
    vMagnitude = aMagnitude;
    vNormAngle = aAngle / (2.0 * 3.14159265);

    float ang = aAngle + (aPos.x - 0.5) * uBarWidth;

    float baseRad = uRadius * catOutline(ang);
    float rad     = baseRad + aPos.y * aMagnitude * uBarMaxHeight;

    vec2 pos = vec2(cos(ang), sin(ang)) * rad;

    // 标记耳朵区域，传递给片元着色器
    float PI = 3.14159265;
    float TAU = 2.0 * PI;
    float dL = abs(ang - 0.87);
    dL = min(dL, TAU - dL);
    float dR = abs(ang - 2.27);
    dR = min(dR, TAU - dR);
    vIsEar = (min(dL, dR) < 0.40) ? 1.0 : 0.0;

    gl_Position = uProjection * vec4(pos, 0.0, 1.0);
}

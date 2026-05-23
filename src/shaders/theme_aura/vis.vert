#version 430 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in float aAngle;
layout(location = 2) in float aMagnitude;

uniform mat4  uProjection;
uniform float uRadius;
uniform float uBarMaxHeight;
uniform float uBarWidth;

out vec2  vUV;
out float vMagnitude;
out float vNormAngle;

void main()
{
    vUV        = aPos;
    vMagnitude = aMagnitude;
    vNormAngle = aAngle / (2.0 * 3.14159265);

    float ang  = aAngle + (aPos.x - 0.5) * uBarWidth;
    float rad  = uRadius + aPos.y * aMagnitude * uBarMaxHeight;
    vec2  pos  = vec2(cos(ang), sin(ang)) * rad;

    gl_Position = uProjection * vec4(pos, 0.0, 1.0);
}
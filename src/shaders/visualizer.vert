#version 430 core

// ── 逐顶点 ──
layout(location = 0) in vec2 aPos;        // 单元四边形 (0,0)→(1,1)

// ── 逐实例 ──
layout(location = 1) in float aAngle;     // 弧度
layout(location = 2) in float aMagnitude; // 0‑1

uniform mat4 uProjection;
uniform float uRadius;        // 内圆半径
uniform float uBarMaxHeight;  // 柱体最大延伸
uniform float uBarWidth;      // 角宽度（弧度）

out vec2 vUV;
out float vMagnitude;
out float vNormAngle;         // 0‑1 归一化角度

//  可视化顶点着色器
void main() {
    vUV = aPos;
    vMagnitude = aMagnitude;
    vNormAngle = aAngle / (2.0 * 3.14159265);

    // 把单元四边形映射为径向柱体
    float ang = aAngle + (aPos.x - 0.5) * uBarWidth;
    float rad = uRadius + aPos.y * aMagnitude * uBarMaxHeight;
    vec2 pos = vec2(cos(ang), sin(ang)) * rad;

    gl_Position = uProjection * vec4(pos, 0.0, 1.0);
}

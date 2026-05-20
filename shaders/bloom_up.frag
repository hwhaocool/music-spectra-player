#version 430 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uSource;       // 低分辨率源
uniform sampler2D uDest;         // 当前层（累加目标）
uniform vec2      uTexelSize;    // 1.0 / 源纹理分辨率

//  Bloom 升采样 + 累加
void main()
{
    float x = uTexelSize.x;
    float y = uTexelSize.y;

    // 3×3 Tent 滤波
    vec3 a = texture(uSource, vUV + vec2(-x, -y)).rgb;
    vec3 b = texture(uSource, vUV + vec2( 0, -y)).rgb;
    vec3 c = texture(uSource, vUV + vec2( x, -y)).rgb;
    vec3 d = texture(uSource, vUV + vec2(-x,  0)).rgb;
    vec3 e = texture(uSource, vUV            ).rgb;
    vec3 f = texture(uSource, vUV + vec2( x,  0)).rgb;
    vec3 g = texture(uSource, vUV + vec2(-x,  y)).rgb;
    vec3 h = texture(uSource, vUV + vec2( 0,  y)).rgb;
    vec3 i = texture(uSource, vUV + vec2( x,  y)).rgb;

    vec3 upsampled = (e * 4.0 + (b+d+f+h) * 2.0 + (a+c+g+i)) / 16.0;

    vec3 existing = texture(uDest, vUV).rgb;
    FragColor = vec4(existing + upsampled, 1.0);
}

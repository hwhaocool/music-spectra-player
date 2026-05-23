#version 430 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uScene;
uniform sampler2D uBloom;
uniform float uBloomStrength;

// 最终合成
void main()
{
    vec3 scene = texture(uScene, vUV).rgb;
    vec3 bloom = texture(uBloom, vUV).rgb;

    vec3 color = scene + bloom * uBloomStrength;

    // Reinhard 色调映射
    color = color / (1.0 + color);

    // Gamma 校正
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
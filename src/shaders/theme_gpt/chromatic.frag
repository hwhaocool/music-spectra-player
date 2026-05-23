#version 430 core

in vec2 vUV;

out vec4 FragColor;

uniform sampler2D uTexture;

uniform float uStrength;

// Chromatic Aberration
void main() {
    vec2 center = vec2(0.5);

    vec2 dir = (vUV - center) *
        uStrength;

    float r = texture(uTexture, vUV + dir).r;

    float g = texture(uTexture, vUV).g;

    float b = texture(uTexture, vUV - dir).b;

    FragColor = vec4(r, g, b, 1.0);
}
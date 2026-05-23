#version 430 core

in vec2 vUV;

out vec4 FragColor;

uniform sampler2D uCurrent;
uniform sampler2D uHistory;

uniform float uFeedback;

// 残影混合
void main() {
    vec3 current = texture(uCurrent, vUV).rgb;

    vec3 history = texture(uHistory, vUV).rgb;

    // ========================================================
    // 残影混合
    // ========================================================

    vec3 color = mix(current, history, uFeedback);

    FragColor = vec4(color, 1.0);
}
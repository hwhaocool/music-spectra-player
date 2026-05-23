#pragma once
#include "shader.h"
#include <glad/glad.h>

class AfterImage {
public:
    bool init(int w, int h);
    void resize(int w, int h);
    // 将当前帧与历史帧混合，返回混合后的纹理 ID
    GLuint apply(GLuint currentTex, float feedback);
    void destroy();

    GLuint outputTexture() const { return tex_[ping_]; }
    GLuint outputFBO()     const { return fbo_[ping_]; }

    float& feedback() { return feedback_; }

private:
    Shader shader_;
    GLuint fbo_[2]    = {0, 0};
    GLuint tex_[2]    = {0, 0};
    GLuint quadVAO_   = 0;
    int    w_ = 0, h_ = 0;
    int    ping_       = 0;   // 当前输出是 tex_[ping_]，另一个作 history 写入目标
    bool   firstFrame_ = true;
    float  feedback_   = 0.85f;
};

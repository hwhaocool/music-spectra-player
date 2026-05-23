#include "after_image.h"
#include "shaders_embed.h"
#include <iostream>

bool AfterImage::init(int w, int h)
{
    w_ = w; h_ = h;

    if (!shader_.loadFromMemory(kFullscreenVert, kAfterImageFrag))
        return false;

    // 创建 ping-pong 纹理和 FBO
    for (int i = 0; i < 2; ++i) {
        glGenFramebuffers(1, &fbo_[i]);
        glGenTextures(1, &tex_[i]);

        glBindTexture(GL_TEXTURE_2D, tex_[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo_[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_[i], 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "[AfterImage] FBO " << i << " incomplete\n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 全屏四边形
    glGenVertexArrays(1, &quadVAO_);
    glBindVertexArray(quadVAO_);
    glBindVertexArray(0);

    firstFrame_ = true;
    return true;
}

void AfterImage::resize(int w, int h)
{
    w_ = w; h_ = h;
    firstFrame_ = true;

    for (int i = 0; i < 2; ++i) {
        glBindTexture(GL_TEXTURE_2D, tex_[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
    }
}

GLuint AfterImage::apply(GLuint currentTex, float feedback)
{
    int pong = 1 - ping_;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_[pong]);
    glViewport(0, 0, w_, h_);
    glClear(GL_COLOR_BUFFER_BIT);

    shader_.use();
    shader_.setFloat("uFeedback", firstFrame_ ? 0.f : feedback);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, currentTex);
    shader_.setInt("uCurrent", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex_[ping_]);
    shader_.setInt("uHistory", 1);

    glBindVertexArray(quadVAO_);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ping_       = pong;
    firstFrame_ = false;
    return tex_[ping_];
}

void AfterImage::destroy()
{
    shader_.destroy();
    for (int i = 0; i < 2; ++i) {
        if (fbo_[i]) glDeleteFramebuffers(1, &fbo_[i]);
        if (tex_[i]) glDeleteTextures(1, &tex_[i]);
        fbo_[i] = tex_[i] = 0;
    }
    if (quadVAO_) glDeleteVertexArrays(1, &quadVAO_);
    quadVAO_ = 0;
}

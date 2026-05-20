#include "bloom.h"
#include <iostream>

bool Bloom::init(int w, int h, int mipLevels)
{
    sceneW_ = w; sceneH_ = h;

    if (!downShader_.loadFromFile("shaders/fullscreen.vert", "shaders/bloom_down.frag"))
        return false;
    if (!upShader_.loadFromFile("shaders/fullscreen.vert", "shaders/bloom_up.frag"))
        return false;
    if (!compositeShader_.loadFromFile("shaders/fullscreen.vert", "shaders/composite.frag"))
        return false;

    // 场景 FBO
    glGenFramebuffers(1, &sceneFBO_);
    glGenTextures(1, &sceneTex_);
    glBindTexture(GL_TEXTURE_2D, sceneTex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneTex_, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "[Bloom] Scene FBO incomplete\n";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    createMipChain(w, h, mipLevels);
    createQuad();
    return true;
}

void Bloom::resize(int w, int h)
{
    sceneW_ = w; sceneH_ = h;
    glBindTexture(GL_TEXTURE_2D, sceneTex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
    int levels = (int)mipChain_.size();
    destroyMipChain();
    createMipChain(w, h, levels);
}

void Bloom::createMipChain(int w, int h, int levels)
{
    mipChain_.resize(levels);
    for (int i = 0; i < levels; ++i) {
        w = std::max(1, w / 2);
        h = std::max(1, h / 2);
        mipChain_[i].w = w;
        mipChain_[i].h = h;

        glGenFramebuffers(1, &mipChain_[i].fbo);
        glGenTextures(1, &mipChain_[i].tex);
        glBindTexture(GL_TEXTURE_2D, mipChain_[i].tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindFramebuffer(GL_FRAMEBUFFER, mipChain_[i].fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, mipChain_[i].tex, 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Bloom::destroyMipChain()
{
    for (auto& m : mipChain_) {
        if (m.fbo) glDeleteFramebuffers(1, &m.fbo);
        if (m.tex) glDeleteTextures(1, &m.tex);
        m.fbo = m.tex = 0;
    }
}

void Bloom::createQuad()
{
    glGenVertexArrays(1, &quadVAO_);
    // 使用 gl_VertexID 的全屏三角形，不需要 VBO
    glBindVertexArray(quadVAO_);
    glBindVertexArray(0);
}

GLuint Bloom::apply(GLuint srcTex, float strength)
{
    glDisable(GL_DEPTH_TEST);

    int levels = (int)mipChain_.size();
    if (levels == 0) return srcTex;

    // ── 降采样 pass ──
    downShader_.use();
    GLuint prevTex = srcTex;
    int pw = sceneW_, ph = sceneH_;
    for (int i = 0; i < levels; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, mipChain_[i].fbo);
        glViewport(0, 0, mipChain_[i].w, mipChain_[i].h);
        glClear(GL_COLOR_BUFFER_BIT);
        downShader_.setVec2("uTexelSize", 1.f / pw, 1.f / ph);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, prevTex);
        downShader_.setInt("uTexture", 0);
        glBindVertexArray(quadVAO_);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        prevTex = mipChain_[i].tex;
        pw = mipChain_[i].w;
        ph = mipChain_[i].h;
    }

    // ── 升采样 pass（累加到 mip 0）──
    upShader_.use();
    for (int i = levels - 1; i > 0; --i) {
        GLuint dstTex = mipChain_[i - 1].tex;
        GLuint dstFBO = mipChain_[i - 1].fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, dstFBO);
        glViewport(0, 0, mipChain_[i - 1].w, mipChain_[i - 1].h);
        // 注意：不做 glClear，因为是累加模式
        upShader_.setVec2("uTexelSize",
                          1.f / mipChain_[i].w, 1.f / mipChain_[i].h);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mipChain_[i].tex);
        upShader_.setInt("uSource", 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, dstTex);
        upShader_.setInt("uDest", 1);
        glBindVertexArray(quadVAO_);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    // ── 最终合成 ──
    // 写入 mipChain_[0] 对应的 FBO
    glBindFramebuffer(GL_FRAMEBUFFER, mipChain_[0].fbo);
    glViewport(0, 0, mipChain_[0].w, mipChain_[0].h);
    glClear(GL_COLOR_BUFFER_BIT);
    compositeShader_.use();
    compositeShader_.setFloat("uBloomStrength", strength);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTex);
    compositeShader_.setInt("uScene", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mipChain_[0].tex);
    compositeShader_.setInt("uBloom", 1);
    glBindVertexArray(quadVAO_);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return mipChain_[0].tex;
}

void Bloom::destroy()
{
    downShader_.destroy();
    upShader_.destroy();
    compositeShader_.destroy();
    destroyMipChain();
    if (sceneFBO_)  glDeleteFramebuffers(1, &sceneFBO_);
    if (sceneTex_)  glDeleteTextures(1, &sceneTex_);
    if (quadVAO_)   glDeleteVertexArrays(1, &quadVAO_);
    sceneFBO_ = sceneTex_ = quadVAO_ = 0;
}

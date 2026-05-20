#include "bloom.h"
#include <iostream>

// ═══════════════════════════════════════════════════════════
//  本次修复的三个核心问题：
//  1. 合成 pass glClear 清除了自身的 bloom 输入纹理 → 新增全分辨率 compositeOut FBO
//  2. 上采样对 mipChain_[0] 读写冲突（未定义行为） → 改用加法混合累加，不再读取目标纹理
//  3. 合成输出半分辨率导致 blit 2x 拉伸       → compositeOut 与 scene 同尺寸
// ═══════════════════════════════════════════════════════════

bool Bloom::init(int w, int h, int mipLevels)
{
    sceneW_ = w; sceneH_ = h;

    // ── 加载着色器 ──
    if (!downShader_.loadFromFile("shaders/fullscreen.vert", "shaders/bloom_down.frag"))
        return false;
    if (!upShader_.loadFromFile("shaders/fullscreen.vert", "shaders/bloom_up.frag"))
        return false;
    if (!compositeShader_.loadFromFile("shaders/fullscreen.vert", "shaders/composite.frag"))
        return false;

    // ── 场景 FBO（全分辨率）──
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

    // ── 【新增】合成输出 FBO（全分辨率，与 scene 同尺寸）
    //    解决：原代码合成 pass 写入 mipChain_[0] 后又从 mipChain_[0] 采样 → 数据被清零
    //    现在合成写入独立的 compositeOutFBO，blit 源也是全分辨率 ──
    glGenFramebuffers(1, &compositeOutFBO_);
    glGenTextures(1, &compositeOutTex_);
    glBindTexture(GL_TEXTURE_2D, compositeOutTex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, compositeOutFBO_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, compositeOutTex_, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "[Bloom] Composite output FBO incomplete\n";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    compositeOutW_ = w;
    compositeOutH_ = h;

    createMipChain(w, h, mipLevels);
    createQuad();
    return true;
}

void Bloom::resize(int w, int h)
{
    sceneW_ = w; sceneH_ = h;

    // 重建场景纹理
    glBindTexture(GL_TEXTURE_2D, sceneTex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);

    // 重建合成输出纹理（与 scene 保持同尺寸）
    glBindTexture(GL_TEXTURE_2D, compositeOutTex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
    compositeOutW_ = w;
    compositeOutH_ = h;

    // 重建 mip 链
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
    glBindVertexArray(quadVAO_);
    glBindVertexArray(0);
}

GLuint Bloom::apply(GLuint srcTex, float strength)
{
    glDisable(GL_DEPTH_TEST);

    int levels = (int)mipChain_.size();
    if (levels == 0) return srcTex;

    // ============================================================
    //  1. 降采样 pass（与原代码相同）
    //     scene → mip[0] → mip[1] → ... → mip[N]
    //     每级 clear 后写入，无冲突
    // ============================================================
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

    // ============================================================
    //  2. 升采样 pass（使用加法混合，避免读写同一纹理）
    //
    //     【原代码问题】i=1 时：
    //       写入 mipChain_[0].fbo（颜色附件 = mipChain_[0].tex）
    //       同时从 mipChain_[0].tex 采样 → OpenGL 未定义行为
    //
    //     【修复】改用加法混合 (GL_ONE, GL_ONE)：
    //       - 不清除目标（保留降采样阶段写入的数据）
    //       - 只从 mip[i]（较小级别）采样，不读取目标纹理
    //       - 混合自动将升采样结果叠加到目标上
    //
    //     ⚠ bloom_up.frag 需同步修改，见下方说明
    // ============================================================
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);       // 加法混合：dst = dst + src
    upShader_.use();
    for (int i = levels - 1; i > 0; --i) {
        glBindFramebuffer(GL_FRAMEBUFFER, mipChain_[i - 1].fbo);
        glViewport(0, 0, mipChain_[i - 1].w, mipChain_[i - 1].h);
        // 不做 glClear —— 保留已有的降采样数据，通过混合累加
        upShader_.setVec2("uTexelSize",
                          1.f / mipChain_[i].w, 1.f / mipChain_[i].h);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mipChain_[i].tex);   // 只采样较小级别
        upShader_.setInt("uSource", 0);
        // 不再绑定 uDest，不再从目标纹理采样
        glBindVertexArray(quadVAO_);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    glDisable(GL_BLEND);

    // ============================================================
    //  3. 最终合成（写入独立的全分辨率 compositeOut FBO）
    //
    //     【原代码问题】写入 mipChain_[0].fbo 后，
    //       glClear 把 mipChain_[0].tex 清零，
    //       紧接着从 mipChain_[0].tex 采样作为 bloom 输入 → 全黑
    //
    //     【修复】合成目标改为 compositeOutFBO_（全分辨率），
    //       从 mipChain_[0].tex 采样 bloom（未被清除），
    //       从 srcTex 采样原场景，合成到独立 FBO。
    //       blit 时源和目标同尺寸，无需拉伸。
    // ============================================================
    glBindFramebuffer(GL_FRAMEBUFFER, compositeOutFBO_);
    glViewport(0, 0, compositeOutW_, compositeOutH_);
    glClear(GL_COLOR_BUFFER_BIT);
    compositeShader_.use();
    compositeShader_.setFloat("uBloomStrength", strength);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTex);               // 原场景
    compositeShader_.setInt("uScene", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mipChain_[0].tex);     // bloom 纹理（完好）
    compositeShader_.setInt("uBloom", 1);
    glBindVertexArray(quadVAO_);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return compositeOutTex_;
}

void Bloom::destroy()
{
    downShader_.destroy();
    upShader_.destroy();
    compositeShader_.destroy();
    destroyMipChain();
    if (sceneFBO_)        glDeleteFramebuffers(1, &sceneFBO_);
    if (sceneTex_)        glDeleteTextures(1, &sceneTex_);
    if (compositeOutFBO_) glDeleteFramebuffers(1, &compositeOutFBO_);
    if (compositeOutTex_) glDeleteTextures(1, &compositeOutTex_);
    if (quadVAO_)         glDeleteVertexArrays(1, &quadVAO_);
    sceneFBO_ = sceneTex_ = compositeOutFBO_ = compositeOutTex_ = quadVAO_ = 0;
}
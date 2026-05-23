#include "vis.h"
#include "audio_engine.h"
#include "math_utils.h"
#include "themes.h"
#include <cmath>
#include <cstring>

bool Visualizer::init(int fftSize, int numBars)
{
    numBars_ = numBars;

    // 柱宽 = 72% 间距
    barWidthRad_ = (2.f * PI_F) / numBars_ * 0.72f; 
    instances_.resize(numBars_);
    smoothed_.resize(numBars_, 0.f);
    fft_ = new FFTProcessor(fftSize, numBars);

    // ── 编译着色器（默认主题） ──
    if (!setTheme(0)) return false;

    // ── 单元四边形（2 个三角形）──
    float quad[] = {
        0,0,  1,0,  1,1,
        0,0,  1,1,  0,1
    };
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vboQuad_);
    glGenBuffers(1, &vboInstance_);

    glBindVertexArray(vao_);

    // layout 0 : quad pos
    glBindBuffer(GL_ARRAY_BUFFER, vboQuad_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    // layout 1,2 : instance buffer (angle, magnitude)
    glBindBuffer(GL_ARRAY_BUFFER, vboInstance_);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(BarInstance) * numBars_, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE,
                          sizeof(BarInstance),
                          (void*)offsetof(BarInstance, angle));
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE,
                          sizeof(BarInstance),
                          (void*)offsetof(BarInstance, magnitude));
    glVertexAttribDivisor(2, 1);

    glBindVertexArray(0);
    return true;
}

bool Visualizer::setTheme(int index)
{
    if (index < 0 || index >= kThemeCount) return false;
    currentTheme_ = index;
    return shader_.loadFromMemory(kThemes[index].vert, kThemes[index].frag);
}

void Visualizer::update(float dt, AudioEngine& audio)
{
    // 从 PCM 环形缓冲读取最新样本
    constexpr int READ = 1024;
    static float tmp[READ];
    audio.pcmRing().readLatest(tmp, READ);
    fft_->process(tmp, READ);

    const float* spec = fft_->getSpectrum();
    for (int i = 0; i < numBars_; ++i) {
        smoothed_[i] = lerpf(smoothed_[i], spec[i],
                             clampf(1.f - smoothing_, 0.05f, 1.f));
        instances_[i].angle     = (2.f * PI_F * i) / numBars_;
        instances_[i].magnitude = smoothed_[i];
    }

    // 上传实例数据
    glBindBuffer(GL_ARRAY_BUFFER, vboInstance_);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    sizeof(BarInstance) * numBars_, instances_.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Visualizer::draw(const float* projMatrix, float cx, float cy,
                      float radius, float barMaxHeight, float time)
{
    shader_.use();
    shader_.setMat4("uProjection", projMatrix);
    shader_.setFloat("uRadius",        radius);
    shader_.setFloat("uBarMaxHeight",  barMaxHeight);
    shader_.setFloat("uBarWidth",      barWidthRad_);
    shader_.setFloat("uTime",          time);

    glBindVertexArray(vao_);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, numBars_);
    glBindVertexArray(0);
}

void Visualizer::destroy()
{
    shader_.destroy();
    if (vao_)        glDeleteVertexArrays(1, &vao_);
    if (vboQuad_)    glDeleteBuffers(1, &vboQuad_);
    if (vboInstance_)glDeleteBuffers(1, &vboInstance_);
    delete fft_; fft_ = nullptr;
    vao_ = vboQuad_ = vboInstance_ = 0;
}
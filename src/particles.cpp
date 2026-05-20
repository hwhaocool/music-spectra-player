#include "particles.h"
#include "audio_engine.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

static float randf() { return (float)std::rand() / RAND_MAX; }

bool ParticleSystem::init(int maxP)
{
    maxP_ = maxP;
    particles_.resize(maxP);
    vertices_.resize(maxP);

    if (!shader_.loadFromFile("shaders/particle.vert", "shaders/particle.frag"))
        return false;

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * maxP, nullptr, GL_DYNAMIC_DRAW);

    // x, y
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, x));
    // rgba
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, r));
    // size
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, size));

    glBindVertexArray(0);
    return true;
}

void ParticleSystem::spawn(float cx, float cy, float radius, float energy)
{
    int count = (int)(energy * 8.f);
    for (int i = 0; i < count; ++i) {
        // 找到空闲槽位
        int idx = -1;
        for (int j = 0; j < maxP_; ++j)
            if (particles_[j].life <= 0.f) { idx = j; break; }
        if (idx < 0) break;

        auto& p = particles_[idx];
        float ang = randf() * 2.f * PI_F;
        float r   = radius + randf() * 20.f;
        p.pos     = Vec2(cx + cosf(ang) * r, cy + sinf(ang) * r);
        p.vel     = Vec2(cosf(ang) * (40.f + randf() * 60.f),
                         sinf(ang) * (40.f + randf() * 60.f));
        p.life    = 0.8f + randf() * 1.2f;
        p.maxLife = p.life;
        p.size    = 2.f + randf() * 4.f;

        // 霓虹色：cyan ↔ magenta 随机
        float hue = 0.5f + randf() * 0.4f;
        float s = 0.85f, v = 1.f;
        // HSV → RGB (simplified for hue 0.5‑0.9 range)
        float c = v * s;
        float x2 = c * (1.f - fabsf(fmodf(hue * 6.f, 2.f) - 1.f));
        float m = v - c;
        if      (hue < 1.f/6.f) { p.color = {c+m, x2+m, m, 1.f}; }
        else if (hue < 2.f/6.f) { p.color = {x2+m, c+m, m, 1.f}; }
        else if (hue < 3.f/6.f) { p.color = {m, c+m, x2+m, 1.f}; }
        else if (hue < 4.f/6.f) { p.color = {m, x2+m, c+m, 1.f}; }
        else if (hue < 5.f/6.f) { p.color = {c+m, m, x2+m, 1.f}; }
        else                     { p.color = {x2+m, m, c+m, 1.f}; }
    }
}

void ParticleSystem::update(float dt, float cx, float cy,
                            float ringRadius, AudioEngine& audio)
{
    // 估算当前能量（均值 magnitude）
    constexpr int N = 256;
    static float buf[N];
    audio.pcmRing().readLatest(buf, N);
    float energy = 0.f;
    for (int i = 0; i < N; ++i) energy += buf[i] * buf[i];
    energy = sqrtf(energy / N);

    // 能量超过阈值时生成粒子
    if (energy > 0.15f) spawn(cx, cy, ringRadius, energy);

    // 更新
    alive_ = 0;
    for (auto& p : particles_) {
        if (p.life <= 0.f) continue;
        p.life -= dt;
        p.pos += p.vel * dt;
        p.vel = p.vel * (1.f - 1.5f * dt); // 阻尼

        float t = clampf(p.life / p.maxLife, 0.f, 1.f);
        p.color.w = t;  // 渐隐
        p.size *= (1.f - 0.5f * dt);

        if (p.life > 0.f) {
            auto& v = vertices_[alive_];
            v.x = p.pos.x; v.y = p.pos.y;
            v.r = p.color.x; v.g = p.color.y;
            v.b = p.color.z; v.a = p.color.w;
            v.size = p.size;
            ++alive_;
        }
    }
}

void ParticleSystem::draw(const float* projMatrix)
{
    if (alive_ == 0) return;

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // 叠加混合 = 发光
    glDepthMask(GL_FALSE);

    shader_.use();
    shader_.setMat4("uProjection", projMatrix);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * alive_, vertices_.data());
    glDrawArrays(GL_POINTS, 0, alive_);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ParticleSystem::destroy()
{
    shader_.destroy();
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    vao_ = vbo_ = 0;
}

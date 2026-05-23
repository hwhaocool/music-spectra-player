#pragma once
#include "shader.h"
#include "math_utils.h"
#include <glad/glad.h>
#include <vector>

class AudioEngine;

struct Particle {
    Vec2  pos;
    Vec4  color;
    float size;
    float life;     // 0 → dead
    float maxLife;
    Vec2  vel;
};

class ParticleSystem {
public:
    ParticleSystem() = default;
    bool init(int maxParticles = 2000);
    bool setShaders(const char* vert, const char* frag);
    void update(float dt, float centerX, float centerY,
                float ringRadius, AudioEngine& audio);
    void draw(const float* projMatrix);
    void destroy();

private:
    int maxP_;
    Shader shader_;
    GLuint vao_ = 0, vbo_ = 0;
    std::vector<Particle> particles_;
    int alive_ = 0;

    void spawn(float cx, float cy, float radius, float energy);
    struct Vertex {
        float x, y;
        float r, g, b, a;
        float size;
    };
    std::vector<Vertex> vertices_;
};

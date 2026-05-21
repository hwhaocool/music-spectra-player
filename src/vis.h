#pragma once
#include "shader.h"
#include "fft.h"
#include <glad/glad.h>
#include <vector>

class AudioEngine;

struct BarInstance {
    float angle;
    float magnitude;
};

class Visualizer {
public:
    Visualizer() = default;
    bool init(int fftSize = 1024, int numBars = 128);
    void update(float dt, AudioEngine& audio);
    
    void draw(const float* projMatrix, float cx, float cy,
              float radius, float barMaxHeight, float time);
    void destroy();

    int   numBars()     const { return numBars_; }
    float radius()      const { return radius_; }
    float barMaxH()     const { return barMaxHeight_; }
    float barWidth()    const { return barWidthRad_; }

    // 外部可调参数
    float radius_      = 140.f;
    float barMaxHeight_= 100.f;
    float smoothing_   = 0.15f;

private:
    // 柱子的数量
    int numBars_ = 128;

    float barWidthRad_ = 0.f;

    Shader shader_;
    GLuint vao_ = 0, vboQuad_ = 0, vboInstance_ = 0;
    std::vector<BarInstance> instances_;
    std::vector<float>       smoothed_;
    FFTProcessor*            fft_ = nullptr;
};
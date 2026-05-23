#pragma once
#include "shader.h"
#include <glad/glad.h>

class VortexOverlay {
public:
    bool init();
    void draw(float time, float ndcRadiusX, float ndcRadiusY);
    void destroy();

private:
    Shader shader_;
    GLuint vao_ = 0;
};

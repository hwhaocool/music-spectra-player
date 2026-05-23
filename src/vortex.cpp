#include "vortex.h"
#include "shaders_embed.h"

bool VortexOverlay::init()
{
    if (!shader_.loadFromMemory(kFullscreenVert, kCenterVortexFrag))
        return false;

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    glBindVertexArray(0);
    return true;
}

void VortexOverlay::draw(float time, float ndcRadiusX, float ndcRadiusY)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader_.use();
    shader_.setFloat("uTime", time);
    shader_.setVec2("uInnerRadius", ndcRadiusX, ndcRadiusY);

    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
}

void VortexOverlay::destroy()
{
    shader_.destroy();
    if (vao_) glDeleteVertexArrays(1, &vao_);
    vao_ = 0;
}

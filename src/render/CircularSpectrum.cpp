#include "CircularSpectrum.h"

#include <glad/glad.h>
#include <cmath>

// 圆形频谱渲染
void CircularSpectrum::render(
    const std::vector<float>& spectrum)
{
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glLineWidth(2.5f);

    glBegin(GL_LINES);

    float radius = 0.35f;

    int count = (int)spectrum.size();

    for (int i = 0; i < count; i++)
    {
        float angle =
            (float)i / count * 6.2831853f;

        float x = cos(angle);
        float y = sin(angle);

        float power = spectrum[i] * 0.0035f;

        float innerX = x * radius;
        float innerY = y * radius;

        float outerX = x * (radius + power);
        float outerY = y * (radius + power);

        float r =
            0.5f + 0.5f * sin(angle);

        float g =
            0.5f + 0.5f * sin(angle + 2.0f);

        float b =
            0.5f + 0.5f * sin(angle + 4.0f);

        glColor4f(r, g, b, 0.9f);

        glVertex2f(innerX, innerY);
        glVertex2f(outerX, outerY);
    }

    glEnd();
}
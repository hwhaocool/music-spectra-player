#pragma once
#include <vector>
#include "shader.h"
#include <glad/glad.h>

class Bloom {
public:
    Bloom() = default;
    bool init(int sceneW, int sceneH, int mipLevels = 5);
    void resize(int w, int h);
    // 将 sceneTex 通过 bloom 滤镜写入内部 FBO，返回合成后的纹理 ID
    GLuint apply(GLuint sceneTex, float strength);
    void destroy();

    GLuint compositeTexture() const { return compositeOutTex_; }

    GLuint sceneFBO()        const { return sceneFBO_; }
    GLuint sceneTexture()    const { return sceneTex_; }
    GLuint compositeFBO()    const { return compositeOutFBO_; }

    void compositeSize(int& w, int& h) const {
        w = compositeOutW_; h = compositeOutH_;
    }


private:
    struct Mip {
        GLuint fbo = 0, tex = 0;
        int w = 0, h = 0;
    };

    Shader downShader_, upShader_, compositeShader_;
    GLuint sceneFBO_ = 0, sceneTex_ = 0;
    int sceneW_ = 0, sceneH_ = 0;
    std::vector<Mip> mipChain_;
    GLuint quadVAO_ = 0;


    GLuint compositeOutFBO_ = 0, compositeOutTex_ = 0;
    int compositeOutW_ = 0, compositeOutH_ = 0;

    void createMipChain(int w, int h, int levels);
    void destroyMipChain();
    void createCompositeTarget(int w, int h);
    void destroyCompositeTarget();
    void createQuad();
    void createCompositeOutput(int w, int h);
};

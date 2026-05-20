#pragma once
#include "audio_engine.h"
#include "visualizer.h"
#include "bloom.h"
#include "particles.h"
#include "playlist.h"
#include "ui.h"
#include "math_utils.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>

struct GLFWwindow;

class App {
public:
    App() = default;
    bool init(int w = 1280, int h = 720);
    void run();
    void shutdown();

    // 子系统访问器（供 UI 调用）
    AudioEngine&  audio()     { return audio_; }
    Visualizer&   vis()       { return vis_; }
    Playlist&     playlist()  { return playlist_; }
    float&        bloomStrength() { return bloomStr_; }

    void playTrack(int index);

    // 文件拖拽
    static void dropCallback(GLFWwindow* win, int count, const char** paths);

private:
    void processDrop();

    int winW_ = 1280, winH_ = 720;
    GLFWwindow* window_ = nullptr;

    AudioEngine  audio_;
    Visualizer   vis_;
    Bloom        bloom_;
    ParticleSystem particles_;
    Playlist     playlist_;
    UI           ui_;

    float bloomStr_  = 0.6f;
    float time_      = 0.f;

    // 拖拽队列
    std::vector<std::string> dropQueue_;
};

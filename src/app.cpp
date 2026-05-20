#include "app.h"
#include <iostream>
#include <filesystem>
#include "unicode_utils.h" 

namespace fs = std::filesystem;

// ── 全局指针（用于 C 回调）──────────────────────────────
static App* g_app = nullptr;

bool App::init(int w, int h)
{
    winW_ = w; winH_ = h;
    g_app = this;

    // ── GLFW ──
    if (!glfwInit()) { std::cerr << "GLFW init failed\n"; return false; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window_ = glfwCreateWindow(w, h, "Music Visualizer", nullptr, nullptr);
    if (!window_) { std::cerr << "Window creation failed\n"; glfwTerminate(); return false; }
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // VSync

    // ── GLAD ──
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD init failed\n"; return false;
    }
    std::cout << "OpenGL " << glGetString(GL_VERSION) << "\n";

    // ── 拖拽回调 ──
    glfwSetDropCallback(window_, dropCallback);

    // ── 子系统 ──
    if (!audio_.init()) return false;
    if (!vis_.init(1024, 128)) return false;
    if (!bloom_.init(w, h, 5)) return false;
    if (!particles_.init(2000)) return false;
    if (!ui_.init(window_)) return false;

    // 添加示例播放列表
    playlist_.addFile("example.mp3");

    // 默认背景
    glClearColor(0.03f, 0.03f, 0.06f, 1.f);
    glEnable(GL_MULTISAMPLE);

    return true;
}

void App::run()
{
    float lastTime = (float)glfwGetTime();

    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        processDrop();

        float now = (float)glfwGetTime();
        float dt  = now - lastTime;
        lastTime  = now;
        time_     = now;

        // 窗口 resize
        int fbW, fbH;
        glfwGetFramebufferSize(window_, &fbW, &fbH);
        if (fbW != winW_ || fbH != winH_) {
            winW_ = fbW; winH_ = fbH;
            bloom_.resize(winW_, winH_);
            glViewport(0, 0, winW_, winH_);
        }

        // ── 1. 渲染到 Bloom 场景 FBO ──
        GLuint sceneFBO = bloom_.sceneFBO();
        glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
        glViewport(0, 0, winW_, winH_);
        glClearColor(0.03f, 0.03f, 0.06f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 投影矩阵（正交）
        float halfW = winW_ * 0.5f;
        float halfH = winH_ * 0.5f;
        Mat4 proj   = Mat4::ortho(-halfW, halfW, -halfH, halfH);
        float cx    = 0.f;
        float cy    = 0.f;

        // 更新频谱
        vis_.update(dt, audio_);

        // 绘制频谱柱
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        vis_.draw(proj.ptr(), cx, cy, vis_.radius_, time_);

        // 绘制粒子
        particles_.update(dt, cx, cy, vis_.radius_, audio_);
        particles_.draw(proj.ptr());

        glDisable(GL_BLEND);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ── 2. Bloom 后处理 ──
        GLuint sceneTex = bloom_.sceneTexture();
        bloom_.apply(sceneTex, bloomStr_);

        // ── 3. 将合成结果 blit 到默认帧缓冲 ──
        glBindFramebuffer(GL_READ_FRAMEBUFFER, bloom_.compositeFBO());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, winW_, winH_,
                          0, 0, winW_, winH_,
                          GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ── 4. ImGui 叠加 ──
        ui_.beginFrame();
        ui_.draw(*this);
        ui_.endFrame();

        glfwSwapBuffers(window_);
    }
}

void App::shutdown()
{
    audio_.shutdown();
    vis_.destroy();
    bloom_.destroy();
    particles_.destroy();
    ui_.shutdown();
    if (window_) glfwDestroyWindow(window_);
    glfwTerminate();
    g_app = nullptr;
}

void App::playTrack(int index)
{
    if (index < 0 || index >= playlist_.size()) return;
    playlist_.setCurrent(index);
    audio_.loadAndPlay(playlist_.at(index).path);
}

// ── 拖拽回调（GLFW C 函数）──
void App::dropCallback(GLFWwindow* /*win*/, int count, const char** paths)
{
    if (!g_app) return;
    for (int i = 0; i < count; ++i)
        g_app->dropQueue_.emplace_back(paths[i]);
}

void App::processDrop()
{
    if (dropQueue_.empty()) return;

    for (auto& p : dropQueue_) {
        // 修复：通过 utf8Path 正确构造 fs::path，extension 检查才能生效
        auto fp = utf8Path(p);
        auto ext = fp.extension().string();
        // 统一转小写比较
        for (auto& c : ext) c = (char)std::tolower((unsigned char)c);

        if (ext == ".mp3" || ext == ".wav" ||
            ext == ".ogg" || ext == ".flac") {
            playlist_.addFile(p);
            // 修复：用原始 UTF-8 字符串直接输出（配合 setupConsoleUtf8）
            std::cout << "[Drop] Added: " << p << "\n";
        } else {
            std::cout << "[Drop] Skipped (unsupported): " << p << "\n";
        }
    }
    dropQueue_.clear();

    if (!audio_.isPlaying() && playlist_.size() > 0) {
        playTrack(playlist_.currentIndex());
    }
}

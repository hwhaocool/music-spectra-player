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
    if (!window_) {
        std::cerr << "Window creation failed\n";
        glfwTerminate(); return false;
    }

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
    if (!vis_.init(1024)) return false;
    // if (!bloom_.init(w, h, 5)) return false;
    if (!particles_.init(2000)) return false;
    if (!ui_.init(window_)) return false;

    // ── Bloom 按 viewport 尺寸初始化 ──
    int initVpW = w - (int)kLeftPanelW;
    int initVpH = h - (int)kControlsH;
    if (initVpW < 1) initVpW = 1;
    if (initVpH < 1) initVpH = 1;
    if (!bloom_.init(initVpW, initVpH, 5)) return false;
    lastVpW_ = initVpW;
    lastVpH_ = initVpH;

    // 添加示例播放列表
    playlist_.addFile("example.mp3");
    playlist_.addFile("X:\\01.wav");

    // 默认背景
    glClearColor(kClearColor[0], kClearColor[1], kClearColor[2], kClearColor[3]);
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

        // ── 窗口尺寸 ──
        int winSizeW, winSizeH;
        glfwGetWindowSize(window_, &winSizeW, &winSizeH);

        int fbW, fbH;
        glfwGetFramebufferSize(window_, &fbW, &fbH);
        if (fbW != winW_ || fbH != winH_) {
            winW_ = fbW;
            winH_ = fbH;
            // ← 注意：这里 不 再 resize bloom
        }

        float dpiScale = (winSizeW > 0) ? ((float)fbW / (float)winSizeW) : 1.f;

        // ── 右侧面板 viewport ──
        float specScreenW = (float)winSizeW - kLeftPanelW;
        float specScreenH = (float)winSizeH - kControlsH;
        if (specScreenW < 1.f) specScreenW = 1.f;
        if (specScreenH < 1.f) specScreenH = 1.f;

        int vpX = (int)(kLeftPanelW * dpiScale);
        int vpY = (int)(kControlsH  * dpiScale);
        int vpW = (int)(specScreenW * dpiScale);
        int vpH = (int)(specScreenH * dpiScale);

        // ── Bloom 按 viewport 尺寸重建（关键！）──
        if (vpW != lastVpW_ || vpH != lastVpH_) {
            lastVpW_ = vpW;
            lastVpH_ = vpH;
            if (vpW > 0 && vpH > 0)
                bloom_.resize(vpW, vpH);
        }

        // ── 自适应尺寸 ──
        float shortEdge  = (specScreenW < specScreenH) ? specScreenW : specScreenH;
        float autoRadius = shortEdge * 0.30f;
        float autoBarMax = shortEdge * 0.25f;

        // ============================================================
        //  1. 渲染频谱到 sceneFBO（尺寸 = vpW × vpH）
        //     暂停时跳过清屏+重绘，保留上一帧画面
        // ============================================================
        bool paused = !audio_.isPlaying() && audio_.isLoaded();

        if (!paused) {
            glBindFramebuffer(GL_FRAMEBUFFER, bloom_.sceneFBO());

            glViewport(0, 0, vpW, vpH);
            glClearColor(kClearColor[0], kClearColor[1], kClearColor[2], kClearColor[3]);
            glClear(GL_COLOR_BUFFER_BIT);

            float halfW = specScreenW * 0.5f;
            float halfH = specScreenH * 0.5f;
            Mat4 proj   = Mat4::ortho(-halfW, halfW, -halfH, halfH);

            vis_.update(dt, audio_);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            vis_.draw(proj.ptr(), 0.f, 0.f, autoRadius, autoBarMax, time_);
            particles_.update(dt, 0.f, 0.f, autoRadius, audio_);
            particles_.draw(proj.ptr());
            glDisable(GL_BLEND);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // ============================================================
        //  2. Bloom 后处理（FBO = viewport 尺寸）
        // ============================================================
        bloom_.apply(bloom_.sceneTexture(), bloomStr_);

        // ============================================================
        //  3. Blit 到屏幕右侧面板
        //     composite ≈ vpW/2 × vpH/2，整体搬移
        // ============================================================
        int compW = 0, compH = 0;
        bloom_.compositeSize(compW, compH);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, bloom_.compositeFBO());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        glBlitFramebuffer(
            0, 0, compW, compH,                // src：composite 全范围
            vpX, vpY, vpX + vpW, vpY + vpH,   // dst：屏幕右侧面板
            GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ============================================================
        //  4. ImGui 覆盖层
        // ============================================================
        glViewport(0, 0, fbW, fbH);
        ui_.beginFrame();
        ui_.draw(*this, winSizeW, winSizeH);
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

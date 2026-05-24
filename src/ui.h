#pragma once

class App;

struct GLFWwindow;
struct GLFWcursor;

class UI {
public:
    UI() = default;
    bool init(void* glfwWindow);
    void shutdown();
    void beginFrame();
    void draw(App& app, int windowWidth, int windowHeight);
    void endFrame();
    void handleWindowResize(int winW, int winH);

private:
    void drawTitleBar(App& app, float winW, float winH);
    void drawLeftPanel(App& app, float winW, float winH);
    void drawControls(App& app, float winW, float winH);
    void drawSettings(App& app);

    GLFWwindow* window_      = nullptr;
    float volumeSlider_    = 0.8f;
    bool  showSettings_    = false;

    char  newPlaylistNameBuf_[128] = {};
    bool  showNewPlaylistPopup_ = false;

    // 布局常量
    float leftPanelWidth_  = 280.f;
    float controlsHeight_  = 110.f;

    // 窗口边缘拖拽调整大小
    GLFWcursor* cursorArrow_ = nullptr;
    GLFWcursor* cursorNS_    = nullptr;   // 上下
    GLFWcursor* cursorEW_    = nullptr;   // 左右
    GLFWcursor* cursorNWSE_  = nullptr;   // 左上↔右下
    GLFWcursor* cursorNESW_  = nullptr;   // 右上↔左下

    bool   resizing_        = false;
    int    resizeEdge_      = 0;      // bitmask: 1=左 2=右 4=上 8=下
    double resizeStartX_    = 0;
    double resizeStartY_    = 0;
    int    resizeStartW_    = 0;
    int    resizeStartH_    = 0;
    int    resizeStartPosX_ = 0;
    int    resizeStartPosY_ = 0;
};

#pragma once

class App;

struct GLFWwindow;

class UI {
public:
    UI() = default;
    bool init(void* glfwWindow);
    void shutdown();
    void beginFrame();
    void draw(App& app, int windowWidth, int windowHeight);
    void endFrame();

private:
    void drawTitleBar(App& app, float winW, float winH);
    void drawLeftPanel(App& app, float winW, float winH);
    void drawControls(App& app, float winW, float winH);
    void drawSettings(App& app);

    GLFWwindow* window_      = nullptr;
    float volumeSlider_    = 0.8f;
    bool  showSettings_    = false;

    // 布局常量
    float leftPanelWidth_  = 280.f;
    float controlsHeight_  = 110.f;
};

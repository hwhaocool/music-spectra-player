#pragma once
#include <string>

class App;

class UI {
public:
    UI() = default;
    bool init(void* glfwWindow);
    void shutdown();
    void beginFrame();
    void draw(App& app);
    void endFrame();

    // Drag‑Drop 回调
    void setDropCount(int n) { dropCount_ = n; }

private:
    void drawPlaylist(App& app);
    void drawPlayerControls(App& app);
    void drawSettings(App& app);

    int  dropCount_ = 0;
    bool showSettings_ = false;
    float volumeSlider_ = 0.8f;
};

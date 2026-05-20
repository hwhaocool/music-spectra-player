#include "ui.h"
#include "app.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <algorithm>

bool UI::init(void* glfwWin)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    io.IniFilename = nullptr;

    // Load font (MS YaHei for Chinese characters; falls back gracefully)
    if (!io.Fonts->AddFontFromFileTTF(
            "C:\\Windows\\Fonts\\msyh.ttc", 18.0f, nullptr,
            io.Fonts->GetGlyphRangesChineseFull())) {
        // Fallback: try another common Chinese font, then default
        io.Fonts->AddFontFromFileTTF(
            "C:\\Windows\\Fonts\\simsun.ttc", 18.0f, nullptr,
            io.Fonts->GetGlyphRangesChineseFull());
    }

    // 暗色 + 圆角风格
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding   = 8.f;
    style.FrameRounding    = 6.f;
    style.GrabRounding     = 4.f;
    style.ScrollbarRounding= 6.f;
    style.WindowPadding    = ImVec2(16, 16);
    style.ItemSpacing      = ImVec2(8, 10);

    // 霓虹配色
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg]        = ImVec4(0.06f, 0.06f, 0.10f, 0.92f);
    colors[ImGuiCol_TitleBg]         = ImVec4(0.08f, 0.08f, 0.14f, 1.f);
    colors[ImGuiCol_TitleBgActive]   = ImVec4(0.12f, 0.12f, 0.22f, 1.f);
    colors[ImGuiCol_FrameBg]         = ImVec4(0.12f, 0.12f, 0.18f, 1.f);
    colors[ImGuiCol_Button]          = ImVec4(0.15f, 0.15f, 0.28f, 1.f);
    colors[ImGuiCol_ButtonHovered]   = ImVec4(0.25f, 0.25f, 0.45f, 1.f);
    colors[ImGuiCol_ButtonActive]    = ImVec4(0.35f, 0.10f, 0.50f, 1.f);
    colors[ImGuiCol_SliderGrab]      = ImVec4(0.00f, 0.90f, 0.90f, 1.f);
    colors[ImGuiCol_SliderGrabActive]= ImVec4(0.90f, 0.10f, 0.90f, 1.f);
    colors[ImGuiCol_Header]          = ImVec4(0.18f, 0.18f, 0.32f, 1.f);
    colors[ImGuiCol_HeaderHovered]   = ImVec4(0.25f, 0.25f, 0.48f, 1.f);
    colors[ImGuiCol_Text]            = ImVec4(0.92f, 0.92f, 0.95f, 1.f);
    colors[ImGuiCol_TextDisabled]    = ImVec4(0.45f, 0.45f, 0.52f, 1.f);

    ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(glfwWin), true);
    ImGui_ImplOpenGL3_Init("#version 430");
    return true;
}

void UI::shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UI::beginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UI::draw(App& app)
{
    // ── 左侧播放列表面板 ──
    ImGui::SetNextWindowPos(ImVec2(16, 16), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);
    ImGui::Begin("播放列表");
    drawPlaylist(app);
    ImGui::End();

    //── 底部播放控制 ──
    ImGui::SetNextWindowPos(ImVec2(16, 530), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 120), ImGuiCond_FirstUseEver);
    ImGui::Begin("控制");
    drawPlayerControls(app);
    ImGui::End();

    // ── 设置面板 ──
    if (showSettings_) {
        ImGui::SetNextWindowSize(ImVec2(280, 200), ImGuiCond_FirstUseEver);
        ImGui::Begin("设置", &showSettings_);
        drawSettings(app);
        ImGui::End();
    }
}

void UI::endFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// ── 播放列表面板 ──
void UI::drawPlaylist(App& app)
{
    auto& pl = app.playlist();

    if (ImGui::Button(" + 添加")) {
        // 由 App 层处理 drop / 文件对话框
    }
    ImGui::SameLine();
    if (ImGui::Button("清除")) pl.clear();
    ImGui::SameLine();
    if (ImGui::Button("设置")) showSettings_ = !showSettings_;

    ImGui::Separator();
    ImGui::Text("拖放文件到这里");
    ImGui::Separator();

    for (int i = 0; i < pl.size(); ++i) {
        ImGui::PushID(i);
        bool selected = (i == pl.currentIndex());
        if (ImGui::Selectable(pl.at(i).displayName.c_str(), selected)) {
            pl.setCurrent(i);
            app.playTrack(i);
        }
        ImGui::SameLine(ImGui::GetWindowWidth() - 60);
        if (ImGui::SmallButton("X")) {
            pl.remove(i);
            --i;
        }
        ImGui::PopID();
    }
}

// ── 底部控制条 ──
void UI::drawPlayerControls(App& app)
{
    auto& engine = app.audio();

    // 进度条
    float cur  = engine.getCursorSeconds();
    float dur  = engine.getDurationSeconds();
    float prog = (dur > 0.f) ? cur / dur : 0.f;
    ImGui::ProgressBar(prog, ImVec2(-1, 6), "");

    // 时间显示
    int cMin = (int)(cur / 60), cSec = (int)cur % 60;
    int dMin = (int)(dur / 60), dSec = (int)dur % 60;
    ImGui::Text("%02d:%02d / %02d:%02d", cMin, cSec, dMin, dSec);

    // 播放按钮
    bool playing = engine.isPlaying();
    if (ImGui::Button(playing ? " || " : " > ")) {
        if (playing) engine.pause();
        else         engine.resume();
    }
    ImGui::SameLine();
    if (ImGui::Button(" |< ")) {
        int idx = app.playlist().prev();
        if (idx >= 0) app.playTrack(idx);
    }
    ImGui::SameLine();
    if (ImGui::Button(" >| ")) {
        int idx = app.playlist().next();
        if (idx >= 0) app.playTrack(idx);
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop")) engine.stop();

    // 音量
    ImGui::SameLine();
    ImGui::PushItemWidth(100);
    if (ImGui::SliderFloat("Vol", &volumeSlider_, 0.f, 1.f, "%.2f"))
        engine.setVolume(volumeSlider_);
    ImGui::PopItemWidth();
}

// ── 设置面板 ──
void UI::drawSettings(App& app)
{
    ImGui::Text("Visualizer Settings");
    ImGui::SliderFloat("Bar Height", &app.vis().barMaxHeight_, 20.f, 200.f);
    ImGui::SliderFloat("Radius",     &app.vis().radius_,       50.f, 300.f);
    ImGui::SliderFloat("Smoothing",  &app.vis().smoothing_,    0.01f, 0.5f);
    ImGui::SliderFloat("Bloom",      &app.bloomStrength(),     0.f, 2.f);
}

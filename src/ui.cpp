#include "ui.h"
#include "app.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <cstdio>

// ── 工具：格式化秒数为 mm:ss ──
static void formatTime(char* buf, size_t len, float seconds)
{
    if (seconds < 0.f) seconds = 0.f;
    int m = (int)(seconds / 60.f);
    int s = (int)seconds % 60;
    snprintf(buf, len, "%02d:%02d", m, s);
}

// ==========================================================
//  初始化
// ==========================================================

bool UI::init(void* glfwWin)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;  // 禁用 imgui.ini 持久化

    // Load font (MS YaHei for Chinese characters; falls back gracefully)
    if (!io.Fonts->AddFontFromFileTTF(
            "C:\\Windows\\Fonts\\msyh.ttc", 18.0f, nullptr,
            io.Fonts->GetGlyphRangesChineseFull())) {
        // Fallback: try another common Chinese font, then default
        io.Fonts->AddFontFromFileTTF(
            "C:\\Windows\\Fonts\\simsun.ttc", 18.0f, nullptr,
            io.Fonts->GetGlyphRangesChineseFull());
    }

    // ── 全局样式 ──
    ImGui::StyleColorsDark();
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding    = 0.f;   // 固定面板无圆角
    s.FrameRounding     = 4.f;
    s.GrabRounding      = 4.f;
    s.ScrollbarRounding = 4.f;
    s.WindowPadding     = ImVec2(12, 12);
    s.FramePadding      = ImVec2(8, 4);
    s.ItemSpacing       = ImVec2(8, 8);
    s.ScrollbarSize     = 10.f;

    // ── 霓虹暗色主题 ──
    ImVec4* c = s.Colors;
    c[ImGuiCol_WindowBg]         = ImVec4(0.05f, 0.05f, 0.08f, 1.f);
    c[ImGuiCol_ChildBg]          = ImVec4(0.07f, 0.07f, 0.11f, 1.f);
    c[ImGuiCol_PopupBg]          = ImVec4(0.08f, 0.08f, 0.12f, 0.96f);
    c[ImGuiCol_Border]           = ImVec4(0.15f, 0.15f, 0.22f, 1.f);
    c[ImGuiCol_FrameBg]          = ImVec4(0.10f, 0.10f, 0.16f, 1.f);
    c[ImGuiCol_FrameBgHovered]   = ImVec4(0.16f, 0.16f, 0.26f, 1.f);
    c[ImGuiCol_FrameBgActive]    = ImVec4(0.22f, 0.12f, 0.36f, 1.f);
    c[ImGuiCol_TitleBg]          = ImVec4(0.06f, 0.06f, 0.10f, 1.f);
    c[ImGuiCol_TitleBgActive]    = ImVec4(0.08f, 0.08f, 0.14f, 1.f);
    c[ImGuiCol_Button]           = ImVec4(0.14f, 0.14f, 0.24f, 1.f);
    c[ImGuiCol_ButtonHovered]    = ImVec4(0.22f, 0.22f, 0.42f, 1.f);
    c[ImGuiCol_ButtonActive]     = ImVec4(0.32f, 0.10f, 0.48f, 1.f);
    c[ImGuiCol_Header]           = ImVec4(0.14f, 0.14f, 0.24f, 1.f);
    c[ImGuiCol_HeaderHovered]    = ImVec4(0.22f, 0.22f, 0.40f, 1.f);
    c[ImGuiCol_HeaderActive]     = ImVec4(0.30f, 0.12f, 0.46f, 1.f);
    c[ImGuiCol_SliderGrab]       = ImVec4(0.00f, 0.85f, 0.85f, 1.f);
    c[ImGuiCol_SliderGrabActive] = ImVec4(0.85f, 0.10f, 0.85f, 1.f);
    c[ImGuiCol_Text]             = ImVec4(0.90f, 0.90f, 0.94f, 1.f);
    c[ImGuiCol_TextDisabled]     = ImVec4(0.40f, 0.40f, 0.50f, 1.f);
    c[ImGuiCol_Separator]        = ImVec4(0.15f, 0.15f, 0.22f, 1.f);
    c[ImGuiCol_ScrollbarBg]      = ImVec4(0.05f, 0.05f, 0.08f, 0.6f);
    c[ImGuiCol_ScrollbarGrab]    = ImVec4(0.18f, 0.18f, 0.28f, 1.f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.25f, 0.25f, 0.38f, 1.f);

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

// ==========================================================
//  帧控制
// ==========================================================

void UI::beginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UI::endFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// ==========================================================
//  主入口
// ==========================================================

void UI::draw(App& app, int windowWidth, int windowHeight)
{
    float winW = (float)windowWidth;
    float winH = (float)windowHeight;

    drawLeftPanel(app, winW, winH);
    drawControls(app, winW, winH);

    if (showSettings_)
        drawSettings(app);
}

// ==========================================================
//  左侧面板（固定，不可移动）
// ==========================================================

void UI::drawLeftPanel(App& app, float winW, float winH)
{
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoMove       |
        ImGuiWindowFlags_NoResize     |
        ImGuiWindowFlags_NoCollapse   |
        ImGuiWindowFlags_NoTitleBar   |
        ImGuiWindowFlags_NoScrollbar  |
        ImGuiWindowFlags_NoScrollWithMouse;

    // 固定到左侧，完全不透明
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(leftPanelWidth_, winH), ImGuiCond_Always);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.08f, 1.f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.f, 16.f));

    ImGui::Begin("##LeftPanel", nullptr, flags);

    // ── 标题区 ──
    ImGui::Text("MUSIC PLAYER");
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.45f, 0.45f, 0.55f, 1.f));
    ImGui::Text("Circular Visualizer");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ── 播放列表标题行 ──
    ImGui::Text("播放列表");
    ImGui::SameLine(ImGui::GetWindowWidth() - 60.f);
    if (ImGui::SmallButton("清除"))
        app.playlist().clear();
    ImGui::Separator();

    // ── 播放列表滚动区域 ──
    //   计算底部固定区域高度：拖拽文本 + 设置按钮 + 间距
    float bottomReserved = 80.f;
    float playlistH = ImGui::GetContentRegionAvail().y - bottomReserved;
    if (playlistH < 60.f) playlistH = 60.f;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.10f, 1.f));

    if (ImGui::BeginChild("##Playlist", ImVec2(0.f, playlistH), ImGuiChildFlags_Borders,
                          ImGuiWindowFlags_AlwaysVerticalScrollbar))
    {
        auto& pl = app.playlist();

        if (pl.size() == 0) {
            ImGui::TextDisabled("  No tracks");
        }

        for (int i = 0; i < pl.size(); ++i) {
            ImGui::PushID(i);

            bool selected = (i == pl.currentIndex());

            // 文件名（可点击选中）
            if (ImGui::Selectable(pl.at(i).displayName.c_str(), selected,
                                  ImGuiSelectableFlags_AllowDoubleClick,
                                  ImVec2(0, 22))) {
                pl.setCurrent(i);
                app.playTrack(i);
            }

            // 删除按钮（同一行右侧）
            ImGui::SameLine(ImGui::GetWindowWidth() - 28.f);
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0,0,0,0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.6f,0.1f,0.1f,0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.8f,0.1f,0.1f,0.8f));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 0));
            char btnId[16];
            snprintf(btnId, sizeof(btnId), "X##%d", i);
            if (ImGui::SmallButton(btnId)) {
                pl.remove(i);
                --i;
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);

            ImGui::PopID();
        }
    }
    ImGui::EndChild();

    ImGui::PopStyleColor(); // ChildBg

    // ── 底部固定区域 ──
    ImGui::Separator();
    ImGui::Spacing();

    // 居中提示文本
    const char* dropText = "Drag & Drop audio file here";
    float textW = ImGui::CalcTextSize(dropText).x;
    float panelW = ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x * 2;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (panelW - textW) * 0.5f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.35f, 0.45f, 1.f));
    ImGui::Text("%s", dropText);
    ImGui::PopStyleColor();

    ImGui::Spacing();

    // 设置按钮
    if (ImGui::Button("设置", ImVec2(-1, 28)))
        showSettings_ = !showSettings_;

    ImGui::End();
    ImGui::PopStyleVar();   // WindowPadding
    ImGui::PopStyleColor(); // WindowBg
}

// ==========================================================
//  右下角控制条
// ==========================================================

void UI::drawControls(App& app, float winW, float winH)
{
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoMove       |
        ImGuiWindowFlags_NoResize     |
        ImGuiWindowFlags_NoCollapse   |
        ImGuiWindowFlags_NoTitleBar;

    float x = leftPanelWidth_;
    float y = winH - controlsHeight_;
    float w = winW - leftPanelWidth_;

    ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(w, controlsHeight_), ImGuiCond_Always);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.08f, 0.95f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.f, 10.f));

    ImGui::Begin("##Controls", nullptr, flags);

    auto& engine   = app.audio();
    auto& playlist = app.playlist();

    // ── 第一行：进度条 ──
    float cur  = engine.getCursorSeconds();
    float dur  = engine.getDurationSeconds();
    float prog = (dur > 0.01f) ? (cur / dur) : 0.f;

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram,       ImVec4(0.0f, 0.85f, 0.85f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg,             ImVec4(0.10f, 0.10f, 0.16f, 1.f));
    ImGui::ProgressBar(prog, ImVec2(-1, 6), "");
    ImGui::PopStyleColor(2);

    ImGui::Spacing();

    // ── 第二行：时间 + 按钮 + 音量 ──
    char timeBuf[32];
    char durBuf[32];
    formatTime(timeBuf, sizeof(timeBuf), cur);
    formatTime(durBuf,  sizeof(durBuf),  dur);
    ImGui::Text("%s / %s", timeBuf, durBuf);

    ImGui::SameLine(0, 20);

    // 播放 / 暂停
    bool playing = engine.isPlaying();
    ImGui::PushStyleColor(ImGuiCol_Button,      ImVec4(0.14f, 0.14f, 0.28f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.22f, 0.45f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.35f, 0.12f, 0.50f, 1.f));

    if (ImGui::Button(playing ? " || " : " > ", ImVec2(40, 26))) {
        if (playing) engine.pause();
        else         engine.resume();
    }
    ImGui::SameLine(0, 4);

    // 上一曲
    if (ImGui::Button(" |< ", ImVec2(40, 26))) {
        int idx = playlist.prev();
        if (idx >= 0) app.playTrack(idx);
    }
    ImGui::SameLine(0, 4);

    // 下一曲
    if (ImGui::Button(" >| ", ImVec2(40, 26))) {
        int idx = playlist.next();
        if (idx >= 0) app.playTrack(idx);
    }
    ImGui::SameLine(0, 4);

    // 停止
    if (ImGui::Button("Stop", ImVec2(48, 26)))
        engine.stop();

    ImGui::SameLine(0, 20);

    // 音量
    ImGui::PushItemWidth(120);
    ImGui::Text("Vol");
    ImGui::SameLine();
    if (ImGui::SliderFloat("##Vol", &volumeSlider_, 0.f, 1.f, "%.2f"))
        engine.setVolume(volumeSlider_);
    ImGui::PopItemWidth();

    ImGui::PopStyleColor(3); // button colors

    ImGui::End();
    ImGui::PopStyleVar();   // WindowPadding
    ImGui::PopStyleColor(); // WindowBg
}

// ==========================================================
//  设置浮窗
// ==========================================================

void UI::drawSettings(App& app)
{
    ImGui::SetNextWindowSize(ImVec2(300, 240), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(leftPanelWidth_ + 40.f, 40.f), ImGuiCond_FirstUseEver);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.06f, 0.10f, 0.97f));

    if (ImGui::Begin("Settings", &showSettings_)) {
        ImGui::Text("Visualizer");
        ImGui::Separator();
        ImGui::SliderFloat("Bar Height", &app.vis().barMaxHeight_, 20.f, 200.f);
        ImGui::SliderFloat("Radius",     &app.vis().radius_,       50.f, 300.f);
        ImGui::SliderFloat("Smoothing",  &app.vis().smoothing_,    0.01f, 0.5f);
        ImGui::Spacing();
        ImGui::Text("Post Processing");
        ImGui::Separator();
        ImGui::SliderFloat("Bloom",      &app.bloomStrength(),     0.f, 2.f);
    }
    ImGui::End();

    ImGui::PopStyleColor();
}

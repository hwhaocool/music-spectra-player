#include "ui.h"
#include "app.h"
#include "themes.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <windows.h>
#include <GLFW/glfw3native.h>
#endif
#include <cstdio>
#include <cstring>
#include "IconsFontAwesome6.h"
#include "assets/font-inter/Inter-Regular.h"
#include "assets/font-awesome/fa-solid-900.h"

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

void setupFonts()
{
    ImGuiIO& io = ImGui::GetIO();

    // 主字体
    const bin2cpp::File & interRegularTtf = bin2cpp::getInterRegularTtfFile();
    const char * buffer = interRegularTtf.getBuffer();

    io.Fonts->AddFontFromMemoryTTF(
        (void*)buffer,
        interRegularTtf.getSize(),
        18.0f
    );

    // 合并中文字体（fallback 链）
    {
        ImFontConfig cfg;
        cfg.MergeMode = true;

        if (!io.Fonts->AddFontFromFileTTF(
                "C:\\Windows\\Fonts\\msyh.ttc", 18.0f, &cfg,
                io.Fonts->GetGlyphRangesChineseFull())) {
            io.Fonts->AddFontFromFileTTF(
                "C:\\Windows\\Fonts\\simsun.ttc", 18.0f, &cfg,
                io.Fonts->GetGlyphRangesChineseFull());
        }
    }

    // 合并 FontAwesome
    {
        static const ImWchar icons_ranges[] =
        {
            ICON_MIN_FA,
            ICON_MAX_16_FA,
            0
        };

        ImFontConfig cfg;
        cfg.MergeMode = true;
        cfg.PixelSnapH = true;

        const bin2cpp::File & fa_solid_900_ttf = bin2cpp::getFasolid900TtfFile();
        const char * buffer = fa_solid_900_ttf.getBuffer();

        io.Fonts->AddFontFromMemoryTTF(
            (void*)fa_solid_900_ttf.getBuffer(),
            fa_solid_900_ttf.getSize(),
            16.0f,
            &cfg,
            icons_ranges);
    }
}

bool UI::init(void* glfwWin)
{
    window_ = static_cast<GLFWwindow*>(glfwWin);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;  // 禁用 imgui.ini 持久化

    setupFonts();
    
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

    cursorArrow_ = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    cursorNS_    = glfwCreateStandardCursor(GLFW_RESIZE_NS_CURSOR);
    cursorEW_    = glfwCreateStandardCursor(GLFW_RESIZE_EW_CURSOR);
    cursorNWSE_  = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
    cursorNESW_  = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);

    return true;
}

void UI::shutdown()
{
    if (cursorArrow_) glfwDestroyCursor(cursorArrow_);
    if (cursorNS_)    glfwDestroyCursor(cursorNS_);
    if (cursorEW_)    glfwDestroyCursor(cursorEW_);
    if (cursorNWSE_)  glfwDestroyCursor(cursorNWSE_);
    if (cursorNESW_)  glfwDestroyCursor(cursorNESW_);

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
//  窗口边缘拖拽调整大小
// ==========================================================

void UI::handleWindowResize(int winW, int winH)
{
    constexpr float kEdgeSize = 8.f;

    double mx, my;
    glfwGetCursorPos(window_, &mx, &my);

    bool nearLeft   = (mx >= 0 && mx <= kEdgeSize);
    bool nearRight  = (mx <= winW && mx >= winW - kEdgeSize);
    bool nearTop    = (my >= 0 && my <= kEdgeSize);
    bool nearBottom = (my <= winH && my >= winH - kEdgeSize);

    int edge = 0;
    if (nearLeft)   edge |= 1;
    if (nearRight)  edge |= 2;
    if (nearTop)    edge |= 4;
    if (nearBottom) edge |= 8;

    bool leftDown = glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    if (!resizing_) {
        if (edge == 0) {
            glfwSetCursor(window_, cursorArrow_);
            return;
        }
        // 设置光标形状
        if (edge == 4 || edge == 8)            glfwSetCursor(window_, cursorNS_);   // 上下
        else if (edge == 1 || edge == 2)        glfwSetCursor(window_, cursorEW_);   // 左右
        else if (edge == 5 || edge == 10)       glfwSetCursor(window_, cursorNWSE_); // 左上/右下
        else if (edge == 6 || edge == 9)        glfwSetCursor(window_, cursorNESW_); // 右上/左下

        if (leftDown) {
            resizing_ = true;
            resizeEdge_ = edge;
            glfwGetCursorPos(window_, &resizeStartX_, &resizeStartY_);
            glfwGetWindowSize(window_, &resizeStartW_, &resizeStartH_);
            glfwGetWindowPos(window_, &resizeStartPosX_, &resizeStartPosY_);
        }
    } else {
        if (leftDown) {
            double dx = mx - resizeStartX_;
            double dy = my - resizeStartY_;

            int newW = resizeStartW_;
            int newH = resizeStartH_;
            int newX = resizeStartPosX_;
            int newY = resizeStartPosY_;

            if (resizeEdge_ & 1) { newW = resizeStartW_ - (int)dx;  newX = resizeStartPosX_ + (int)dx; }
            if (resizeEdge_ & 2) { newW = resizeStartW_ + (int)dx; }
            if (resizeEdge_ & 4) { newH = resizeStartH_ - (int)dy;  newY = resizeStartPosY_ + (int)dy; }
            if (resizeEdge_ & 8) { newH = resizeStartH_ + (int)dy; }

            int minW = (int)App::kLeftPanelW + 200;
            int minH = (int)App::kTitleBarH + (int)App::kControlsH + 100;
            if (newW < minW) newW = minW;
            if (newH < minH) newH = minH;

            glfwSetWindowSize(window_, newW, newH);
            if (resizeEdge_ & (1 | 4))
                glfwSetWindowPos(window_, newX, newY);
        } else {
            resizing_ = false;
            resizeEdge_ = 0;
            glfwSetCursor(window_, cursorArrow_);
        }
    }
}

// ==========================================================
//  主入口
// ==========================================================

void UI::draw(App& app, int windowWidth, int windowHeight)
{
    float winW = (float)windowWidth;
    float winH = (float)windowHeight;

    drawTitleBar(app, winW, winH);
    drawLeftPanel(app, winW, winH);
    drawControls(app, winW, winH);

    if (showSettings_) {
        drawSettings(app);
    }
}

// ==========================================================
//  自定义标题栏
// ==========================================================

void UI::drawTitleBar(App& app, float winW, float winH)
{
    const float barH = App::kTitleBarH;
    const float btnW = 46.f;

    // 标题栏窗口（提供 ImGui 交互上下文）
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(winW, barH));
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.04f, 0.08f, 1.f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##titlebar", nullptr, flags);
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();

    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 wPos = ImGui::GetWindowPos();
    ImVec2 wSize = ImGui::GetWindowSize();

    // 底部分割线
    draw->AddLine(ImVec2(wPos.x, wPos.y + wSize.y),
                  ImVec2(wPos.x + wSize.x, wPos.y + wSize.y),
                  IM_COL32(30, 30, 50, 255), 1.f);

    // 标题
    draw->AddText(ImVec2(wPos.x + 16.f, wPos.y + 14.f),
                  IM_COL32(241, 36, 186, 255), ICON_FA_MUSIC);
    draw->AddText(ImVec2(wPos.x + 38.f, wPos.y + 14.f),
                  IM_COL32(180, 100, 255, 255), "Music Spectra Player");

    // ── 右侧按钮 ──
    float rightX = wPos.x + wSize.x;

    // --- Close ---
    float cx = rightX - btnW;
    ImGui::SetCursorScreenPos(ImVec2(cx, wPos.y));
    ImGui::InvisibleButton("##close", ImVec2(btnW, barH));
    if (ImGui::IsItemHovered())
        draw->AddRectFilled(ImVec2(cx, wPos.y), ImVec2(cx + btnW, wPos.y + barH),
                            IM_COL32(200, 40, 40, 200));
    draw->AddText(ImVec2(cx + 15.f, wPos.y + 14.f),
                  ImGui::IsItemHovered() ? IM_COL32_WHITE : IM_COL32(140, 140, 160, 255),
                  ICON_FA_XMARK);
    if (ImGui::IsItemClicked())
        glfwSetWindowShouldClose(window_, GLFW_TRUE);

    // --- Maximize ---
    float mx = cx - btnW;
    ImGui::SetCursorScreenPos(ImVec2(mx, wPos.y));
    ImGui::InvisibleButton("##maximize", ImVec2(btnW, barH));
    if (ImGui::IsItemHovered())
        draw->AddRectFilled(ImVec2(mx, wPos.y), ImVec2(mx + btnW, wPos.y + barH),
                            IM_COL32(40, 40, 80, 200));
    bool maximized = glfwGetWindowAttrib(window_, GLFW_MAXIMIZED);
    draw->AddText(ImVec2(mx + 15.f, wPos.y + 14.f),
                  ImGui::IsItemHovered() ? IM_COL32_WHITE : IM_COL32(140, 140, 160, 255),
                  maximized ? ICON_FA_WINDOW_RESTORE : ICON_FA_WINDOW_MAXIMIZE);
    if (ImGui::IsItemClicked()) {
        if (maximized)
            glfwRestoreWindow(window_);
        else
            glfwMaximizeWindow(window_);
    }

    // --- Minimize ---
    float minx = mx - btnW;
    ImGui::SetCursorScreenPos(ImVec2(minx, wPos.y));
    ImGui::InvisibleButton("##minimize", ImVec2(btnW, barH));
    if (ImGui::IsItemHovered())
        draw->AddRectFilled(ImVec2(minx, wPos.y), ImVec2(minx + btnW, wPos.y + barH),
                            IM_COL32(40, 40, 80, 200));
    draw->AddText(ImVec2(minx + 15.f, wPos.y + 14.f),
                  ImGui::IsItemHovered() ? IM_COL32_WHITE : IM_COL32(140, 140, 160, 255),
                  ICON_FA_MINUS);
    if (ImGui::IsItemClicked())
        glfwIconifyWindow(window_);

    // --- Theme popup (在最小化左边) ---
    float tx = minx - btnW;
    ImGui::SetCursorScreenPos(ImVec2(tx, wPos.y));
    ImGui::InvisibleButton("##theme", ImVec2(btnW, barH));
    if (ImGui::IsItemHovered())
        draw->AddRectFilled(ImVec2(tx, wPos.y), ImVec2(tx + btnW, wPos.y + barH),
                            IM_COL32(50, 40, 80, 200));
    draw->AddText(ImVec2(tx + 13.f, wPos.y + 14.f),
                  ImGui::IsItemHovered() ? IM_COL32(200, 150, 255, 255) : IM_COL32(160, 120, 200, 255),
                  ICON_FA_PALETTE);
    if (ImGui::IsItemClicked())
        ImGui::OpenPopup("##theme_popup");

    if (ImGui::BeginPopup("##theme_popup")) {
        for (int i = 0; i < kThemeCount; ++i) {
            bool selected = (i == app.vis().currentTheme_);
            if (ImGui::Selectable(kThemes[i].name, selected)) {
                app.vis().setTheme(i);
                app.particles().setShaders(kThemes[i].particleVert, kThemes[i].particleFrag);
            }
            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndPopup();
    }

    // ── 窗口拖拽（非按钮区域，使用原生 Windows 拖拽避免残影）──
    {
        ImGui::SetCursorScreenPos(ImVec2(wPos.x, wPos.y));
        ImGui::InvisibleButton("##drag", ImVec2(tx - wPos.x, barH));

        if (ImGui::IsItemActivated()) {
            HWND hwnd = glfwGetWin32Window(window_);
            ReleaseCapture();
            SendMessageA(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        }
    }

    ImGui::End();
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

    // 固定到左侧（标题栏下方）
    ImGui::SetNextWindowPos(ImVec2(0.f, App::kTitleBarH), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(App::kLeftPanelW, winH - App::kTitleBarH), ImGuiCond_Always);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.08f, 1.f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.f, 16.f));

    ImGui::Begin("##LeftPanel", nullptr, flags);

    // ── 播放列表选择器 ──
    {
        auto plNames = app.playlist().listPlaylistNames();
        std::string currentPlName = app.playlist().currentName();

        float btnW = 52.f;
        ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - btnW - 12.f);
        if (ImGui::BeginCombo("##playlistCombo", currentPlName.c_str())) {
            for (int i = 0; i < (int)plNames.size(); ++i) {
                bool isSelected = (plNames[i] == currentPlName);
                if (ImGui::Selectable(plNames[i].c_str(), isSelected))
                    app.playlist().switchToPlaylist(plNames[i]);
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine();
        if (ImGui::SmallButton("新增")) {
            showNewPlaylistPopup_ = true;
            ImGui::OpenPopup("新建播放列表###newPlaylistPopup");
            memset(newPlaylistNameBuf_, 0, sizeof(newPlaylistNameBuf_));
        }

        // 新建播放列表弹窗
        if (ImGui::BeginPopupModal("新建播放列表###newPlaylistPopup",
                                   &showNewPlaylistPopup_,
                                   ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("输入播放列表名称:");
            ImGui::SetItemDefaultFocus();
            ImGui::InputText("##newPlaylistName", newPlaylistNameBuf_,
                             sizeof(newPlaylistNameBuf_));
            if (ImGui::Button("确定", ImVec2(100, 0))) {
                std::string name(newPlaylistNameBuf_);
                if (!name.empty()) {
                    app.playlist().createPlaylist(name);
                    app.playlist().switchToPlaylist(name);
                    showNewPlaylistPopup_ = false;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("取消", ImVec2(100, 0)))
                showNewPlaylistPopup_ = false;
            ImGui::EndPopup();
        }
    }
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

            // 当前播放曲目紫色边框
            if (selected) {
                ImVec2 rMin = ImGui::GetItemRectMin();
                ImVec2 rMax = ImGui::GetItemRectMax();
                ImGui::GetWindowDrawList()->AddRect(rMin, rMax,
                    IM_COL32(180, 100, 255, 255), 0.f, 0, 2.f);
            }

            // 右键菜单
            if (ImGui::BeginPopupContextItem("##songMenu")) {
                if (ImGui::MenuItem("移除")) {
                    pl.remove(i);
                    --i;
                    ImGui::EndPopup();
                    ImGui::PopID();
                    continue;
                }
                ImGui::EndPopup();
            }

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
    // ── 关键修复：只在窗口未打开时允许点击打开 ──
    if (!showSettings_) {
        if (ImGui::Button("Settings", ImVec2(-1, 28)))
            showSettings_ = true;   // 单向：只能打开
    } else {
        // 窗口已打开：按钮显示为"关闭"样式，点击直接关闭
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.18f, 0.18f, 0.30f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.28f, 0.12f, 0.12f, 1.f));
        if (ImGui::Button("Settings", ImVec2(-1, 28)))
            showSettings_ = false;  // 单向：只能关闭
        ImGui::PopStyleColor(2);
    }

    ImGui::End();
    ImGui::PopStyleVar();   // WindowPadding
    ImGui::PopStyleColor(); // WindowBg
}

// ==========================================================
//  右下角控制条
// ==========================================================

// void drawControlsV1(App& app, float winW, float winH)
// {
//     ImGuiWindowFlags flags =
//         ImGuiWindowFlags_NoMove       |
//         ImGuiWindowFlags_NoResize     |
//         ImGuiWindowFlags_NoCollapse   |
//         ImGuiWindowFlags_NoTitleBar;

//     float x = leftPanelWidth_;
//     float y = winH - App::kControlsH;
//     float w = winW - App::kLeftPanelW;

//     ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
//     ImGui::SetNextWindowSize(ImVec2(w, App::kControlsH), ImGuiCond_Always);

//     ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.08f, 0.95f));
//     ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.f, 10.f));

//     ImGui::Begin("##Controls", nullptr, flags);

//     auto& engine   = app.audio();
//     auto& playlist = app.playlist();

//     // ── 第一行：进度条 ──
//     float cur  = engine.getCursorSeconds();
//     float dur  = engine.getDurationSeconds();
//     float prog = (dur > 0.01f) ? (cur / dur) : 0.f;

//     ImGui::PushStyleColor(ImGuiCol_PlotHistogram,       ImVec4(0.0f, 0.85f, 0.85f, 1.f));
//     ImGui::PushStyleColor(ImGuiCol_FrameBg,             ImVec4(0.10f, 0.10f, 0.16f, 1.f));
//     ImGui::ProgressBar(prog, ImVec2(-1, 6), "");
//     ImGui::PopStyleColor(2);

//     ImGui::Spacing();

//     // ── 第二行：时间 + 按钮 + 音量 ──
//     char timeBuf[32];
//     char durBuf[32];
//     formatTime(timeBuf, sizeof(timeBuf), cur);
//     formatTime(durBuf,  sizeof(durBuf),  dur);
//     ImGui::Text("%s / %s", timeBuf, durBuf);

//     ImGui::SameLine(0, 20);

//     // 播放 / 暂停
//     bool playing = engine.isPlaying();
//     ImGui::PushStyleColor(ImGuiCol_Button,      ImVec4(0.14f, 0.14f, 0.28f, 1.f));
//     ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.22f, 0.45f, 1.f));
//     ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.35f, 0.12f, 0.50f, 1.f));

//     if (ImGui::Button(playing ? " || " : " > ", ImVec2(40, 26))) {
//         if (playing) engine.pause();
//         else         engine.resume();
//     }
//     ImGui::SameLine(0, 4);

//     // 上一曲
//     if (ImGui::Button(" |< ", ImVec2(40, 26))) {
//         int idx = playlist.prev();
//         if (idx >= 0) app.playTrack(idx);
//     }
//     ImGui::SameLine(0, 4);

//     // 下一曲
//     if (ImGui::Button(" >| ", ImVec2(40, 26))) {
//         int idx = playlist.next();
//         if (idx >= 0) app.playTrack(idx);
//     }
//     ImGui::SameLine(0, 4);

//     // 停止
//     if (ImGui::Button("Stop", ImVec2(48, 26)))
//         engine.stop();

//     ImGui::SameLine(0, 20);

//     // 音量
//     ImGui::PushItemWidth(120);
//     ImGui::Text("Vol");
//     ImGui::SameLine();
//     if (ImGui::SliderFloat("##Vol", &volumeSlider_, 0.f, 1.f, "%.2f"))
//         engine.setVolume(volumeSlider_);
//     ImGui::PopItemWidth();

//     ImGui::PopStyleColor(3); // button colors

//     ImGui::End();
//     ImGui::PopStyleVar();   // WindowPadding
//     ImGui::PopStyleColor(); // WindowBg
// }

void UI::drawControls(App& app, float winW, float winH)
{
    auto& engine   = app.audio();
    auto& playlist = app.playlist();

    float panelX = leftPanelWidth_;
    float panelW = winW - panelX;
    float panelH = App::kControlsH;

    float panelY = winH - panelH;

    ImGui::SetNextWindowPos(ImVec2(panelX, panelY));
    ImGui::SetNextWindowSize(ImVec2(panelW, panelH));

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize   |
        ImGuiWindowFlags_NoMove     |
        ImGuiWindowFlags_NoScrollbar|
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ImGui::Begin("##controls", nullptr, flags);

    ImDrawList* draw = ImGui::GetWindowDrawList();

    ImVec2 pos  = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();

    //-----------------------------------------
    // 背景
    //-----------------------------------------

    draw->AddRectFilled(
        pos,
        ImVec2(pos.x + size.x, pos.y + size.y),
        IM_COL32(5, 5, 10, 220)
    );

    //-----------------------------------------
    // 时间
    //-----------------------------------------

    float cur = engine.getCursorSeconds();
    float dur = engine.getDurationSeconds();

    float progress = dur > 0.01f ? cur / dur : 0.f;

    char curBuf[32];
    char durBuf[32];

    formatTime(curBuf, sizeof(curBuf), cur);
    formatTime(durBuf, sizeof(durBuf), dur);

    //-----------------------------------------
    // 中心区域
    //-----------------------------------------

    float centerX = pos.x + size.x * 0.5f;

    //-----------------------------------------
    // 进度条
    //-----------------------------------------

    float barW = 500.f;
    float barY = pos.y + 42.f;

    ImVec2 barStart(centerX - barW * 0.5f, barY);
    ImVec2 barEnd(centerX + barW * 0.5f, barY);

    // 背景线
    draw->AddLine(
        barStart,
        barEnd,
        IM_COL32(80, 80, 90, 255),
        3.f
    );

    // 已播放
    draw->AddLine(
        barStart,
        ImVec2(barStart.x + barW * progress, barY),
        IM_COL32(210, 80, 255, 255),
        4.f
    );

    // 圆点
    draw->AddCircleFilled(
        ImVec2(barStart.x + barW * progress, barY),
        6.f,
        IM_COL32(220, 100, 255, 255)
    );

    //-----------------------------------------
    // 时间文字
    //-----------------------------------------

    draw->AddText(
        ImVec2(barStart.x - 55.f, barY - 8.f),
        IM_COL32_WHITE,
        curBuf
    );

    draw->AddText(
        ImVec2(barEnd.x + 15.f, barY - 8.f),
        IM_COL32_WHITE,
        durBuf
    );

    //-----------------------------------------
    // 按钮区域
    //-----------------------------------------

    float btnY = pos.y + 95.f;

    auto iconButton =
        [&](const char* id,
            const char* icon,
            ImVec2 p,
            float radius,
            bool filled = false)
    {
        ImGui::SetCursorScreenPos(
            ImVec2(p.x - radius, p.y - radius));

        ImGui::InvisibleButton(id,
            ImVec2(radius * 2, radius * 2));

        bool hovered = ImGui::IsItemHovered();

        if (filled)
        {
            draw->AddCircleFilled(
                p,
                radius,
                hovered
                    ? IM_COL32(220,100,255,255)
                    : IM_COL32(180,70,255,255)
            );

            draw->AddCircle(
                p,
                radius + 3,
                IM_COL32(220,100,255,120),
                0,
                2.f
            );
        }

        draw->AddText(
            ImVec2(
                p.x - 8.f,
                p.y - 10.f),
            IM_COL32_WHITE,
            icon);

        return ImGui::IsItemClicked();
    };

    //-----------------------------------------
    // 按钮坐标
    //-----------------------------------------

    float spacing = 70.f;

    ImVec2 shufflePos(centerX - spacing * 2.0f, btnY);
    ImVec2 prevPos   (centerX - spacing,       btnY);
    ImVec2 playPos   (centerX,                 btnY);
    ImVec2 nextPos   (centerX + spacing,       btnY);
    ImVec2 repeatPos (centerX + spacing * 2.f, btnY);

    //-----------------------------------------
    // Shuffle
    //-----------------------------------------

    iconButton(
        "shuffle",
        ICON_FA_SHUFFLE,
        shufflePos,
        16.f);

    //-----------------------------------------
    // Prev
    //-----------------------------------------

    if (iconButton(
        "prev",
        ICON_FA_BACKWARD,
        prevPos,
        18.f))
    {
        int idx = playlist.prev();
        if (idx >= 0)
            app.playTrack(idx);
    }

    //-----------------------------------------
    // Play
    //-----------------------------------------

    bool playing = engine.isPlaying();

    if (iconButton(
        "play",
        playing
            ? ICON_FA_PAUSE
            : ICON_FA_PLAY,
        playPos,
        32.f,
        true))
    {
        if (playing)
            engine.pause();
        else
            engine.resume();
    }

    //-----------------------------------------
    // Next
    //-----------------------------------------

    if (iconButton(
        "next",
        ICON_FA_FORWARD,
        nextPos,
        18.f))
    {
        int idx = playlist.next();
        if (idx >= 0)
            app.playTrack(idx);
    }

    //-----------------------------------------
    // Repeat
    //-----------------------------------------

    iconButton(
        "repeat",
        ICON_FA_REPEAT,
        repeatPos,
        16.f);

    //-----------------------------------------
    // 音量
    //-----------------------------------------

    float volX = pos.x + size.x - 220.f;

    draw->AddText(
        ImVec2(volX, btnY - 10.f),
        IM_COL32_WHITE,
        ICON_FA_VOLUME_HIGH);

    ImGui::SetCursorScreenPos(
        ImVec2(volX + 30.f, btnY - 8.f));

    ImGui::PushItemWidth(120);

    ImGui::PushStyleColor(
        ImGuiCol_SliderGrab,
        ImVec4(0.85f, 0.3f, 1.f, 1.f));

    ImGui::PushStyleColor(
        ImGuiCol_FrameBg,
        ImVec4(0.2f, 0.2f, 0.25f, 1.f));

    ImGui::PushStyleColor(
        ImGuiCol_FrameBgHovered,
        ImVec4(0.3f, 0.3f, 0.35f, 1.f));

    if (ImGui::SliderFloat(
        "##volume",
        &volumeSlider_,
        0.f,
        1.f,
        ""))
    {
        engine.setVolume(volumeSlider_);
    }

    ImGui::PopStyleColor(3);

    //-----------------------------------------
    // 文件名
    //-----------------------------------------

    // std::string title = playlist.currentName();

    // ImVec2 textSize =
    //     ImGui::CalcTextSize(title.c_str());

    // draw->AddText(
    //     ImVec2(centerX - textSize.x * 0.5f,
    //            pos.y + 5.f),
    //     IM_COL32_WHITE,
    //     title.c_str());

    ImGui::End();
    ImGui::PopStyleVar();
}

// ==========================================================
//  设置浮窗
// ==========================================================

void UI::drawSettings(App& app)
{
    ImGui::SetNextWindowSize(ImVec2(300, 240), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(App::kLeftPanelW + 40.f, 40.f), ImGuiCond_FirstUseEver);

    // ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.06f, 0.10f, 0.97f));

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

    // ImGui::PopStyleColor();
}

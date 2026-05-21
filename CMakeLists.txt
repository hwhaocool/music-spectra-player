cmake_minimum_required(VERSION 3.20)
project(MusicVisualizer LANGUAGES C CXX)
set(PROJECT_NAME MusicVisualizer)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── 路径常量 ────────────────────────────────────────────
set(EXTERN_DIR ${CMAKE_SOURCE_DIR}/third_party)

set(GLFW_DIR      ${EXTERN_DIR}/glfw)
set(IMGUI_DIR     ${EXTERN_DIR}/imgui)
set(GLAD_DIR      ${EXTERN_DIR}/glad)
set(MINIAUDIO_DIR ${EXTERN_DIR}/miniaudio)

# ── GLFW ──────────────────────────────────────────────
set(GLFW_INCLUDE_DIR ${GLFW_DIR}/include)
if(MINGW)
    set(GLFW_LIB_DIR ${GLFW_DIR}/lib-mingw-w64)
endif()

# 验证 lib 目录
if(NOT EXISTS ${GLFW_LIB_DIR})
    message(FATAL_ERROR
        "[依赖缺失] GLFW 预编译库目录不存在：${GLFW_LIB_DIR}\n"
        "请从 https://github.com/glfw/glfw/releases 下载对应平台的预编译包。")
endif()

add_library(glfw STATIC IMPORTED)
if(MINGW)
    set(GLFW_LIB_FILE "${GLFW_LIB_DIR}/libglfw3.a")
endif()
set_target_properties(glfw PROPERTIES
    IMPORTED_LOCATION             "${GLFW_LIB_FILE}"
    INTERFACE_INCLUDE_DIRECTORIES "${GLFW_INCLUDE_DIR}")

# Windows 下需要链接的系统库
set(GLFW_SYSTEM_LIBS opengl32 gdi32 user32 shell32)

set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# ── Dear ImGui ────────────────────────────────────────
set(IMGUI_DIR ${EXTERN_DIR}/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)

# ── GLAD (生成后放入 extern/glad) ────────────────────
set(GLAD_DIR ${CMAKE_SOURCE_DIR}/third_party/glad)

# ── miniaudio (单头文件) ─────────────────────────────
set(MINIAUDIO_DIR ${CMAKE_SOURCE_DIR}/third_party/miniaudio)

# ── 可执行文件 ────────────────────────────────────────
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/fft.cpp
    src/audio_engine.cpp
    src/vis.cpp
    src/bloom.cpp
    src/particles.cpp
    src/playlist.cpp
    src/ui.cpp
    src/app.cpp
    ${GLAD_DIR}/src/glad.c
    ${IMGUI_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    ${GLAD_DIR}/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${MINIAUDIO_DIR}
    ${CMAKE_SOURCE_DIR}/third_party/icons
)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw opengl32)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE dwmapi)
endif()

# 复制着色器到构建目录
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders)

# 静态链接
target_link_options(${PROJECT_NAME} PRIVATE -static)
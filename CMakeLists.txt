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
    src/vortex.cpp
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

# ── 从着色器源文件生成主题头文件 ──────────────────────
# 自动遍历 src/shaders/*/theme.cfg，每个子文件夹是一个主题
# theme.cfg 格式（3行）：struct名 / 显示名 / 基类

file(GLOB THEME_CFG_FILES "${CMAKE_SOURCE_DIR}/src/shaders/*/theme.cfg")

foreach(THEME_CFG ${THEME_CFG_FILES})
    get_filename_component(THEME_DIR "${THEME_CFG}" DIRECTORY)
    get_filename_component(THEME_DIRNAME "${THEME_DIR}" NAME)

    file(STRINGS "${THEME_CFG}" CFG_LINES)
    list(GET CFG_LINES 0 THEME_NAMESPACE)
    list(GET CFG_LINES 1 THEME_DISPLAY_NAME)
    list(GET CFG_LINES 2 THEME_BASE)

    set(THEME_HEADER "${THEME_DIR}/${THEME_DIRNAME}.h")

    # 按约定映射着色器文件 → 变量名
    set(SHADER_DEFS "")
    set(SHADER_DEPS  "")
    foreach(PAIR
            "vis.vert|kVert"
            "vis.frag|kFrag"
            "particle.vert|kParticleVert"
            "particle.frag|kParticleFrag")
        string(REPLACE "|" ";" KV "${PAIR}")
        list(GET KV 0 SHADER_FILE)
        list(GET KV 1 VAR_NAME)
        set(SHADER_PATH "${THEME_DIR}/${SHADER_FILE}")
        if(EXISTS "${SHADER_PATH}")
            list(APPEND SHADER_DEFS "${VAR_NAME}=${SHADER_PATH}")
            list(APPEND SHADER_DEPS  "${SHADER_PATH}")
        endif()
    endforeach()

    list(JOIN SHADER_DEFS ";" SHADERS_STR)

    add_custom_command(
        OUTPUT "${THEME_HEADER}"
        COMMAND ${CMAKE_COMMAND}
            "-DOUTPUT=${THEME_HEADER}"
            "-DNAMESPACE=${THEME_NAMESPACE}"
            "-DNAME=${THEME_DISPLAY_NAME}"
            "-DBASE=${THEME_BASE}"
            "-DSHADERS=${SHADERS_STR}"
            -P "${CMAKE_SOURCE_DIR}/cmake/generate_theme_header.cmake"
        DEPENDS
            ${SHADER_DEPS}
            "${CMAKE_SOURCE_DIR}/cmake/generate_theme_header.cmake"
            "${THEME_CFG}"
        COMMENT "Generating theme header: ${THEME_DIRNAME}.h"
    )

    list(APPEND ALL_GENERATED_HEADERS "${THEME_HEADER}")
endforeach()

if(ALL_GENERATED_HEADERS)
    add_custom_target(generate_theme_headers DEPENDS ${ALL_GENERATED_HEADERS})
    add_dependencies(${PROJECT_NAME} generate_theme_headers)
endif()

# 静态链接
target_link_options(${PROJECT_NAME} PRIVATE -static)
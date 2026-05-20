#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <string>
#include <filesystem>

// UTF-8 → std::wstring
inline std::wstring utf8ToWide(const char* utf8)
{
    if (!utf8 || !*utf8) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
    if (len <= 0) return {};
    std::wstring w(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, &w[0], len);
    return w;
}

// std::wstring → UTF-8
inline std::string wideToUtf8(const wchar_t* wide)
{
    if (!wide || !*wide) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, wide, -1,
                                  nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string s(len - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide, -1,
                        &s[0], len, nullptr, nullptr);
    return s;
}

// 从 UTF-8 路径字符串正确构造 std::filesystem::path
inline std::filesystem::path utf8Path(const char* utf8)
{
    return std::filesystem::path(utf8ToWide(utf8));
}

inline std::filesystem::path utf8Path(const std::string& utf8)
{
    return utf8Path(utf8.c_str());
}

// 从 UTF-8 路径提取文件名（纯字符串操作，不经过 fs::path 窄字符构造）
inline std::string utf8DisplayName(const std::string& utf8Path)
{
    auto pos = utf8Path.find_last_of("/\\");
    return (pos != std::string::npos) ? utf8Path.substr(pos + 1) : utf8Path;
}

// 设置控制台为 UTF-8（程序启动时调用一次）
inline void setupConsoleUtf8()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    // 允许 ANSI 转义序列（Windows 10+）
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode))
            SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}

#else
// Linux / macOS 不需要转换
#include <string>
#include <filesystem>

inline std::filesystem::path utf8Path(const char* s)
    { return std::filesystem::path(s); }
inline std::filesystem::path utf8Path(const std::string& s)
    { return std::filesystem::path(s); }
inline std::string utf8DisplayName(const std::string& p)
    { auto pos = p.find_last_of("/\\");
      return (pos != std::string::npos) ? p.substr(pos+1) : p; }
inline void setupConsoleUtf8() {}
#endif

#pragma once
#include "ring_buffer.h"
#include <string>
#include <atomic>
#include <functional>
#include <cstdint>
#include <mutex>
#include <thread>
#include "miniaudio.h"

// Forward declare to avoid exposing miniaudio.h everywhere
struct ma_engine;
struct ma_sound;
struct ma_device;

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    bool init();
    void shutdown();

    // 播放控制
    bool loadAndPlay(const std::string& path);
    void pause();
    void resume();
    void stop();
    bool isPlaying() const;
    bool isLoaded() const { return loaded_.load(); }

    // 进度
    float getCursorSeconds() const;
    float getDurationSeconds() const;

    // 音量 0‑1
    void  setVolume(float v);
    float getVolume() const { return volume_.load(); }

    // 频谱数据
    RingBuffer&       pcmRing()       { return pcmRing_; }
    const RingBuffer& pcmRing() const { return pcmRing_; }
    

private:
    // 不加锁的内部版本（调用方必须已持有 mtx_）
    void stopLocked();

    // capture 回调
    static void dataCallback(ma_device* pDevice, void* pOutput,
                             const void* pInput, uint32_t frameCount);
    void onPCMAudioFrames(const float* input, uint32_t frameCount);

    ma_engine*   engine_  = nullptr;
    ma_sound*    sound_   = nullptr;
    ma_device*   device_  = nullptr;

    RingBuffer           pcmRing_{16384};
    std::atomic<bool>    loaded_{false};
    std::atomic<bool>    playing_{false};
    std::atomic<float>   volume_{0.8f};

    mutable std::mutex   mtx_;

    std::vector<uint8_t>  fileData_;          // 文件原始字节（保持存活）
    ma_decoder*           decoder_ = nullptr;  // 内存解码器

    // 读取文件到内存（内部处理 UTF-8 中文路径）
    static std::vector<uint8_t> readFileToMemory(const std::string& path);
};
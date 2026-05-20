#include "audio_engine.h"
#include "math_utils.h"

// miniaudio 单编译单元实现
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <iostream>
#include <algorithm>

// ── 全局单例指针（供 C 回调访问 C++ 对象）──────────────
static AudioEngine* g_engineInstance = nullptr;

AudioEngine::AudioEngine()  = default;
AudioEngine::~AudioEngine() { shutdown(); }

bool AudioEngine::init()
{
    g_engineInstance = this;

    // ── 初始化混音引擎 ──
    engine_ = new ma_engine;
    ma_engine_config ecfg = ma_engine_config_init();
    ecfg.channels   = 2;
    ecfg.sampleRate = 48000;
    if (ma_engine_init(&ecfg, engine_) != MA_SUCCESS) {
        std::cerr << "[Audio] Failed to init engine\n";
        delete engine_; engine_ = nullptr;
        return false;
    }

    // ── 初始化捕获设备（loopback：捕获系统混音输出）──
    device_ = new ma_device;
    ma_device_config dcfg = ma_device_config_init(ma_device_type_loopback);
    dcfg.capture.pDeviceID  = nullptr;  // 默认输出设备
    dcfg.capture.format     = ma_format_f32;
    dcfg.capture.channels   = 2;
    dcfg.sampleRate         = 48000;
    dcfg.dataCallback       = dataCallback;
    dcfg.pUserData          = this;

    if (ma_device_init(nullptr, &dcfg, device_) != MA_SUCCESS) {
        std::cerr << "[Audio] Loopback init failed, trying NULL capture\n";
        // Fallback：不使用 loopback，频谱将不显示
        delete device_; device_ = nullptr;
    } else {
        ma_device_start(device_);
    }

    std::cout << "[Audio] Initialized (loopback "
              << (device_ ? "OK" : "unavailable") << ")\n";
    return true;
}

// ── 内部版本：不加锁，调用方必须已持有 mtx_ ──
void AudioEngine::stopLocked()
{
    if (sound_) {
        ma_sound_stop(sound_);
        ma_sound_uninit(sound_);
        delete sound_;
        sound_ = nullptr;
    }
    loaded_.store(false);
    playing_.store(false);
}

// ── 公开版本：加锁后委托 ──
void AudioEngine::stop()
{
    std::lock_guard<std::mutex> lk(mtx_);
    stopLocked();
}

// void AudioEngine::stop() {
//     std::lock_guard<std::mutex> lk(mtx_);
//     if (sound_) {
//         ma_sound_uninit(sound_);
//         delete sound_;
//         sound_ = nullptr;
//     }
//     loaded_.store(false);
//     playing_.store(false);
// }

void AudioEngine::shutdown()
{
    stop();
    if (device_) { ma_device_uninit(device_); delete device_; device_ = nullptr; }
    if (engine_) { ma_engine_uninit(engine_); delete engine_; engine_ = nullptr; }
    g_engineInstance = nullptr;
}

bool AudioEngine::loadAndPlay(const std::string& path)
{
    std::lock_guard<std::mutex> lk(mtx_);

    // 关键修复：调用 stopLocked() 而非 stop()，避免死锁
    stopLocked();

    sound_ = new ma_sound;
    if (ma_sound_init_from_file(engine_, path.c_str(), 0, nullptr, nullptr, sound_) != MA_SUCCESS) {
        std::cerr << "[Audio] Failed to load: " << path << "\n";
        delete sound_; sound_ = nullptr;
        return false;
    }
    ma_sound_set_volume(sound_, volume_.load());
    ma_sound_start(sound_);
    loaded_.store(true);
    playing_.store(true);
    pcmRing_.clear();
    std::cout << "[Audio] Playing: " << path << "\n";
    return true;
}

void AudioEngine::pause() {
    std::lock_guard<std::mutex> lk(mtx_);
    if (sound_ && playing_.load()) {
        ma_sound_stop(sound_);
        playing_.store(false);
    }
}

void AudioEngine::resume() {
    std::lock_guard<std::mutex> lk(mtx_);
    if (sound_ && !playing_.load()) {
        ma_sound_start(sound_);
        playing_.store(true);
    }
}



bool AudioEngine::isPlaying() const { return playing_.load(); }

float AudioEngine::getCursorSeconds() const {
    std::lock_guard<std::mutex> lk(const_cast<std::mutex&>(mtx_));
    if (!sound_) return 0.f;
    float pos = 0.f;
    ma_sound_get_cursor_in_seconds(sound_, &pos);
    return pos;
}

float AudioEngine::getDurationSeconds() const {
    std::lock_guard<std::mutex> lk(const_cast<std::mutex&>(mtx_));
    if (!sound_) return 0.f;
    float dur = 0.f;
    ma_sound_get_length_in_seconds(sound_, &dur);
    return dur;
}

void AudioEngine::setVolume(float v) {
    volume_.store(clampf(v, 0.f, 1.f));
    std::lock_guard<std::mutex> lk(mtx_);
    if (sound_) ma_sound_set_volume(sound_, volume_.load());
}

// ── loopback 回调：把系统音频写入 PCM 环形缓冲 ────────
void AudioEngine::dataCallback(ma_device* pDevice, void* /*pOutput*/,
                                const void* pInput, ma_uint32 frameCount)
{
    auto* self = static_cast<AudioEngine*>(pDevice->pUserData);
    if (self && pInput)
        self->onPCMAudioFrames(static_cast<const float*>(pInput), frameCount);
}

void AudioEngine::onPCMAudioFrames(const float* input, uint32_t frameCount)
{
    // 立体声→单声道取平均
    std::vector<float> mono(frameCount);
    for (uint32_t i = 0; i < frameCount; ++i)
        mono[i] = (input[i * 2] + input[i * 2 + 1]) * 0.5f;
    pcmRing_.write(mono.data(), frameCount);
}

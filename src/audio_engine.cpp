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
    if (decoder_) {                     // ← 新增
        ma_decoder_uninit(decoder_);
        delete decoder_;
        decoder_ = nullptr;
    }
    fileData_.clear();                  // ← 新增：释放文件内存
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
    // ① 清理上一首
    stopLocked();

    // ② 把整个文件读进内存
    fileData_ = readFileToMemory(path);
    if (fileData_.empty()) {
        std::cerr << "[Audio] Failed to read file: " << path << "\n";
        return false;
    }

    // ③ 从内存初始化解码器
    decoder_ = new ma_decoder;
    ma_decoder_config dcfg = ma_decoder_config_init(ma_format_f32, 2, 48000);
    if (ma_decoder_init_memory(fileData_.data(), fileData_.size(),
                                &dcfg, decoder_) != MA_SUCCESS) {
        std::cerr << "[Audio] Failed to decode: " << path << "\n";
        delete decoder_; decoder_ = nullptr;
        fileData_.clear();
        return false;
    }

    // ④ 从解码器（data source）创建声音
    sound_ = new ma_sound;
    if (ma_sound_init_from_data_source(engine_, decoder_,
                                        0, nullptr, sound_) != MA_SUCCESS) {
        std::cerr << "[Audio] Failed to create sound\n";
        ma_decoder_uninit(decoder_);
        delete decoder_; decoder_ = nullptr;
        delete sound_;   sound_   = nullptr;
        fileData_.clear();
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

std::vector<uint8_t> AudioEngine::readFileToMemory(const std::string& path)
{
    std::vector<uint8_t> buf;

#ifdef _WIN32
    // UTF-8 → UTF-16
    int wlen = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, nullptr, 0);
    if (wlen <= 0) return buf;
    std::vector<wchar_t> wp(wlen);
    MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, wp.data(), wlen);

    FILE* fp = _wfopen(wp.data(), L"rb");
#else
    FILE* fp = fopen(path.c_str(), "rb");
#endif

    if (!fp) return buf;

    fseek(fp, 0, SEEK_END);
    auto sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (sz > 0) {
        buf.resize(static_cast<size_t>(sz));
        fread(buf.data(), 1, buf.size(), fp);
    }
    fclose(fp);
    return buf;
}

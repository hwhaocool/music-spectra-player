#pragma once

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <vector>
#include <string>
#include <mutex>

class AudioEngine
{
public:
    bool load(const std::string& path);

    void play();
    void stop();

    const std::vector<float>& getPCM() const;

    bool isPlaying() const;

private:
    static void dataCallback(
        ma_device* device,
        void* output,
        const void* input,
        ma_uint32 frameCount
    );

private:
    ma_decoder decoder;
    ma_device device;

    std::vector<float> pcmBuffer;

    mutable std::mutex mutex;

    bool loaded = false;
    bool playing = false;
};
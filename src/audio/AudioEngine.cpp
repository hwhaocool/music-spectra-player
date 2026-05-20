#include "AudioEngine.h"

bool AudioEngine::load(const std::string& path)
{
    if (loaded)
    {
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);
    }

    ma_result result =
        ma_decoder_init_file(path.c_str(), NULL, &decoder);

    if (result != MA_SUCCESS)
        return false;

    ma_device_config config =
        ma_device_config_init(ma_device_type_playback);

    config.playback.format = decoder.outputFormat;
    config.playback.channels = decoder.outputChannels;
    config.sampleRate = decoder.outputSampleRate;

    config.dataCallback = dataCallback;
    config.pUserData = this;

    result = ma_device_init(NULL, &config, &device);

    if (result != MA_SUCCESS)
        return false;

    loaded = true;

    return true;
}

void AudioEngine::play()
{
    if (!loaded)
        return;

    ma_device_start(&device);

    playing = true;
}

void AudioEngine::stop()
{
    ma_device_stop(&device);

    playing = false;
}

void AudioEngine::dataCallback(
    ma_device* device,
    void* output,
    const void* input,
    ma_uint32 frameCount)
{
    AudioEngine* self =
        (AudioEngine*)device->pUserData;

    ma_decoder_read_pcm_frames(
        &self->decoder,
        output,
        frameCount,
        NULL
    );

    float* samples = (float*)output;

    std::lock_guard<std::mutex> lock(self->mutex);

    for (ma_uint32 i = 0; i < frameCount; i++)
    {
        self->pcmBuffer.push_back(samples[i * 2]);

        if (self->pcmBuffer.size() > 4096)
            self->pcmBuffer.erase(self->pcmBuffer.begin());
    }

    (void)input;
}

const std::vector<float>& AudioEngine::getPCM() const
{
    return pcmBuffer;
}
#include "FFTProcessor.h"

#include "kiss_fft.h"

#include <cmath>

FFTProcessor::FFTProcessor()
{
    spectrum.resize(512);
}

void FFTProcessor::process(const std::vector<float>& pcm)
{
    const int FFT_SIZE = 1024;

    if (pcm.size() < FFT_SIZE)
        return;

    static kiss_fft_cfg cfg =
        kiss_fft_alloc(FFT_SIZE, 0, NULL, NULL);

    std::vector<kiss_fft_cpx> in(FFT_SIZE);
    std::vector<kiss_fft_cpx> out(FFT_SIZE);

    for (int i = 0; i < FFT_SIZE; i++)
    {
        in[i].r = pcm[i];
        in[i].i = 0;
    }

    kiss_fft(cfg, in.data(), out.data());

    for (int i = 0; i < 512; i++)
    {
        float real = out[i].r;
        float imag = out[i].i;

        float mag = sqrt(real * real + imag * imag);

        mag = log10f(mag + 1.0f) * 25.0f;

        spectrum[i] =
            spectrum[i] * 0.92f +
            mag * 0.08f;
    }
}

const std::vector<float>& FFTProcessor::getSpectrum() const
{
    return spectrum;
}
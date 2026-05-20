
// ─── fft.h ───
#pragma once
#include <vector>
#include <complex>

class FFTProcessor {
public:
    FFTProcessor(int fftSize = 1024, int outBins = 128);
    void process(const float* samples, int count);
    const float* getSpectrum() const { return smoothed_.data(); }
    int   getNumBins()         const { return outBins_; }
private:
    int fftSize_, outBins_;
    std::vector<std::complex<float>> buf_;
    std::vector<float> win_, raw_, smoothed_;
    void computeFFT();
};

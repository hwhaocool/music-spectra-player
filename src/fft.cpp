#include "fft.h"
#include "math_utils.h"
#include <cmath>
#include <algorithm>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

FFTProcessor::FFTProcessor(int n, int bins)
    : fftSize_(n), outBins_(bins),
      buf_(n), win_(n), raw_(n / 2), smoothed_(bins, 0.f)
{
    for (int i = 0; i < n; ++i)
        win_[i] = 0.5f * (1.f - std::cos(2.0 * M_PI * i / (n - 1)));
}

void FFTProcessor::process(const float* s, int cnt)
{
    int n = std::min(cnt, fftSize_);
    for (int i = 0; i < fftSize_; ++i)
        buf_[i] = (i < n) ? std::complex<float>(s[i] * win_[i], 0.f)
                          : std::complex<float>(0.f, 0.f);
    computeFFT();

    int half = fftSize_ / 2;
    for (int i = 0; i < half; ++i)
        raw_[i] = std::abs(buf_[i]);

    // 对数频率映射：把 half 个 bin 合并到 outBins_ 个
    for (int b = 0; b < outBins_; ++b) {
        float lo = std::pow((float)half, (float)b       / outBins_);
        float hi = std::pow((float)half, (float)(b + 1) / outBins_);
        int iLo  = std::max(1, (int)lo);
        int iHi  = std::min(half - 1, (int)hi);
        if (iHi <= iLo) iHi = iLo + 1;

        float sum = 0.f;
        int   cnt2 = 0;
        for (int i = iLo; i < iHi; ++i) { sum += raw_[i]; ++cnt2; }
        float mag = (cnt2 > 0) ? sum / cnt2 : 0.f;

        // 归一化 + 限幅
        mag = clampf(mag / (fftSize_ * 0.12f), 0.f, 1.f);

        // 时间平滑
        smoothed_[b] = lerpf(smoothed_[b], mag, 0.35f);
    }
}

void FFTProcessor::computeFFT()
{
    // 迭代 Cooley‑Tukey FFT（位反转 + 蝶形）
    int n = fftSize_;
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(buf_[i], buf_[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        float ang = -2.f * (float)M_PI / len;
        std::complex<float> wn(std::cos(ang), std::sin(ang));
        for (int i = 0; i < n; i += len) {
            std::complex<float> w(1.f, 0.f);
            for (int j = 0; j < len / 2; ++j) {
                auto u = buf_[i + j];
                auto t = w * buf_[i + j + len / 2];
                buf_[i + j]            = u + t;
                buf_[i + j + len / 2]  = u - t;
                w *= wn;
            }
        }
    }
}
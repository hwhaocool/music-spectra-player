#pragma once

#include <vector>

class FFTProcessor
{
public:
    FFTProcessor();

    void process(const std::vector<float>& pcm);

    const std::vector<float>& getSpectrum() const;

private:
    std::vector<float> spectrum;
};
// 环形缓冲
#pragma once
#include <vector>
#include <atomic>
#include <algorithm>

class RingBuffer {
public:
    explicit RingBuffer(size_t cap) : buf_(cap, 0.f), cap_(cap) {}

    void write(const float* data, size_t n) {
        size_t wp = wp_.load(std::memory_order_relaxed);
        for (size_t i = 0; i < n; ++i) buf_[(wp+i)%cap_] = data[i];
        wp_.store(wp + n, std::memory_order_release);
    }

    void readLatest(float* out, size_t n) const {
        size_t wp   = wp_.load(std::memory_order_acquire);
        size_t have  = std::min(n, cap_);
        size_t start = wp > have ? wp - have : 0;
        for (size_t i = 0; i < have; ++i) out[i] = buf_[(start+i)%cap_];
        for (size_t i = have; i < n; ++i)  out[i] = 0.f;
    }

    void clear() {
        wp_.store(0, std::memory_order_release);
        std::fill(buf_.begin(), buf_.end(), 0.f);
    }

private:
    std::vector<float>       buf_;
    size_t                   cap_;
    std::atomic<size_t>      wp_{0};
};

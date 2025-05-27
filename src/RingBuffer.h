#pragma once
#include <atomic>
#include <memory>

class RingBuffer {
public:
    explicit RingBuffer(size_t size);
    ~RingBuffer();
    
    void write(const float* data, size_t numSamples);
    void read(float* data, size_t numSamples);
    size_t getSize() const { return size_; }
    size_t getAvailableForWrite() const;
    size_t getAvailableForRead() const;
    
private:
    std::unique_ptr<float[]> buffer_;
    size_t size_;
    std::atomic<size_t> writeIndex_;
    std::atomic<size_t> readIndex_;
};
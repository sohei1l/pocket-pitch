#include "RingBuffer.h"
#include <algorithm>
#include <cstring>

RingBuffer::RingBuffer(size_t size) 
    : size_(size)
    , writeIndex_(0)
    , readIndex_(0) {
    buffer_ = std::make_unique<float[]>(size_);
    std::memset(buffer_.get(), 0, size_ * sizeof(float));
}

RingBuffer::~RingBuffer() = default;

void RingBuffer::write(const float* data, size_t numSamples) {
    if (!data || numSamples == 0) return;
    
    size_t currentWrite = writeIndex_.load(std::memory_order_relaxed);
    
    for (size_t i = 0; i < numSamples; ++i) {
        buffer_[currentWrite] = data[i];
        currentWrite = (currentWrite + 1) % size_;
    }
    
    writeIndex_.store(currentWrite, std::memory_order_release);
}

void RingBuffer::read(float* data, size_t numSamples) {
    if (!data || numSamples == 0) return;
    
    size_t currentRead = readIndex_.load(std::memory_order_relaxed);
    
    for (size_t i = 0; i < numSamples; ++i) {
        data[i] = buffer_[currentRead];
        currentRead = (currentRead + 1) % size_;
    }
    
    readIndex_.store(currentRead, std::memory_order_release);
}

size_t RingBuffer::getAvailableForWrite() const {
    size_t write = writeIndex_.load(std::memory_order_acquire);
    size_t read = readIndex_.load(std::memory_order_acquire);
    
    if (write >= read) {
        return size_ - (write - read) - 1;
    } else {
        return read - write - 1;
    }
}

size_t RingBuffer::getAvailableForRead() const {
    size_t write = writeIndex_.load(std::memory_order_acquire);
    size_t read = readIndex_.load(std::memory_order_acquire);
    
    if (write >= read) {
        return write - read;
    } else {
        return size_ - (read - write);
    }
}
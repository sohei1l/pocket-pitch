#pragma once
#include "RingBuffer.h"
#include <cmath>

class PitchShifter {
public:
    PitchShifter(size_t bufferSize, float sampleRate);
    ~PitchShifter();
    
    void setPitchRatio(float ratio);
    void setMixLevel(float mix);  // 0.0 = dry, 1.0 = wet
    void processBlock(const float* input, float* output, size_t numSamples);
    
private:
    float linearInterpolate(float a, float b, float fraction);
    float getGrainWindow(float phase);
    float applyLowpassFilter(float input);
    
    RingBuffer* buffer_;
    float sampleRate_;
    size_t bufferSize_;
    
    float pitchRatio_;
    float mixLevel_;
    float grainSize_;
    
    // Two read heads for cross-fading
    float readHead1_;
    float readHead2_;
    float grainPhase1_;
    float grainPhase2_;
    
    bool grain1Active_;
    size_t samplesSinceLastGrain_;
    size_t grainOverlap_;
    
    // Simple FIR lowpass filter (3-tap)
    float filterHistory_[3];
    float filterCoeffs_[3];
};
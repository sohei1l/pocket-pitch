#include "PitchShifter.h"
#include <algorithm>
#include <cstring>

PitchShifter::PitchShifter(size_t bufferSize, float sampleRate)
    : sampleRate_(sampleRate)
    , bufferSize_(bufferSize)
    , pitchRatio_(1.0f)
    , mixLevel_(1.0f)  // Default to 100% wet
    , grainSize_(1024.0f)
    , readHead1_(0.0f)
    , readHead2_(0.0f)
    , grainPhase1_(0.0f)
    , grainPhase2_(0.5f)  // Start second grain halfway through
    , grain1Active_(true)
    , samplesSinceLastGrain_(0)
    , grainOverlap_(512) {
    
    // Create a larger buffer for pitch shifting (4x size for good margin)
    buffer_ = new RingBuffer(bufferSize * 4);
    
    // Initialize lowpass filter (simple 3-tap FIR)
    filterHistory_[0] = filterHistory_[1] = filterHistory_[2] = 0.0f;
    // Normalized coefficients for simple averaging filter
    filterCoeffs_[0] = 0.25f;
    filterCoeffs_[1] = 0.5f;
    filterCoeffs_[2] = 0.25f;
}

PitchShifter::~PitchShifter() {
    delete buffer_;
}

void PitchShifter::setPitchRatio(float ratio) {
    // Clamp to ±1 octave (0.5 to 2.0)
    pitchRatio_ = std::max(0.5f, std::min(2.0f, ratio));
}

void PitchShifter::setMixLevel(float mix) {
    mixLevel_ = std::max(0.0f, std::min(1.0f, mix));
}

float PitchShifter::linearInterpolate(float a, float b, float fraction) {
    return a + fraction * (b - a);
}

float PitchShifter::getGrainWindow(float phase) {
    // Simple Hann window
    if (phase < 0.0f || phase > 1.0f) return 0.0f;
    return 0.5f * (1.0f - std::cos(2.0f * M_PI * phase));
}

float PitchShifter::applyLowpassFilter(float input) {
    // Shift history
    filterHistory_[2] = filterHistory_[1];
    filterHistory_[1] = filterHistory_[0];
    filterHistory_[0] = input;
    
    // Apply FIR filter
    return filterCoeffs_[0] * filterHistory_[0] + 
           filterCoeffs_[1] * filterHistory_[1] + 
           filterCoeffs_[2] * filterHistory_[2];
}

void PitchShifter::processBlock(const float* input, float* output, size_t numSamples) {
    // Write input to buffer
    buffer_->write(input, numSamples);
    
    for (size_t i = 0; i < numSamples; ++i) {
        float sample1 = 0.0f, sample2 = 0.0f;
        
        // Process grain 1
        if (grainPhase1_ >= 0.0f && grainPhase1_ < 1.0f) {
            // Calculate buffer read position for grain 1
            float bufferPos = readHead1_;
            size_t pos1 = static_cast<size_t>(bufferPos) % buffer_->getSize();
            size_t pos2 = (pos1 + 1) % buffer_->getSize();
            
            // Get samples for interpolation
            float tempBuffer[2];
            buffer_->read(tempBuffer, 1);  // Read current sample
            float samp1 = tempBuffer[0];
            
            // Simple approximation for interpolation (would need better buffer access in real implementation)
            float fraction = bufferPos - std::floor(bufferPos);
            sample1 = samp1 * getGrainWindow(grainPhase1_);
            
            readHead1_ += pitchRatio_;
            if (readHead1_ >= buffer_->getSize()) {
                readHead1_ -= buffer_->getSize();
            }
        }
        
        // Process grain 2
        if (grainPhase2_ >= 0.0f && grainPhase2_ < 1.0f) {
            // Calculate buffer read position for grain 2
            float bufferPos = readHead2_;
            size_t pos1 = static_cast<size_t>(bufferPos) % buffer_->getSize();
            
            // Get sample for grain 2
            float tempBuffer[2];
            buffer_->read(tempBuffer, 1);
            float samp2 = tempBuffer[0];
            
            sample2 = samp2 * getGrainWindow(grainPhase2_);
            
            readHead2_ += pitchRatio_;
            if (readHead2_ >= buffer_->getSize()) {
                readHead2_ -= buffer_->getSize();
            }
        }
        
        // Cross-fade between grains and apply anti-aliasing filter
        float wetSample = applyLowpassFilter(sample1 + sample2);
        
        // Mix dry and wet signals
        output[i] = (1.0f - mixLevel_) * input[i] + mixLevel_ * wetSample;
        
        // Update grain phases
        grainPhase1_ += 1.0f / grainSize_;
        grainPhase2_ += 1.0f / grainSize_;
        
        // Reset grains when they complete
        if (grainPhase1_ >= 1.0f) {
            grainPhase1_ = -0.5f;  // Start next grain with overlap
            readHead1_ = readHead2_ - grainOverlap_;
            if (readHead1_ < 0) readHead1_ += buffer_->getSize();
        }
        
        if (grainPhase2_ >= 1.0f) {
            grainPhase2_ = -0.5f;  // Start next grain with overlap
            readHead2_ = readHead1_ - grainOverlap_;
            if (readHead2_ < 0) readHead2_ += buffer_->getSize();
        }
    }
}
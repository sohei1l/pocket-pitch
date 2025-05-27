#include "SpectralMeter.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>

SpectralMeter::SpectralMeter(size_t fftSize) 
    : fftSize_(fftSize)
    , sampleCount_(0)
    , displayCounter_(0) {
    
    inputBuffer_.resize(fftSize_);
    fftBuffer_.resize(fftSize_);
    magnitude_.resize(fftSize_ / 2);
    window_ = generateHannWindow(fftSize_);
    
    std::fill(inputBuffer_.begin(), inputBuffer_.end(), 0.0f);
}

SpectralMeter::~SpectralMeter() = default;

std::vector<float> SpectralMeter::generateHannWindow(size_t size) {
    std::vector<float> window(size);
    for (size_t i = 0; i < size; ++i) {
        window[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (size - 1)));
    }
    return window;
}

void SpectralMeter::addSample(float sample) {
    inputBuffer_[sampleCount_] = sample;
    sampleCount_ = (sampleCount_ + 1) % fftSize_;
    
    // Update display every ~50ms (assuming 44.1kHz sample rate)
    displayCounter_++;
    if (displayCounter_ >= 2205) {  // 44100 / 20 = 2205 samples for 50ms
        updateDisplay();
        displayCounter_ = 0;
    }
}

void SpectralMeter::updateDisplay() {
    performFFT();
    printSpectrum();
}

void SpectralMeter::performFFT() {
    // Apply window and copy to FFT buffer
    for (size_t i = 0; i < fftSize_; ++i) {
        size_t bufferIndex = (sampleCount_ + i) % fftSize_;
        fftBuffer_[i] = std::complex<float>(inputBuffer_[bufferIndex] * window_[i], 0.0f);
    }
    
    // Perform FFT
    fft(fftBuffer_);
    
    // Calculate magnitude spectrum
    for (size_t i = 0; i < magnitude_.size(); ++i) {
        magnitude_[i] = std::abs(fftBuffer_[i]);
    }
}

size_t SpectralMeter::bitReversedIndex(size_t index, size_t bits) {
    size_t reversed = 0;
    for (size_t i = 0; i < bits; ++i) {
        reversed = (reversed << 1) | (index & 1);
        index >>= 1;
    }
    return reversed;
}

void SpectralMeter::bitReverse(std::vector<std::complex<float>>& data) {
    size_t n = data.size();
    size_t bits = 0;
    size_t temp = n;
    while (temp > 1) {
        temp >>= 1;
        bits++;
    }
    
    for (size_t i = 0; i < n; ++i) {
        size_t j = bitReversedIndex(i, bits);
        if (i < j) {
            std::swap(data[i], data[j]);
        }
    }
}

void SpectralMeter::fft(std::vector<std::complex<float>>& data) {
    size_t n = data.size();
    
    // Bit-reverse the input
    bitReverse(data);
    
    // Perform FFT
    for (size_t len = 2; len <= n; len <<= 1) {
        float angle = -2.0f * M_PI / len;
        std::complex<float> wlen(std::cos(angle), std::sin(angle));
        
        for (size_t i = 0; i < n; i += len) {
            std::complex<float> w(1.0f, 0.0f);
            
            for (size_t j = 0; j < len / 2; ++j) {
                std::complex<float> u = data[i + j];
                std::complex<float> v = data[i + j + len / 2] * w;
                
                data[i + j] = u + v;
                data[i + j + len / 2] = u - v;
                
                w *= wlen;
            }
        }
    }
}

void SpectralMeter::printSpectrum() {
    const int barWidth = 60;
    const int numBins = 32;  // Show 32 frequency bins
    
    // Group bins for display
    std::vector<float> displayBins(numBins, 0.0f);
    size_t binsPerDisplay = magnitude_.size() / numBins;
    
    for (int i = 0; i < numBins; ++i) {
        float sum = 0.0f;
        for (size_t j = 0; j < binsPerDisplay; ++j) {
            size_t binIndex = i * binsPerDisplay + j;
            if (binIndex < magnitude_.size()) {
                sum += magnitude_[binIndex];
            }
        }
        displayBins[i] = sum / binsPerDisplay;
    }
    
    // Find max for scaling
    float maxVal = *std::max_element(displayBins.begin(), displayBins.end());
    if (maxVal < 1e-6f) maxVal = 1e-6f;  // Avoid division by zero
    
    // Clear screen and move cursor to top
    std::cout << "\033[2J\033[H";
    
    std::cout << "Pocket Pitch - Spectral Meter\n";
    std::cout << "Frequency Analysis (32 bins, " << fftSize_ << "-point FFT)\n";
    std::cout << std::string(barWidth + 10, '=') << "\n";
    
    // Print spectrum bars
    for (int i = 0; i < numBins; ++i) {
        float normalized = displayBins[i] / maxVal;
        int barLength = static_cast<int>(normalized * barWidth);
        
        // Calculate frequency range for this bin
        float freqStart = (i * 22050.0f) / numBins;  // Nyquist = 22050 Hz
        float freqEnd = ((i + 1) * 22050.0f) / numBins;
        
        std::cout << std::setw(4) << static_cast<int>(freqStart) << "Hz |";
        
        // Print the bar
        for (int j = 0; j < barWidth; ++j) {
            if (j < barLength) {
                if (j < barWidth * 0.6f) {
                    std::cout << "█";  // Full block
                } else if (j < barWidth * 0.8f) {
                    std::cout << "▓";  // Medium block
                } else {
                    std::cout << "░";  // Light block
                }
            } else {
                std::cout << " ";
            }
        }
        
        std::cout << "| " << std::fixed << std::setprecision(3) << normalized << "\n";
    }
    
    std::cout << std::string(barWidth + 10, '=') << "\n";
    std::cout << "Press Enter to quit...\n";
}
#pragma once
#include <vector>
#include <complex>

class SpectralMeter {
public:
    SpectralMeter(size_t fftSize = 1024);
    ~SpectralMeter();
    
    void addSample(float sample);
    void updateDisplay();
    
private:
    void performFFT();
    void printSpectrum();
    std::vector<float> generateHannWindow(size_t size);
    
    size_t fftSize_;
    size_t sampleCount_;
    size_t displayCounter_;
    
    std::vector<float> inputBuffer_;
    std::vector<float> window_;
    std::vector<std::complex<float>> fftBuffer_;
    std::vector<float> magnitude_;
    
    // Simple decimation-in-time FFT implementation
    void fft(std::vector<std::complex<float>>& data);
    void bitReverse(std::vector<std::complex<float>>& data);
    size_t bitReversedIndex(size_t index, size_t bits);
};
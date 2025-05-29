#include <iostream>
#include <RtAudio.h>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include "RingBuffer.h"
#include "PitchShifter.h"
#include "SpectralMeter.h"

#define POCKET_PITCH_VERSION "1.0.0"

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options]\n"
              << "Options:\n"
              << "  -s, --semitones <value>   Pitch shift in semitones [-12 to +12] (default: 5)\n"
              << "  -m, --mix <value>         Wet/dry mix [0.0 to 1.0] (default: 1.0)\n"
              << "  -g, --gain <value>        Output gain [0.1 to 2.0] (default: 1.0)\n"
              << "  -p, --preset <name>       Use preset (octave-up, octave-down, fifth-up, chipmunk, deep)\n"
              << "  -f, --fft                 Enable spectral meter visualization\n"
              << "  -v, --version             Show version information\n"
              << "  -h, --help                Show this help message\n"
              << "\nPresets:\n"
              << "  octave-up    +12 semitones (double pitch)\n"
              << "  octave-down  -12 semitones (half pitch)\n"
              << "  fifth-up     +7 semitones (perfect fifth)\n"
              << "  chipmunk     +8 semitones (high cartoon voice)\n"
              << "  deep         -5 semitones (lower voice)\n"
              << "\nExamples:\n"
              << "  " << programName << " -p chipmunk -m 0.8   # Chipmunk preset with 80% mix\n"
              << "  " << programName << " -s 7 -m 0.5 -g 1.5   # +7 semitones, 50% mix, +3dB gain\n";
}

struct AudioData {
    PitchShifter* pitchShifter;
    SpectralMeter* spectralMeter;
    unsigned int bufferSize;
    float pitchSemitones;
    float outputGain;
    bool enableFFT;
    bool usingPreset;
    const char* presetName;
};

int audioCallback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
                 double /*streamTime*/, RtAudioStreamStatus status, void* userData) {
    
    if (status) {
        std::cerr << "Stream underflow/overflow detected!" << std::endl;
    }
    
    AudioData* data = static_cast<AudioData*>(userData);
    float* input = static_cast<float*>(inputBuffer);
    float* output = static_cast<float*>(outputBuffer);
    
    // Process audio through pitch shifter
    if (input && output && data->pitchShifter) {
        data->pitchShifter->processBlock(input, output, nBufferFrames);
        
        // Apply output gain
        for (unsigned int i = 0; i < nBufferFrames; ++i) {
            output[i] *= data->outputGain;
        }
        
        // Feed output to spectral meter if enabled
        if (data->enableFFT && data->spectralMeter) {
            for (unsigned int i = 0; i < nBufferFrames; ++i) {
                data->spectralMeter->addSample(output[i]);
            }
        }
    } else {
        // Fill with silence if no input/output buffers
        if (output) {
            std::memset(output, 0, nBufferFrames * sizeof(float));
        }
    }
    
    return 0;
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    float semitones = 5.0f;  // Default to +5 semitones
    float mixLevel = 1.0f;   // Default to 100% wet
    float outputGain = 1.0f; // Default to unity gain
    bool enableFFT = false;  // Default to no FFT display
    bool usingPreset = false;
    const char* presetName = nullptr;
    
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-s") == 0 || std::strcmp(argv[i], "--semitones") == 0) {
            if (i + 1 < argc) {
                semitones = std::atof(argv[++i]);
                semitones = std::max(-12.0f, std::min(12.0f, semitones));
            }
        } else if (std::strcmp(argv[i], "-m") == 0 || std::strcmp(argv[i], "--mix") == 0) {
            if (i + 1 < argc) {
                mixLevel = std::atof(argv[++i]);
                mixLevel = std::max(0.0f, std::min(1.0f, mixLevel));
            }
        } else if (std::strcmp(argv[i], "-g") == 0 || std::strcmp(argv[i], "--gain") == 0) {
            if (i + 1 < argc) {
                outputGain = std::atof(argv[++i]);
                outputGain = std::max(0.1f, std::min(2.0f, outputGain));
            }
        } else if (std::strcmp(argv[i], "-p") == 0 || std::strcmp(argv[i], "--preset") == 0) {
            if (i + 1 < argc) {
                presetName = argv[++i];
                usingPreset = true;
                
                if (std::strcmp(presetName, "octave-up") == 0) {
                    semitones = 12.0f;
                } else if (std::strcmp(presetName, "octave-down") == 0) {
                    semitones = -12.0f;
                } else if (std::strcmp(presetName, "fifth-up") == 0) {
                    semitones = 7.0f;
                } else if (std::strcmp(presetName, "chipmunk") == 0) {
                    semitones = 8.0f;
                } else if (std::strcmp(presetName, "deep") == 0) {
                    semitones = -5.0f;
                } else {
                    std::cerr << "Unknown preset: " << presetName << std::endl;
                    std::cerr << "Available presets: octave-up, octave-down, fifth-up, chipmunk, deep" << std::endl;
                    return -1;
                }
            }
        } else if (std::strcmp(argv[i], "-f") == 0 || std::strcmp(argv[i], "--fft") == 0) {
            enableFFT = true;
        } else if (std::strcmp(argv[i], "-v") == 0 || std::strcmp(argv[i], "--version") == 0) {
            std::cout << "Pocket Pitch v" << POCKET_PITCH_VERSION << std::endl;
            std::cout << "Real-time granular pitch shifter with anti-aliasing" << std::endl;
            return 0;
        } else if (std::strcmp(argv[i], "-h") == 0 || std::strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        }
    }
    
    RtAudio audio;
    
    if (audio.getDeviceCount() < 1) {
        std::cerr << "No audio devices found!" << std::endl;
        return -1;
    }
    
    RtAudio::StreamParameters inputParams, outputParams;
    inputParams.deviceId = audio.getDefaultInputDevice();
    inputParams.nChannels = 1;
    inputParams.firstChannel = 0;
    
    outputParams.deviceId = audio.getDefaultOutputDevice();
    outputParams.nChannels = 1;
    outputParams.firstChannel = 0;
    
    unsigned int sampleRate = 44100;
    unsigned int bufferFrames = 256;
    
    // Create pitch shifter
    PitchShifter pitchShifter(bufferFrames, static_cast<float>(sampleRate));
    
    // Set pitch and mix from command line arguments
    float pitchRatio = std::pow(2.0f, semitones / 12.0f);
    pitchShifter.setPitchRatio(pitchRatio);
    pitchShifter.setMixLevel(mixLevel);
    
    // Create spectral meter if FFT is enabled
    SpectralMeter* spectralMeter = nullptr;
    if (enableFFT) {
        spectralMeter = new SpectralMeter(1024);
    }
    
    AudioData data;
    data.pitchShifter = &pitchShifter;
    data.spectralMeter = spectralMeter;
    data.bufferSize = bufferFrames;
    data.pitchSemitones = semitones;
    data.outputGain = outputGain;
    data.enableFFT = enableFFT;
    data.usingPreset = usingPreset;
    data.presetName = presetName;
    
    try {
        audio.openStream(&outputParams, &inputParams, RTAUDIO_FLOAT32,
                        sampleRate, &bufferFrames, &audioCallback, &data);
        audio.startStream();
        
        if (!enableFFT) {
            std::cout << "Pocket Pitch - Granular pitch shifter with anti-aliasing" << std::endl;
            std::cout << "Sample Rate: " << sampleRate << " Hz" << std::endl;
            std::cout << "Buffer Size: " << bufferFrames << " samples" << std::endl;
            
            if (usingPreset) {
                std::cout << "Preset: " << presetName << " (";
                if (semitones >= 0) {
                    std::cout << "+" << semitones;
                } else {
                    std::cout << semitones;
                }
                std::cout << " semitones)" << std::endl;
            } else {
                if (semitones >= 0) {
                    std::cout << "Pitch Shift: +" << semitones << " semitones";
                } else {
                    std::cout << "Pitch Shift: " << semitones << " semitones";
                }
                std::cout << " (ratio: " << pitchRatio << ")" << std::endl;
            }
            std::cout << "Mix Level: " << (mixLevel * 100.0f) << "% wet" << std::endl;
            std::cout << "Output Gain: " << outputGain << "x (" << (20.0f * std::log10(outputGain)) << " dB)" << std::endl;
            std::cout << "Press Enter to quit..." << std::endl;
        }
        
        std::cin.get();
        
        audio.stopStream();
    } catch (RtAudioError& e) {
        e.printMessage();
        return -1;
    }
    
    if (audio.isStreamOpen()) {
        audio.closeStream();
    }
    
    // Clean up spectral meter
    if (spectralMeter) {
        delete spectralMeter;
    }
    
    return 0;
}
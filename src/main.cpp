#include <iostream>
#include <RtAudio.h>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include "RingBuffer.h"
#include "PitchShifter.h"

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options]\n"
              << "Options:\n"
              << "  -s, --semitones <value>   Pitch shift in semitones [-12 to +12] (default: 5)\n"
              << "  -m, --mix <value>         Wet/dry mix [0.0 to 1.0] (default: 1.0)\n"
              << "  -h, --help                Show this help message\n"
              << "\nExamples:\n"
              << "  " << programName << " -s 7 -m 0.5    # +7 semitones, 50% mix\n"
              << "  " << programName << " -s -5          # -5 semitones, 100% wet\n";
}

struct AudioData {
    PitchShifter* pitchShifter;
    unsigned int bufferSize;
    float pitchSemitones;
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
    
    AudioData data;
    data.pitchShifter = &pitchShifter;
    data.bufferSize = bufferFrames;
    data.pitchSemitones = semitones;
    
    try {
        audio.openStream(&outputParams, &inputParams, RTAUDIO_FLOAT32,
                        sampleRate, &bufferFrames, &audioCallback, &data);
        audio.startStream();
        
        std::cout << "Pocket Pitch - Granular pitch shifter with anti-aliasing" << std::endl;
        std::cout << "Sample Rate: " << sampleRate << " Hz" << std::endl;
        std::cout << "Buffer Size: " << bufferFrames << " samples" << std::endl;
        
        if (semitones >= 0) {
            std::cout << "Pitch Shift: +" << semitones << " semitones";
        } else {
            std::cout << "Pitch Shift: " << semitones << " semitones";
        }
        std::cout << " (ratio: " << pitchRatio << ")" << std::endl;
        std::cout << "Mix Level: " << (mixLevel * 100.0f) << "% wet" << std::endl;
        std::cout << "Press Enter to quit..." << std::endl;
        
        std::cin.get();
        
        audio.stopStream();
    } catch (RtAudioError& e) {
        e.printMessage();
        return -1;
    }
    
    if (audio.isStreamOpen()) {
        audio.closeStream();
    }
    
    return 0;
}
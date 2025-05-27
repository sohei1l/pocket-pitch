#include <iostream>
#include <RtAudio.h>
#include <cstring>
#include <cmath>
#include "RingBuffer.h"
#include "PitchShifter.h"

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

int main() {
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
    
    // Default to +5 semitones (perfect fourth up)
    float semitones = 5.0f;
    float pitchRatio = std::pow(2.0f, semitones / 12.0f);
    pitchShifter.setPitchRatio(pitchRatio);
    
    AudioData data;
    data.pitchShifter = &pitchShifter;
    data.bufferSize = bufferFrames;
    data.pitchSemitones = semitones;
    
    try {
        audio.openStream(&outputParams, &inputParams, RTAUDIO_FLOAT32,
                        sampleRate, &bufferFrames, &audioCallback, &data);
        audio.startStream();
        
        std::cout << "Pocket Pitch - Granular pitch shifter active" << std::endl;
        std::cout << "Sample Rate: " << sampleRate << " Hz" << std::endl;
        std::cout << "Buffer Size: " << bufferFrames << " samples" << std::endl;
        std::cout << "Pitch Shift: +" << semitones << " semitones (ratio: " << pitchRatio << ")" << std::endl;
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
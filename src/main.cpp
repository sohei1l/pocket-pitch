#include <iostream>
#include <RtAudio.h>
#include <cstring>

struct AudioData {
    float* inputBuffer;
    float* outputBuffer;
    unsigned int bufferSize;
};

int audioCallback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
                 double /*streamTime*/, RtAudioStreamStatus status, void* userData) {
    
    if (status) {
        std::cerr << "Stream underflow/overflow detected!" << std::endl;
    }
    
    float* input = static_cast<float*>(inputBuffer);
    float* output = static_cast<float*>(outputBuffer);
    
    // Simple pass-through: copy input to output
    if (input && output) {
        std::memcpy(output, input, nBufferFrames * sizeof(float));
    } else {
        // Fill with silence if no input
        std::memset(output, 0, nBufferFrames * sizeof(float));
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
    
    AudioData data;
    data.bufferSize = bufferFrames;
    
    try {
        audio.openStream(&outputParams, &inputParams, RTAUDIO_FLOAT32,
                        sampleRate, &bufferFrames, &audioCallback, &data);
        audio.startStream();
        
        std::cout << "Pocket Pitch - Audio pass-through active" << std::endl;
        std::cout << "Sample Rate: " << sampleRate << " Hz" << std::endl;
        std::cout << "Buffer Size: " << bufferFrames << " samples" << std::endl;
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
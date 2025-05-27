#include <iostream>
#include <RtAudio.h>
#include <cstring>
#include "RingBuffer.h"

struct AudioData {
    RingBuffer* ringBuffer;
    unsigned int bufferSize;
};

int audioCallback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
                 double /*streamTime*/, RtAudioStreamStatus status, void* userData) {
    
    if (status) {
        std::cerr << "Stream underflow/overflow detected!" << std::endl;
    }
    
    AudioData* data = static_cast<AudioData*>(userData);
    float* input = static_cast<float*>(inputBuffer);
    float* output = static_cast<float*>(outputBuffer);
    
    // Write input to ring buffer
    if (input && data->ringBuffer) {
        data->ringBuffer->write(input, nBufferFrames);
    }
    
    // Read from ring buffer to output (zero-latency echo)
    if (output && data->ringBuffer) {
        data->ringBuffer->read(output, nBufferFrames);
    } else {
        // Fill with silence if no output buffer
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
    
    // Create ring buffer (2x buffer size for decoupling)
    RingBuffer ringBuffer(bufferFrames * 2);
    
    AudioData data;
    data.ringBuffer = &ringBuffer;
    data.bufferSize = bufferFrames;
    
    try {
        audio.openStream(&outputParams, &inputParams, RTAUDIO_FLOAT32,
                        sampleRate, &bufferFrames, &audioCallback, &data);
        audio.startStream();
        
        std::cout << "Pocket Pitch - Ring buffer echo active" << std::endl;
        std::cout << "Sample Rate: " << sampleRate << " Hz" << std::endl;
        std::cout << "Buffer Size: " << bufferFrames << " samples" << std::endl;
        std::cout << "Ring Buffer Size: " << ringBuffer.getSize() << " samples" << std::endl;
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
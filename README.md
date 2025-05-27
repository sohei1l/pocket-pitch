# Pocket Pitch

A tiny C++ tool that takes your mic input, shifts the pitch ± 12 semitones with a granular delay algorithm, and plays it back instantaneously. Runs in a terminal.

## Building

Requires RtAudio library:

### macOS
```bash
brew install rtaudio
```

### Build
```bash
mkdir build
cd build
cmake ..
make
```

## Running
```bash
./pocket-pitch                         # Default: +5 semitones, 100% wet, unity gain
./pocket-pitch -s 7 -m 0.5 -g 1.5      # +7 semitones, 50% mix, +3dB gain
./pocket-pitch -s -3 -m 0.8 -g 0.7     # -3 semitones, 80% wet, -3dB gain
./pocket-pitch -f                      # Enable real-time FFT spectral meter
./pocket-pitch -s 5 -f -g 1.2          # +5 semitones with spectral visualization
./pocket-pitch --help                  # Show usage
```

## Features

- Real-time granular pitch shifting (±12 semitones)
- Lock-free audio processing for minimal latency
- Anti-aliasing filter to reduce artifacts
- Wet/dry mix control
- Output gain control (0.1x to 2.0x)
- Live FFT spectral visualization
- Cross-platform terminal interface
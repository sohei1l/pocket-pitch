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
./pocket-pitch -p chipmunk -m 0.8      # Chipmunk preset (+8 semitones) with 80% mix
./pocket-pitch -p octave-down -f       # Octave down preset with FFT visualization
./pocket-pitch -s 7 -m 0.5 -g 1.5      # +7 semitones, 50% mix, +3dB gain
./pocket-pitch --help                  # Show usage
```

## Presets
Quick access to common pitch shift settings:
- `octave-up`: +12 semitones (double pitch)
- `octave-down`: -12 semitones (half pitch) 
- `fifth-up`: +7 semitones (perfect fifth)
- `chipmunk`: +8 semitones (high cartoon voice)
- `deep`: -5 semitones (lower voice)

## Features

- Real-time granular pitch shifting (±12 semitones)
- Built-in presets for common effects (octaves, fifths, voice effects)
- Lock-free audio processing for minimal latency
- Anti-aliasing filter to reduce artifacts
- Wet/dry mix control
- Output gain control (0.1x to 2.0x)
- Live FFT spectral visualization
- Cross-platform terminal interface
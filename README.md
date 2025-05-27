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
./pocket-pitch                    # Default: +5 semitones, 100% wet
./pocket-pitch -s 7 -m 0.5        # +7 semitones, 50% mix
./pocket-pitch -s -3 -m 0.8       # -3 semitones, 80% wet
./pocket-pitch -f                 # Enable real-time FFT spectral meter
./pocket-pitch -s 5 -f            # +5 semitones with spectral visualization
./pocket-pitch --help             # Show usage
```

## Milestones

- [x] **Milestone 1**: I/O Loop - CMake skeleton + single .cpp. Use RtAudio for duplex stream; pass-through audio untouched.
- [x] **Milestone 2**: Ring Buffer - Implement lock-free circular buffer (2× block size) to decouple callback timing; verify zero-latency echo.
- [x] **Milestone 3**: Granular Shift - Add two cross-fading read heads that glide through the buffer at rate = pitchRatio. Linear-interp resampling gives ±1 octave without FFT.
- [x] **Milestone 4**: Anti-Aliasing & Wet/Dry - Insert simple FIR low-pass on each grain, expose mix and semitones CLI flags.
- [x] **Milestone 5**: Spectral Meter (Bonus) - Pipe a 1024-point FFT via KISS FFT every 50 ms and print an ASCII bar graph so you can "see" the shift.
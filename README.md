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
./pocket-pitch
```

## Milestones

- [x] **Milestone 1**: I/O Loop - CMake skeleton + single .cpp. Use RtAudio for duplex stream; pass-through audio untouched.
- [ ] **Milestone 2**: Ring Buffer - Implement lock-free circular buffer (2× block size) to decouple callback timing; verify zero-latency echo.
- [ ] **Milestone 3**: Granular Shift - Add two cross-fading read heads that glide through the buffer at rate = pitchRatio. Linear-interp resampling gives ±1 octave without FFT.
- [ ] **Milestone 4**: Anti-Aliasing & Wet/Dry - Insert simple FIR low-pass on each grain, expose mix and semitones CLI flags.
- [ ] **Milestone 5**: Spectral Meter (Bonus) - Pipe a 1024-point FFT via KISS FFT every 50 ms and print an ASCII bar graph so you can "see" the shift.
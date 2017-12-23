# VOSIMSynth
[![Build status](https://ci.appveyor.com/api/projects/status/49ghy4v5wbkmi0ot?svg=true)](https://ci.appveyor.com/project/austensatterlee/vosimsynth)

VOSIMSynth is a fully modular audio synthesizer with a focus on speed, audio quality, and ease of use.

<p align="center">
<img src="https://raw.github.com/austensatterlee/VOSIMSynth/newgraphics/screenshots/VOSIMProject_1.png"
width=800>
</p>

## Design Goals
- **Speed**. The real-time audio code includes very few dynamic memory allocations (including usage of STL
  containers), and no dynamic memory allocations in the hot path. Communication with the GUI even uses lock-free
  queues to minimize the chance of latency spikes. Benchmarking code is included for many of the real-time
  audio components.
- **Audio Quality**. Band-limited waveforms are generated during compilation and are resampled on-the-fly
  using massively oversampled sinc kernels to preserve fidelity at different pitches. To keep things speedy,
  the waveforms are first resampled to each octave during initialization. When playing a note, the waveform closest in size is used for further resampling to minimize convolution length.
- **Flexibility**. Any output may be connected to any input. Even feedback loops are permissible as long as a
  the unit delay is inserted somewhere in the loop. 
- **Extensibility**. New units can be easily created by deriving from the Unit class. Macros are used to achieve
  a certain degree of reflection to automate things like voice cloning and serialization.

## GUI Features
- **Automatic wire routing**. Wires route themselves to their destination with the objective of minimizing
  their length and avoiding overlap with other wires.
- **Style customization**. The styling of most GUI components may be customized in the settings menu.
- **Resizable**. The GUI generated with vector graphics, and so the window may be freely resized.

VOSIMSynth's UI strives to make the common use case simple (and default) while allowing flexibility to those
who want to delve into the details. 

For example, by default, an envelope's attack phase is triggered by a MIDI noteOn event, and their release
phase by a MIDI noteOff event. However, an arbitrary signal can be hooked up to the envelope's "gate" input in
order to override that behavior. The envelope will automatically detect rising and falling edges of the gate
signal in order to trigger an attack and release phases. This allows envelopes to be triggered and released by
LFO's, MIDI CC values, other envelopes, or any other signal you feel like using.

## Directory Structure
The VOSIMLib directory contains the core code for the real-time audio thread. It creates and maintains the
signal flow graph and contains all of the DSP code for each unit. It has no dependency on the VOSIMSynth
directory, so it may be compiled independently and used in other projects to quickly create processing graphs
that may be edited during run-time.

The VOSIMSynth directory contains the application/VST code, including the GUI.

## Compatibility
Due to a small chunk of platform-specific GUI code, VOSIMSynth is only compatible with Windows, at least for
the moment. The rest of the code has been written with platform independence in mind, and porting to other
environments is planned.

## Dependencies
Most of the dependencies are included in the repository. Be sure to grab all the submodules, either by cloning
the repo recursively, or running:
```
git submodule update --init --recursive
```

The dependencies that you are responsible for (i.e. not included in the repo) are listed below.

#### Boost
VOSIMSynth depends on some components of the Boost library. Prior to running CMake, set the `BOOST_ROOT`
environment variable to the location where Boost lives, or simply run `cmake-gui .` in the build directory and
configure the build options graphically.

#### Python + NumPy + SciPy
VOSIMSynth requires some wavetables to be pre-generated before being built. These are massively oversampled
windowed sinc filters that are used to perform interpolation, filtering, and resampling during runtime.

A python script will be executed automatically during the build phase in order to generate this data. You must
have numpy and scipy installed in order to execute the script.

If you are having trouble installing these dependencies on Windows, check out Christoph Gohlke's [awesome
collection of windows binaries](http://www.lfd.uci.edu/~gohlke/pythonlibs). Download the appropriate versions
of NumPy and SciPy for your system, then install them with `pip install <path_to_whl_file>`.

## Build
VOSIMSynth uses CMake to generate build scripts. It has been tested with Visual Studio 2015 and Visual
Studio 2017.

For example, to generate the x86 project files for Visual Studio 2015 inside a folder named "_build32":
```cmd
mkdir _build32 
cd _build32
cmake .. -G "Visual Studio 14 2015"
```

To configure the build (e.g. to set up the directories your dependencies live in), run 
```cmd
cmake-gui .
``` 

You can also pre-configure the build using environment variables.

After successfully generating the project files, you may open the solution using Visual Studio and work
normally, ignoring CMake ever existed,

Alternatively, build directly from the command line:
```cmd
cmake --build . --config Release
```

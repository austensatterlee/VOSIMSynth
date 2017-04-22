# VOSIMSynth
[![Build status](https://ci.appveyor.com/api/projects/status/49ghy4v5wbkmi0ot?svg=true)](https://ci.appveyor.com/project/austensatterlee/vosimsynth)

In short, VOSIMSynth is a modular audio synthesizer that compiles to a VST instrument.

Basically, you place these so-called "Units" onto a grid, and then connect them together with wires:

<img src="https://raw.github.com/austensatterlee/VOSIMSynth/newgraphics/screenshots/VOSIMProject_1.png"
width=800>

## Design Goals
VOSIMSynth's UI strives to make the common use case simple (and default), while allowing flexibility to those
who want to delve into the details. 

For example, by default an envelope's attack phase is triggered by a MIDI noteOn event, an their release phase
by a MIDI noteOff event. Business as usual. However, an arbitrary signal can be hooked up to the envelope's
"trigger" input in order to override that behavior. This allows envelopes to be triggered and released by
LFO's, MIDI CC values, other envelopes, or any other signal you feel like using.

## Components
The VOSIMLib directory contains the core code for creating the processing
graph and executing DSP code. It may be compiled independently and used in
other projects to quickly create processing graphs that may be edited
during run-time.

The VOSIMSynth directory contains the application/VST code, including the GUI
library.

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
```
mkdir _build32 
cd _build32
cmake .. -G "Visual Studio 14 2015"
```

To configure the build (e.g. to set up the directories your dependencies live in), run 
```
cmake-gui .
``` 

You can also pre-configure the build using environment variables.

After successfully generating the project files, you may open the solution using Visual Studio and work normally, ignoring CMake ever existed,

**OR**

You may build from the command line:
```
cmake --build . --config Release
```

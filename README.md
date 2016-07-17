# VOSIMProject
Modular VST synthesizer environment.

To build the project files for Visual Studio 2015 inside a folder named "_vsbuild":
```
mkdir _vsbuild
cd _vsbuild
cmake .. -G "Visual Studio 14 2015"
```

![screenshot](https://raw.github.com/austensatterlee/VOSIMSynth/newgraphics/screenshots/VOSIMProject_1.png)

The VOSIMLib directory contains the core code for creating the processing
graph and executing DSP code. It may be compiled independently and used in
other projects to quickly create processing graphs that may be edited
during run-time.

The VOSIMSynth directory contains the application code, including the GUI
library.

Clone the libraries in this directory according to the readme in each
sub-folder.

Additional required libraries (not in this directory):
- **Boost Headers**
  - *http://www.boost.org/users/download/*
  - If CMake can't locate it, add the install directory to an environment variable called BOOST_ROOT.
  - No compiled Boost libraries have yet been used.
- **FFTS**
  - Original repo: *https://github.com/anthonix/ffts*
  - Windows compatible: *https://github.com/linkotec/ffts*
  - Set the cmake variable `FFTS_ROOT` to the library directory. Relative
    to that directory, place the static libraries inside `lib/`, and then
    copy the `ffts.h` header file to `include/ffts/ffts.h`.

**Note for MSVC**

When building statically linked libraries with MSVC, use the `/MT` compiler flag for release builds and `/MTd` for debug builds to ensure it is compiled with the same runtime as VOSIMSynth.

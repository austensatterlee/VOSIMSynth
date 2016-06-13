Clone the libraries in this directory according to the readme in each
sub-folder.

Additional required libraries (not in this directory):
- *SFML*
  - https://github.com/SFML/SFML.git
  - Build and install with CMake
- *GLEW*
  - https://github.com/Perlmint/glew-cmake.git
  - Build and install with CMake
- *Boost.Lockfree*
  - http://www.boost.org/users/download/
  - If CMake can't locate it, add the install directory to an environment variable called BOOST_ROOT.

When building statically linked libraries with MSVC, use the "/MT" compiler flag to ensure it has the same runtime as VOSIMSynth.

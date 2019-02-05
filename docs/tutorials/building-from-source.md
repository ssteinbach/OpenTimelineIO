# Overview

This page gives information on how to compile OpenTimelineIO from source.

OpenTimelineIO is a C++ core with a set of utilies that are written in Python using a Python wrapper around the C++ core.  Most of the adapters are also written in Python, and require a valid python interpreter, so even if you intend to exclusively use the C++ core of OpenTimelineIO we strongly recomend having a compatible Python interpreter installed so that you can run the full set of adapers.

## Before you do anything else

You'll need the source code and to have intialized the submodules:

- Clone the source: `git clone git@github.com:PixarAnimationStudios/OpenTimelineIO.git OpenTimelineIO`
- Initialize the submodules: `cd OpenTimelineIO; git submodule init; git submodule update`

## Build Dependencies:

- Git (to get the source code)
- CMake, either installed on the system or via the `cmake` `pip` package (`pip install cmake`)

# If You Just Care About Python

If all you care about is the python bindings, you can run:

- `pip install .` from the root directory of the project, and the `setup.py` should correctly compile the C++ core and the python bindings, link them and install them into your python environment.

## If You Just Care About C++

If all you care about is the C++ core, you can use the CMake build system directly:

- Make a build directory: `mkdir build`
- Configure: `cd build; ccmake ..`
- Then you should be able to compile using whichever build system you chose to have Cmake build for you.

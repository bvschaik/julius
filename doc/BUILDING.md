# Building Julius

To build Julius, you need:

- a compiler
- `cmake`
- `SDL2`
- `SDL2_mixer`
- `libpng` (optional, a bundled copy will be used if not found)

After downloading or cloning the sources, run the following commands:

	$ mkdir build && cd build
	$ cmake ..
	$ make

This results in a `julius` executable for your platform.

See [Running Julius](RUNNING.md) for instructions on how to configure Julius.

See [Building Julius (Wiki)](https://github.com/bvschaik/julius/wiki/Building-Julius) for detailed build instructions and additional CMake flags.

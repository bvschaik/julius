# Building Julius

If you have experience in compiling from source, these are the basic instructions.

To build Julius, you'll need:

- `git`
- a compiler (`gcc`, `clang`, Visual Studio and MinGW(-w64) are all known to be supported)
- `cmake`
- `SDL2`
- `SDL2_mixer`
- `libpng` (optional, a bundled copy will be used if not found)

After cloning the repo (URL: `https://github.com/bvschaik/julius.git`), run the following commands:

	$ mkdir build && cd build
	$ cmake ..
	$ make

This results in a `julius` executable.

To use the bundled copies of optional libraries even if a system version is available, add `-DSYSTEM_LIBS=OFF` to `cmake` invocation.

To build the Vita or Switch versions, use `cmake .. -DVITA_BUILD=ON` or `cmake .. -DSWITCH_BUILD=ON`
instead of `cmake ..`.

You'll obviously need the Vita or Switch SDK's. Docker images for the SDK's are available:

- Vita: `gnuton/vitasdk-docker:20190626`
- Switch: `rsn8887/switchdev`

See [Running Julius](RUNNING.md) for instructions on how to configure Julius for your platform.

--------------------------------------------------

For detailed building instructions, please check out the respective page:

- [Building for Windows](building_windows.md)
- [Building for Linux](building_linux.md)
- [Building for Mac](building_macos.md)
- [Building for Playstation Vita](building_vita.md)
- [Building for Nintendo Switch](building_switch.md)
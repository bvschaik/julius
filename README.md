# Julius ![](res/julius_48.png)

Julius is an open source re-implementation of Caesar III.

The aim of this project is to create an open-source version of Caesar 3, with the same logic as the original, but with some UI enhancements, that is able to be played on multiple platforms. The same logic means that the saved games are 100% compatible, and any gameplay bugs present in the original Caesar 3 game will also be present in Julius.

UI enhancements include:
- Support for widescreen resolutions
- Windowed mode support for 32-bit desktops

Julius requires the original assets (graphics, sounds, etc) from Caesar 3 to run.

## Building from source

Requirements:
- cmake
- SDL2
- SDL2_mixer

After cloning the repo, run the following commands:

	$ mkdir build && cd build
	$ cmake ..
	$ make

This results in a `julius` executable.

## Running the game

The game data should be in the same directory as in which the binary is run
or it can be run as `julius /path/to/game-data`. You must have permission
to write in that directory as the saves will be stored there.

See [Running](RUNNING.md) for instructions.

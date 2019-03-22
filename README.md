# Julius ![](res/julius_48.png)

Chat: [![Gitter](https://badges.gitter.im/julius-game/community.svg)](https://gitter.im/julius-game/community)

|             | Status | Download |
|-------------|--------|----------|
| Windows  | [![AppVeyor](https://ci.appveyor.com/api/projects/status/github/bvschaik/julius?branch=master&svg=true)](https://ci.appveyor.com/project/bvschaik/julius) | [![Latest unstable](https://img.shields.io/badge/latest_unstable-gray.svg?logo=appveyor&logoColor=silver)](https://ci.appveyor.com/api/projects/bvschaik/julius/artifacts/julius.zip?branch=master&job=Environment%3A%20APPVEYOR_BUILD_WORKER_IMAGE%3DVisual%20Studio%202015%2C%20COMPILER%3Dmingw%2C%20CMAKE_BUILD_TYPE%3DRelease%2C%20PLATFORMTOOLSET%3Dv140) |
| Linux | [![Travis](https://travis-matrix-badges.herokuapp.com/repos/bvschaik/julius/branches/master/1)](https://travis-ci.org/bvschaik/julius) |
| Mac | [![Travis](https://travis-matrix-badges.herokuapp.com/repos/bvschaik/julius/branches/master/4)](https://travis-ci.org/bvschaik/julius) |
| PS Vita | [![Travis](https://travis-matrix-badges.herokuapp.com/repos/bvschaik/julius/branches/master/5)](https://travis-ci.org/bvschaik/julius) | [![Latest unstable](https://img.shields.io/badge/latest_unstable-gray.svg)](https://bintray.com/bvschaik/julius/vita-unstable/_latestVersion) |
| Nintendo Switch | [![Travis](https://travis-matrix-badges.herokuapp.com/repos/bvschaik/julius/branches/master/6)](https://travis-ci.org/bvschaik/julius) |

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

After downloading the Windows build or compiling the game yourself, the game data should be in the same directory as in which the binary is run or it can be run as `julius /path/to/game-data`.

The easiest way to run the game is to copy either the compiled executable (in case you built it from source) or the contents of the downloaded zip file to your Caesar 3 installation folder. Then run the `julius` executable as any other program.

Note that you must have permission to write in the game data directory as the saves will be stored there.

See [Running](RUNNING.md) for further instructions.

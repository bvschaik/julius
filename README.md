# Julius ![](res/julius_48.png)

[![AppVeyor](https://ci.appveyor.com/api/projects/status/github/bvschaik/julius?branch=master&svg=true)](https://ci.appveyor.com/project/bvschaik/julius) [![Travis](https://api.travis-ci.com/bvschaik/julius.svg?branch=master)](https://travis-ci.com/github/bvschaik/julius)

Chat: [Discord](http://www.discord.gg/GamerZakh) (kindly hosted by GamerZakh) or [Gitter](https://gitter.im/julius-game/community)

| Platform | Latest release | Latest build (may be unstable) |
|----------|----------------|-----------------|
| Windows  | [![Download](https://api.bintray.com/packages/bvschaik/julius/windows/images/download.svg)](https://bintray.com/bvschaik/julius/windows/_latestVersion) | [![Download](https://api.bintray.com/packages/bvschaik/julius-dev/windows/images/download.svg)](https://bintray.com/bvschaik/julius-dev/windows/_latestVersion#files) |
| Linux AppImage | [![Download](https://api.bintray.com/packages/bvschaik/julius/linux/images/download.svg)](https://bintray.com/bvschaik/julius/linux/_latestVersion#files) | [![Download](https://api.bintray.com/packages/bvschaik/julius-dev/linux/images/download.svg)](https://bintray.com/bvschaik/julius-dev/linux/_latestVersion#files) |
| Mac | [![Download](https://api.bintray.com/packages/bvschaik/julius/mac/images/download.svg)](https://bintray.com/bvschaik/julius/mac/_latestVersion) | [![Download](https://api.bintray.com/packages/bvschaik/julius-dev/mac/images/download.svg)](https://bintray.com/bvschaik/julius-dev/mac/_latestVersion#files) |
| PS Vita | [![Download](https://api.bintray.com/packages/bvschaik/julius/vita/images/download.svg)](https://bintray.com/bvschaik/julius/vita/_latestVersion) | [![Download](https://api.bintray.com/packages/bvschaik/julius-dev/vita/images/download.svg)](https://bintray.com/bvschaik/julius-dev/vita/_latestVersion#files) |
| Nintendo Switch | [![Download](https://api.bintray.com/packages/bvschaik/julius/switch/images/download.svg)](https://bintray.com/bvschaik/julius/switch/_latestVersion) | [![Download](https://api.bintray.com/packages/bvschaik/julius-dev/switch/images/download.svg)](https://bintray.com/bvschaik/julius-dev/switch/_latestVersion#files) |
| Android | [![Download](https://api.bintray.com/packages/bvschaik/julius/android/images/download.svg)](https://bintray.com/bvschaik/julius/android/_latestVersion) | [![Download](https://api.bintray.com/packages/bvschaik/julius-dev/android/images/download.svg)](https://bintray.com/bvschaik/julius-dev/android/_latestVersion#files) |

## Julius: an open source re-implementation of Caesar III

Julius is a fully working open-source version of Caesar 3, with the same logic
as the original, but with some UI enhancements, that can be played on multiple platforms.

Julius will not run without the original Caesar 3 files. You can buy a digital copy from [GOG](https://www.gog.com/game/caesar_3) or
[Steam](https://store.steampowered.com/app/517790/Caesar_3/), or you can use an original CD-ROM version.

The goal of the project is to have exactly the same game logic as Caesar 3, with the same look
and feel. This means that the saved games are 100% compatible with Caesar 3, and any gameplay bugs
present in the original Caesar 3 game will also be present in Julius.

Enhancements for Julius include:
- Support for widescreen resolutions
- Windowed mode support for 32-bit desktops
- A lot of small in-game [quality of life improvements](https://github.com/bvschaik/julius/wiki/Improvements-from-Caesar-3)
- Support for the [high-quality MP3 files](https://github.com/bvschaik/julius/wiki/MP3-Support) once provided on the Sierra website

While Julius does not implement any gameplay changes, a fork of Julius named [Augustus](https://github.com/Keriew/augustus) is implementing many long-wanted gameplay changes, such as roadblocks.
Beware: Augustus does not support the original Caesar 3 save files, while Julius does. So use Julius for a more vanilla experience with some visual improvements, or Augustus for a different gameplay experience.

![](res/sce_sys/livearea/contents/bg.png)

## Running the game

First, download the game for your platform from the list above. Alternatively, you can [build Julius](doc/BUILDING.md) yourself.

Then you can either copy the game to the Caesar 3 folder, or run the game from an independent
folder, in which case the game will ask you to point to the Caesar 3 folder.

Note that you must have permission to write in the game data directory as the saves will be
stored there.

See [Running](doc/RUNNING.md) for further instructions and startup options.

## Bugs

Julius recreates many bugs that you can find in original Caesar 3 game, to preserve 100% saves compatibility. See the list of [Bugs & idiosyncrasies](https://github.com/bvschaik/julius/wiki/Caesar-3-bugs) to find out more.

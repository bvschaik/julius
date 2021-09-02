# Julius ![](res/julius_48.png)

[![Github Actions](https://github.com/bvschaik/julius/workflows/Build%20Julius/badge.svg)](https://github.com/bvschaik/julius/actions)

Download:
[![Download release version](https://julius.biancavanschaik.nl/badge/release.svg)](https://github.com/bvschaik/julius/wiki/Julius-release)
[![Download development version](https://julius.biancavanschaik.nl/badge/development.svg)](https://julius.biancavanschaik.nl/)

Android version on Google Play: [install release](https://play.google.com/store/apps/details?id=com.github.bvschaik.julius) or [sign up for test builds](https://play.google.com/apps/testing/com.github.bvschaik.julius)

Chat: [Discord](http://www.discord.gg/GamerZakh) (kindly hosted by GamerZakh)

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

Supported platforms:
- Windows
- Linux AppImage
- MacOS
- PS Vita
- Nintendo Switch
- Android
- Any compatible browser (Emscripten)

While Julius does not implement any gameplay changes, a fork of Julius named [Augustus](https://github.com/Keriew/augustus) is implementing many long-wanted gameplay changes, such as roadblocks.
Beware: Augustus does not support the original Caesar 3 save files, while Julius does. So use Julius for a more vanilla experience with some visual improvements, or Augustus for a different gameplay experience.

![](res/vita/bg.png)

## Running the game

First, download the game for your platform from the list above. Alternatively, you can [build Julius](doc/BUILDING.md) yourself.

Then you can either copy the game to the Caesar 3 folder, or run the game from an independent
folder, in which case the game will ask you to point to the Caesar 3 folder.

Note that you must have permission to write in the game data directory as the saves will be
stored there. Also, your game must be patched to 1.0.1.0 to use Julius. If Julius tells you that
you are missing it, you can [download the update here](https://github.com/bvschaik/julius/wiki/Patches).

See [Running Julius](https://github.com/bvschaik/julius/wiki/Running-Julius) for further instructions and startup options.

## Bugs

Julius recreates many bugs that you can find in original Caesar 3 game, to preserve 100% saves compatibility. See the list of [Bugs & idiosyncrasies](https://github.com/bvschaik/julius/wiki/Caesar-3-bugs) to find out more.

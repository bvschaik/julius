# Augustus ![](res/julius_48.png)

[![Github Actions](https://github.com/Keriew/augustus/workflows/Build%20Augustus/badge.svg)](https://github.com/Keriew/Augustus/actions)

Chat: [Discord](http://www.discord.gg/GamerZakh) (kindly hosted by GamerZakh)

  
| Platform | Latest release | Unstable build |
|----------|----------------|----------------|
| Windows  | [![Download](https://api.bintray.com/packages/keriew/Augustus/windows_release/images/download.svg)](https://bintray.com/keriew/Augustus/windows_release/_latestVersion#files) | [![Download](https://api.bintray.com/packages/keriew/Augustus-unstable/windows-unstable/images/download.svg)](https://bintray.com/keriew/Augustus-unstable/windows-unstable/_latestVersion#files)  (needs assets files from the release version) | 
| Linux AppImage | [![Download](https://api.bintray.com/packages/keriew/Augustus/linux/images/download.svg)](https://bintray.com/keriew/Augustus/linux/_latestVersion#files) | [![Download](https://api.bintray.com/packages/keriew/Augustus-unstable/linux/images/download.svg)](https://bintray.com/keriew/Augustus-unstable/linux/_latestVersion#files)
| Mac | [![Download](https://api.bintray.com/packages/keriew/Augustus/mac/images/download.svg)](https://bintray.com/keriew/Augustus/mac/_latestVersion#files) | [![Download](https://api.bintray.com/packages/keriew/Augustus-unstable/mac/images/download.svg)](https://bintray.com/keriew/Augustus-unstable/mac/_latestVersion#files) |
| PS Vita | [![Download](https://api.bintray.com/packages/keriew/Augustus/vita-unstable/images/download.svg)](https://bintray.com/keriew/Augustus/vita-unstable/_latestVersion#files)| [![Download](https://api.bintray.com/packages/keriew/Augustus-unstable/vita/images/download.svg)](https://bintray.com/keriew/Augustus-unstable/vita/_latestVersion#files) |
| Switch | Next Release! | [![Download](https://api.bintray.com/packages/keriew/Augustus-unstable/switch/images/download.svg)](https://bintray.com/keriew/Augustus-unstable/switch/_latestVersion#files) |
| Android APK | Next Release! | Soon... |


Augustus is a fork of the Julius project that intends to incorporate gameplay changes.
=======
The aim of this project is to provide enhanced, customizable gameplay to Caesar 3 using project Julius UI enhancements.

Augustus is able to load Caesar 3 and Julius saves, however saves made with Augustus **will not work** outside Augustus.

Gameplay enhancements include:
- Roadblocks
- Market special orders
- Global labour pool
- Partial warehouse storage
- Increased game limits
- Zoom controls
- And more!

Because of gameplay changes and additions, save files from Augustus are NOT compatible with Caesar 3 or Julius. Augustus is able to load Caesar 3 save files, but not the other way around. If you want vanilla experience with visual and UI improvements, or want to use save files in base Caesar 3, check [Julius](https://github.com/bvschaik/julius).

Augustus, like Julius, requires the original assets (graphics, sounds, etc) from Caesar 3 to run. Caesar 3 must also be upgraded to v1.1. You can [download the patch files for your language here](https://github.com/bvschaik/julius/wiki/Patches).

Augustus optionally [supports the high-quality MP3 files](https://github.com/bvschaik/julius/wiki/MP3-Support) once provided on the Sierra website.

[![](doc/main-image.png)](https://ppb.chymera.eu/fefa2d.png)

## Running the game

First, download the game for your platform from the list above.

Alternatively, you can build Augustus yourself. Check [Building Julius](doc/BUILDING.md)
for details.

Then you can either copy the game to the Caesar 3 folder, or run the game from an independent
folder, in which case the game will ask you to point to the Caesar 3 folder.

Note that you must have permission to write in the game data directory as the saves will be
stored there.

See [Running Julius](https://github.com/bvschaik/julius/wiki/Running-Julius) for further instructions and startup options.

## Bugs

See the list of [Bugs & idiosyncrasies](doc/bugs.md) to find out more about some known bugs.

# Augustus ![](res/julius_48.png)

[![Github Actions](https://github.com/Keriew/augustus/workflows/Build%20Augustus/badge.svg)](https://github.com/Keriew/Augustus/actions)

Chat: [Discord](http://www.discord.gg/GamerZakh) (kindly hosted by GamerZakh)

  
| Platform | Latest release | Unstable build |
|----------|----------------|----------------|
| Windows  | [![Download](https://augustus.josecadete.net/badge/release/windows.svg)](https://augustus.josecadete.net/download/latest/release/windows) | [![Download](https://augustus.josecadete.net/badge/development/windows.svg)](https://augustus.josecadete.net/download/latest/development/windows)  (needs assets files from the release version) | 
| Linux AppImage | [![Download](https://augustus.josecadete.net/badge/release/linux-appimage.svg)](https://augustus.josecadete.net/download/latest/release/linux-appimage) | [![Download](https://augustus.josecadete.net/badge/development/linux-appimage.svg)](https://augustus.josecadete.net/download/latest/development/linux-appimage)
| Mac | [![Download](https://augustus.josecadete.net/badge/release/mac.svg)](https://augustus.josecadete.net/download/latest/release/mac) | [![Download](https://augustus.josecadete.net/badge/development/mac.svg)](https://augustus.josecadete.net/download/latest/development/mac) |
| PS Vita | [![Download](https://augustus.josecadete.net/badge/release/vita.svg)](https://augustus.josecadete.net/download/latest/release/vita)| [![Download](https://augustus.josecadete.net/badge/development/vita.svg)](https://augustus.josecadete.net/download/latest/development/vita) |
| Switch | Next Release! | [![Download](https://augustus.josecadete.net/badge/development/switch.svg)](https://augustus.josecadete.net/download/latest/development/switch) |
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

Augustus, like Julius, requires the original assets (graphics, sounds, etc) from Caesar 3 to run. Augustus optionally [supports the high-quality MP3 files](https://github.com/bvschaik/julius/wiki/MP3-Support) once provided on the Sierra website.

[![](doc/main-image.png)](https://ppb.chymera.eu/fefa2d.png)

## Running the game

First, download the game for your platform from the list above.

Alternatively, you can build Augustus yourself. Check [Building Julius](doc/BUILDING.md)
for details.

Then you can either copy the game to the Caesar 3 folder, or run the game from an independent
folder, in which case the game will ask you to point to the Caesar 3 folder.

Note that you must have permission to write in the game data directory as the saves will be
stored there. Also, your game must be patched to 1.0.1.0 to use Augustus. If Augustus tells you that
you are missing it, you can [download the update here](https://github.com/bvschaik/julius/wiki/Patches).

See [Running Julius](https://github.com/bvschaik/julius/wiki/Running-Julius) for further instructions and startup options.

## Bugs

See the list of [Bugs & idiosyncrasies](doc/bugs.md) to find out more about some known bugs.

# Running Julius

Julius requires the original Caesar 3 files to run. This can be an old CD-ROM version, or a digital copy
from either [GOG](https://www.gog.com/game/caesar_3) or
[Steam](https://store.steampowered.com/app/517790/Caesar_3/).

Note that there are [command line options](#command-line-options) which are available to all platforms.

If you have the high-quality MP3 music files that were once available for download from the Sierra Website,
the game supports playing them.

Please check [MP3 support](https://github.com/bvschaik/julius/wiki/MP3-Support) for details.

## Windows

Julius supports Windows XP and higher.

1. Install Caesar 3 using the provided installer (GOG/Steam/CD-ROM).
2. Download the [latest release](https://github.com/bvschaik/julius/releases) of Julius or compile from source.
3. Copy julius.exe, SDL2.dll and SDL2_mixer.dll to the folder where you installed Caesar 3
4. Run julius.exe

## Linux/BSD

OpenBSD and FreeBSD provide Julius as [package](http://ports.su/games/julius).

On Linux you can use the provided [AppImage](https://appimage.org/):

1. Obtain the game data files of Caesar 3 by checking one of the two next sections.
2. Download the [latest AppImage release](https://github.com/bvschaik/julius/releases) of Julius.
3. Make the downloaded AppImage executable by going into the file properties or running
   `chmod +x julius-*.AppImage` in the same folder as the AppImage.
4. You can then run it just like any Linux executable.
5. (Optional) You can install [AppImageLauncher](https://github.com/TheAssassin/AppImageLauncher#readme)
   in order to integrate the AppImage in your OS. You'll then be able to launch it easily from the menu
   just like other apps.

### GOG version with InnoExtract
If you bought the GOG edition, you can download the offline installer exe, and use
[InnoExtract](http://constexpr.org/innoextract/) to extract the game files:

1. Build Julius or install using your package manager
2. [Install](http://constexpr.org/innoextract/install) `innoextract` for your distribution
3. Download the Caesar 3 offline installer exe from GOG
4. Run the following command to extract the game files to a new `app` directory:

        $ innoextract -m setup_caesar3_2.0.0.9.exe

5. Move the `julius` executable to the extracted `app` directory and run from there, OR run Julius
   with the path to the game files as parameter:

        $ julius path-to-app-directory

Note that your user requires write access to the directory containing the game files, since the
saved games are also stored there.

### Using WINE

Another option is to get the game files by installing Caesar 3 using [WINE](https://www.winehq.org/):

1. Build Julius or install using your package manager
2. Install Caesar 3 using WINE, take note where the game is installed
3. Run Julius with the path where the game is installed:

        $ julius path-to-c3-directory

## Vita

1. Ensure you have a jailbroken Vita. Detailed jailbreaking instructions can be found on
   [vita.hacks.guide](https://vita.hacks.guide).
2. Install the `julius.vpk` file using Vitashell, like any other homebrew.
3. Copy all the files from a Caesar 3 install into a folder `ux0:/data/julius/`, so that you
   have the file `ux0:/data/julius/c3.eng` and more in your folder.

### Controls

| Input                                          | Effect                                                       |
| ---------------------------------------------- | ------------------------------------------------------------ |
| Left Analog Stick                              | Move the mouse pointer                                       |
| Right Analog Stick or Dpad Up/Down/Left/Right  | Scroll the map                                               |
| R / Cross                                      | Left mouse button                                            |
| L / Circle                                     | Right mouse button                                           |
| Triangle                                       | Simulate Page Up keypress (speed up in-game time)            |
| Square                                         | Simulate Page Down keypress (slow down in-game time)         |
| Start                                          | Bring up on-screen keyboard, useful to enter player name etc.|
| Select                                         | Toggle between touch modes                                   |

Touch modes can be toggled with the select button. There are three modes:
1. Touchpad mode (default)
    * Single finger drag = move the mouse pointer (indirectly like on a touchpad)
    * Single short tap = left mouse click
    * Single short tap while holding a second finger down = right mouse click
    * Dual finger drag = drag'n'drop (left mouse button is held down)
    * Three finger drag = drag'n'drop (right mouse button is held down)
2. Direct mode
    * Pointer jumps to finger, nothing else
3. Julius mode
    * Pointer jumps to finger
    * Single short tap = left mouse click
    * Single short tap while holding a second finger down = right mouse click
    * Dual finger drag = scroll
    * Dragging is context sensitive

For multi-touch gestures, the fingers have to be far enough apart from each other, so that the
Vita will not erroneously recognize them as a single finger. Otherwise the pointer will jump around.

Physical Bluetooth mice and keyboards are supported. This was tested with the "Jelly Comb Mini Bluetooth
Keyboard With Mouse Touchpad," and with the "Jelly Comb Bluetooth Wireless Mouse." The Vita doesn't pair
with all Bluetooth devices.

## Switch

1. Ensure you have a jailbroken Switch. Detailed jailbreaking instructions can be found on
   [Nintendo Homebrew's Guide](https://nh-server.github.io/switch-guide/) or, alternatively, on
   [AtlasNX's Guide](https://guide.teamatlasnx.com/).
2. Extract the contents of `julius_switch.zip` into the `switch` folder on your SD card,
   so that you have a folder `/switch/julius` with `julius.nro` inside.
3. Copy all the files from a Caesar 3 install into the `/switch/julius/` folder, so that you
   have the file `/switch/julius/c3.eng` and more.

### Controls

| Input                                          | Effect                                                       |
| ---------------------------------------------- | ------------------------------------------------------------ |
| Left Analog Stick                              | Move the mouse pointer                                       |
| Right Analog Stick or Dpad Up/Down/Left/Right  | Scroll the map                                               |
| R / A                                          | Left mouse button                                            |
| L / B                                          | Right mouse button                                           |
| ZR                                             | Hold to slow down analog stick mouse pointer                 |
| ZL                                             | Hold to speed up analog stick mouse pointer                  |
| X                                              | Simulate Page Up keypress (speed up in-game time)            |
| Y                                              | Simulate Page Down keypress (slow down in-game time)         |
| Plus                                           | Bring up on-screen keyboard, useful to enter player name etc.|
| Minus                                          | Toggle between touch modes                                   |

Touch modes can be toggled with the minus button. There are three modes:
1. Touchpad mode (default)
    * Single finger drag = move the mouse pointer (indirectly like on a touchpad)
    * Single short tap = left mouse click
    * Single short tap while holding a second finger down = right mouse click
    * Dual finger drag = drag'n'drop (left mouse button is held down)
    * Three finger drag = drag'n'drop (right mouse button is held down)
2. Direct mode
    * Pointer jumps to finger, nothing else
3. Julius mode
    * Pointer jumps to finger
    * Single short tap = left mouse click
    * Single short tap while holding a second finger down = right mouse click
    * Dual finger drag = scroll
    * Dragging is context sensitive

For multi-touch gestures, the fingers have to be far enough apart from each other, so that the
Switch will not erroneously recognize them as a single finger. Otherwise the pointer will jump around.

Physical USB mice and keyboards are supported. All keyboards seem to work. Not all mice work.
A mouse compatibility list is available
[here](https://docs.google.com/spreadsheets/d/1Drbo5-QuSX901MwtOytSMuqRGxeIkq2HELM806I9dj0/edit#gid=0)

## Command line options

Julius supports some command-line options. Its usage is:

    $ julius [ARGUMENTS] [DATA_DIR]

`[ARGUMENTS]` can be the following:

* `--display-scale NUMBER`

    Optional. Scales the entire Julius application by a factor of `NUMBER`. Useful for high-dpi systems.

    `NUMBER`can be any number between `0.5` and `5`. The default is `1`.

* `--cursor-scale NUMBER`

    Optional. Scales the mouse cursor by a factor of `NUMBER`. Cursor scaling is independent of display scaling.

    `NUMBER` can only be set to `1`, `1.5` or `2`. The default is `1`.

`[DATA_DIR]` Is the location of the Caesar 3 asset files.

If `[DATA_DIR]` is not provided, Julius will try to load the asset files from the directory where it is installed.

If the files are not found, it will check if a previous valid directory was stored in the internal preferences
and load the asset files from that directory.

If Julius still fails to load the assets, it will ask you to point to a valid directory.

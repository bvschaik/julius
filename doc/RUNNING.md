# Running Julius

Julius requires the original Caesar 3 files to run. This can be an old CD-ROM version, or a digital copy
from either [GOG](https://www.gog.com/game/caesar_3) or
[Steam](https://store.steampowered.com/app/517790/Caesar_3/).

There are [command line options](https://github.com/bvschaik/julius/wiki/Command-line-options) which are available to all platforms.

If you have the high-quality MP3 music files that were once available for download from the Sierra Website,
the game supports playing them.

Please check [MP3 support](https://github.com/bvschaik/julius/wiki/MP3-Support) for details.

## Windows

Julius supports Windows Vista and higher. Windows XP has limited support.

1. Install Caesar 3 using the provided installer (GOG/Steam/CD-ROM).
2. Download the [latest release](https://github.com/bvschaik/julius/releases/latest) of Julius or compile from source.
3. Copy julius.exe, SDL2.dll, SDL2_mixer.dll and libmpg123.dll to the folder where you installed Caesar 3
4. Run Julius

**Note:** If you install Caesar 3 using Steam and plan to use Steam to launch the game,
***do not*** rename `julius.exe` to `c3.exe`.
Doing so will make the mouse cursor disappear when using right-click to scroll.
   
Instead, open `SierraLauncher.ini` and replace `Game1Exe=c3.exe` with the `Game1Exe=julius.exe`.

Alternatively, you can check the `Disable right click to drag the map` option do disable right-click scrolling.

### Windows XP

Julius still works on Windows XP, but the most recent version of SDL does not. To get it to work:

1. Follow the general instructions for Windows
2. Download [SDL 2.0.9](http://libsdl.org/release/SDL2-2.0.9-win32-x86.zip)
3. Extract `SDL2.dll` from that zip file to the same folder where you installed Julius, overwriting the existing file
4. Run Julius

## Linux/BSD

OpenBSD and FreeBSD provide Julius as [package](http://ports.su/games/julius).

On Linux you can use the provided [AppImage](https://appimage.org/):

1. Obtain the game data files of Caesar 3 by checking one of the two next sections.
2. Download the [latest AppImage release](https://github.com/bvschaik/julius/releases/latest) of Julius.
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

## MacOS

Follow these instructions for either the GOG or CD-ROM version of the game.

### GOG: using InnoExtract

1. Install InnoExtract through [Homebrew](https://brew.sh/):
   ```
   $ brew install innoextract
   ```
2. Download the Caesar 3 offline installer exe from GOG
3. Extract the setup file to a new `app` folder:
   ```
   $ innoextract -m setup_caesar3_2.0.0.9.exe
   ```
4. Start Julius, and point the game to the `app` folder that was just extracted

### CD-ROM: using UnShield

1. Install Unshield through [Homebrew](https://brew.sh/):
   ```
   $ brew install unshield
   ```
2. Open a Terminal in the directory where you want the Caesar 3 files
3. Insert your CD-ROM and run the following command to extract the installer (replace `{CD}` with the name of your C3 cd-rom):
   ```
   $ unshield -g Exe x /Volumes/{CD}/data1.cab
   ```
4. Copy sound and video files over to the Exe directory:
   ```
   $ cp -r /Volumes/{CD}/555 Exe
   $ cp -r /Volumes/{CD}/SMK Exe
   $ cp -r /Volumes/{CD}/wavs Exe
   $ cp -r /Volumes/{CD}/Soundfx/* Exe/wavs
   ```
5. Start Julius, and point the game to the `Exe` folder that was just extracted

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
2. Direct mode
    * Pointer jumps to finger, nothing else
3. [Julius mode](https://github.com/bvschaik/julius/wiki/Touch-Support)

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
2. Direct mode
    * Pointer jumps to finger, nothing else
3. [Julius mode](https://github.com/bvschaik/julius/wiki/Touch-Support)

For multi-touch gestures, the fingers have to be far enough apart from each other, so that the
Switch will not erroneously recognize them as a single finger. Otherwise the pointer will jump around.

Physical USB mice and keyboards are supported. All keyboards seem to work. Not all mice work.
A mouse compatibility list is available
[here](https://docs.google.com/spreadsheets/d/1Drbo5-QuSX901MwtOytSMuqRGxeIkq2HELM806I9dj0/edit#gid=0)

## Android

Like on other platforms, you must first make sure you obtain the original Caesar III files to your
Android device. You may do so by installing Caesar III on your computer then copying the files to
Android, or, if you have purchased Caesar III from GOG, you can use [Inno Setup Extractor](https://play.google.com/store/apps/details?id=uk.co.armedpineapple.innoextract).

If you do use Inno Setup Extractor, the game files will be in the `app` folder inside your desired
extraction folder. You can freely delete the `tmp` folder.

The first time you run Julius, the game will notify you that you need to point it to the game folder
location. If you used Inno Setup Extractor, that would be the `app` folder, otherwise point the game to
wherever you downloaded the files to. After setting up the folder for the first time, you will not be
asked to do so again.

Julius has full touch support. For detailed touch usage, please check the
[Touch Support](https://github.com/bvschaik/julius/wiki/Touch-Support) page.

Julius for Android has limited mouse support. Right mouse button clicks are notoriously flaky.
However, if you have a recent Samsung smartphone, Julius is fully compatibly with Samsung DeX.

# Running Augustus

Augustus requires the original Caesar 3 files to run. This can be an old CD-ROM version, or a digital copy
from either [GOG](https://www.gog.com/game/caesar_3) or
[Steam](https://store.steampowered.com/app/517790/Caesar_3/).

There are [command line options](https://github.com/Keriew/augustus/wiki/Command-line-options) which are available to all platforms.

If you have the high-quality MP3 music files that were once available for download from the Sierra Website,
the game supports playing them.

Please check [MP3 support](https://github.com/Keriew/augustus/wiki/MP3-Support) for details.

## Windows

Augustus supports Windows Vista and higher. Windows XP has limited support.

1. Install Caesar 3 using the provided installer (GOG/Steam/CD-ROM).
2. Download the [latest release](https://github.com/keriew/augustus/releases/latest) of Augustus or compile from source.
3. Copy augustus.exe, SDL2.dll, SDL2_mixer.dll and libmpg123.dll to the folder where you installed Caesar 3
4. Run Augustus

**Note:** If you install Caesar 3 using Steam and plan to use Steam to launch the game,
***do not*** rename `augustus.exe` to `c3.exe`.
Doing so will make the mouse cursor disappear when using right-click to scroll.
   
Instead, open `SierraLauncher.ini` and replace `Game1Exe=c3.exe` with the `Game1Exe=augustus.exe`.

Alternatively, you can check the `Disable right click to drag the map` option do disable right-click scrolling.

### Windows XP

Augustus still works on Windows XP, but the most recent version of SDL does not. To get it to work:

1. Follow the general instructions for Windows
2. Download [SDL 2.0.9](http://libsdl.org/release/SDL2-2.0.9-win32-x86.zip)
3. Extract `SDL2.dll` from that zip file to the same folder where you installed Augustus, overwriting the existing file
4. Run Augustus

## Linux/BSD

OpenBSD and FreeBSD provide Augustus as [package](http://ports.su/games/augustus-game).

On Linux you can use the provided [AppImage](https://appimage.org/):

1. Obtain the game data files of Caesar 3 by checking one of the two next sections.
2. Download the [latest AppImage release](https://github.com/keriew/augustus/releases/latest) of Augustus.
3. Make the downloaded AppImage executable by going into the file properties or running
   `chmod +x augustus-*.AppImage` in the same folder as the AppImage.
4. You can then run it just like any Linux executable.
5. (Optional) You can install [AppImageLauncher](https://github.com/TheAssassin/AppImageLauncher#readme)
   in order to integrate the AppImage in your OS. You'll then be able to launch it easily from the menu
   just like other apps.

### GOG version with InnoExtract
If you bought the GOG edition, you can download the offline installer exe, and use
[InnoExtract](http://constexpr.org/innoextract/) to extract the game files:

1. Build Augustus or install using your package manager
2. [Install](http://constexpr.org/innoextract/install) `innoextract` for your distribution (`brew install innoextract` for mac)
3. Download the Caesar 3 offline installer exe from GOG
4. Run the following command to extract the game files to a new `app` directory:

        $ innoextract -m setup_caesar3_2.0.0.9.exe

5. Move the `augustus` executable, `mods` and `maps` directories to the extracted `app` directory and run from there, OR run Augustus
   with the path to the game files as parameter:

        $ augustus path-to-app-directory

Note that your user requires write access to the directory containing the game files, since the
saved games are also stored there.

### Using WINE

Another option is to get the game files by installing Caesar 3 using [WINE](https://www.winehq.org/):

1. Build Augustus or install using your package manager
2. Install Caesar 3 using WINE, take note where the game is installed
3. Run Augustus with the path where the game is installed:

        $ augustus path-to-c3-directory

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
4. Start Augustus, and point the game to the `app` folder that was just extracted

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
5. Start Augustus, and point the game to the `Exe` folder that was just extracted

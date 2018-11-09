# Running Julius

Julius requires the original Caesar 3 files to run. This can be an old CD-ROM version, or a digital copy
from either [GOG](https://www.gog.com/game/caesar_3) or [Steam](https://store.steampowered.com/app/517790/Caesar_3/).

## Windows

1. Install Caesar 3 using the provided installer.
1. Download the latest release of Julius or compile from source.
1. Copy julius.exe, SDL2.dll and SDL2_mixer.dll to the folder where you installed Caesar 3
1. Run julius.exe

## Linux

### GOG version with InnoExtract
If you bought the GOG edition, you can download the offline installer exe, and use
[InnoExtract](http://constexpr.org/innoextract/) to extract the game files:

1. Build Julius
1. [Install](http://constexpr.org/innoextract/install) `innoextract` for your distribution
1. Download the Caesar 3 offline installer exe from GOG
1. Run the following command to extract the game files to a new `app` directory:

        $ innoextract -m setup_caesar3_2.0.0.9.exe

1. Move the `julius` executable to the extracted `app` directory and run from there, OR run Julius with the path to the game files as parameter:

        $ julius path-to-app-directory

Note that your user requires write access to the directory containing the game files, since the saved games are also stored there.

### Using WINE

Another option is to get the game files by installing Caesar 3 using [WINE](https://www.winehq.org/):

1. Build Julius
1. Install the game using WINE, take note where the game is installed
1. Run Julius with the path where the game is installed:

        $ julius path-to-c3-directory

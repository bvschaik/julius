# MP3 playback support in Julius

Julius currently supports the playback of MP3 files instead of the original, low-quality wav files.

If you have the high-quality MP3 files that were once available from the
[Sierra Website](http://web.archive.org/web/20000303064239/www.caesar3.com/downloads.shtml),
you can play them instead of the original files. Alternatively, you can play any MP3 file you like instead
of the original music.

In order to play the files, you must have the library `libmpg123-0` installed. For Windows, if you download
[`SDL2_mixer`](https://www.libsdl.org/projects/SDL_mixer/), the library should be available as a DLL (`libmpg123-0.dll`)
with the download. Simply copy it to the same directory from where you run Julius.

For Apple, if you installed Julius using the provided `dmg` file, `libmpg123-0` is already installed.

For Linux, `libmpg123-0` is usually installed with `SDL_mixer`. If not, you can install it using a package manager.

You can then place some or all of the following files in the main folder where your Caesar 3 data files are:

* `"mp3/ROME1.mp3"` - City background music, pop 0 to 1000;
* `"mp3/ROME2.mp3"` - City background music, pop 1000 to 2000;
* `"mp3/ROME3.mp3"` - City background music, pop 2000 to 5000;
* `"mp3/ROME4.mp3"` - City background music, pop 5000 to 7000;
* `"mp3/ROME5.mp3"` - City background music, pop 7000+;
* `"mp3/Combat_Long.mp3"` - Battle music;
* `"mp3/Combat_Short.mp3"` - Short battle music;
* `"mp3/setup.mp3"` - Main menu music.

For each file that is missing, Julius will play the original wav instead.

The original mp3 files can be downloaded [here](https://bintray.com/bvschaik/caesar3-music/mp3).

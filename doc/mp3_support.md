# MP3 playback support in Julius

Julius currently supports the playback of MP3 files instead of the original, low-quality wav files.

If you have the high-quality MP3 files that were once available from the
[Sierra Website](http://web.archive.org/web/20000303064239/www.caesar3.com/downloads.shtml),
you can play them instead of the original files. Alternatively, you can play any MP3 file you like instead
of the original music.

In order to play the files, you must have the library `libmpg123-0` installed.

For Windows, if you installed Julius using the provided zip file, `libmpg123-0` is already installed.

For macOS, if you installed Julius using the provided `dmg` file, `libmpg123-0` is already installed.

For Linux, `libmpg123-0` is usually installed with `SDL_mixer`. If not, you can install it using a package manager.

Then, create a folder named `mp3` in the main folder where your Caesar 3 data files are. In this folder, place some or all of the following files:

* `ROME1.mp3` - City background music, pop 0 to 1000;
* `ROME2.mp3` - City background music, pop 1000 to 2000;
* `ROME3.mp3` - City background music, pop 2000 to 5000;
* `ROME4.mp3` - City background music, pop 5000 to 7000;
* `ROME5.mp3` - City background music, pop 7000+;
* `Combat_Long.mp3` - Battle music;
* `Combat_Short.mp3` - Short battle music;
* `setup.mp3` - Main menu music.

For each file that is missing, Julius will play the original wav instead.

The original mp3 files can be downloaded [here](https://bintray.com/bvschaik/caesar3-music/mp3).

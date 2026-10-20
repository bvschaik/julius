# Julius for Wii U

Requires the Aroma environment.

Copy `julius.wuhb` to `sd:/wiiu/apps/` and the Caesar 3 files to `sd:/wiiu/apps/julius/`.

## Controls

| Button | Action |
|---|---|
| Left stick / D-pad | Move cursor |
| A / B | Left / right click |
| Right stick | Scroll |
| L / R | Faster / slower cursor |
| ZL / ZR | Rotate map |
| X / Y | Game speed up / down |
| R stick click | Pause |
| L stick click | Cycle legions |
| Plus | Keyboard |
| Minus | Touch mode |

## Building

	$ /opt/devkitpro/portlibs/wiiu/bin/powerpc-eabi-cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
	$ cmake --build build

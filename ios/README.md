# iOS instructions

Download SDL2 and SDL2_mixer source releases and put them in `ext/SDL2/SDL2` and `ext/SDL2/SDL2_mixer` respectively.

run the following commands:

```
	$ mkdir build && cd build
	$ cmake .. -DTARGET_PLATFORM=ios -G Xcode
```

Open the resulting Xcode Project file `julius.xcodeproj`

## Build for Simulator
Choose `julius` in the Build Schemes dropdown, choose a Simulator from the Run Destinations dropdown, and click Run.

## Build for Device
With the Julius project and target selected, set a Bundle Id, and choose a Development Team, then choose `julius` in the Build Schemes dropdown, choose a device from the Run Destinations dropdown, and click Run.

## Running
AirDrop or otherwise download a C3 install folder to your iOS device (or simulator).

Launch Julius. Immediately after the first launch, Julius will display a file-picker dialog.  Navigate to your C3 folder, and choose 'Open'. The C3 files will be *copied* to Julius's application documents directory, and the game will use those files from then on. Use the default location for saved games.
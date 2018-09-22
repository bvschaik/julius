Julius 
====== 
An open source re-implementation of Caesar III 

Building from source  
--------------------
 
Requirements:
  * SDL2
  * SDL2_mixer
  * cmake

After cloning the repo:

	cd julius/
	mkdir -p build
	cd build
	cmake ..
	make

The game data should be in the same directory as in which the binary is run
or it can be run as `julius /path/to/game-data`. You must have permission
to write in that directory as the saves will be stored there.

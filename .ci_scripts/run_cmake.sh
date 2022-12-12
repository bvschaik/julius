#!/usr/bin/env bash

case "$BUILD_TARGET" in
"vita")
	docker exec vitasdk /bin/bash -c "git config --global --add safe.directory /build/git && mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DTARGET_PLATFORM=vita .."
	;;
"switch")
	docker exec switchdev /bin/bash -c "mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DTARGET_PLATFORM=switch .."
	;;
"mac")
	mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DSYSTEM_LIBS=OFF -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" ..
	;;
"appimage")
	mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DSYSTEM_LIBS=OFF -DCMAKE_INSTALL_PREFIX=/usr ..
	;;
"linux")
    if [ ! -z "$MPG123_VERSION" ]
    then
		MPG123_OPT="-DLINK_MPG123=ON"
	fi
	mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DSYSTEM_LIBS=OFF $MPG123_OPT ..
	;;
"android")
	mkdir build
	;;
"emscripten")
	export EMSDK=${PWD}/emsdk
	mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DSYSTEM_LIBS=OFF -DTARGET_PLATFORM=emscripten -DEMSCRIPTEN_LOAD_SDL_PORTS=1 ..
	;;
*)
	mkdir build && cd build && cmake ..
	;;
esac

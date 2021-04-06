#!/usr/bin/env bash

case "$BUILD_TARGET" in
"vita")
	docker exec vitasdk /bin/bash -c "mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DVITA_BUILD=ON .."
	;;
"switch")
	docker exec switchdev /bin/bash -c "mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DSWITCH_BUILD=ON .."
	;;
"mac")
	mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DSYSTEM_LIBS=OFF ..
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
	mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DSYSTEM_LIBS=OFF -DEMSCRIPTEN_BUILD=ON ..
	;;
*)
	mkdir build && cd build && cmake ..
	;;
esac

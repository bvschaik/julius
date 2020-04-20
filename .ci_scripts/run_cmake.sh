#!/usr/bin/env bash

case "$BUILD_TARGET" in
"vita")
	export VITASDK=/usr/local/vitasdk
	export PATH=$VITASDK/bin:$PATH
	mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DVITA_BUILD=ON ..
	;;
"switch")
	export DEVKITPRO=/opt/devkitpro
	export PATH=/opt/devkitpro/devkitA64/bin:/opt/devkitpro/tools/bin:$PATH
	mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DSWITCH_BUILD=ON ..
	;;
"mac")
	mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release ..
	;;
"appimage")
	mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr ..
	;;
"linux")
	mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release ..
	;;
*)
	mkdir build && cd build && cmake ..
	;;
esac

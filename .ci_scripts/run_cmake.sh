#!/usr/bin/env bash

case "$BUILD_TARGET" in
"vita")
	docker exec vitasdk /bin/bash -c "mkdir build && cd build && cmake -DVITA_BUILD=ON .."
	;;
"switch")
	docker exec switchdev /bin/bash -c "mkdir build && cd build && cmake -DSWITCH_BUILD=ON .."
	;;
*)
	mkdir build && cd build && cmake ..
esac

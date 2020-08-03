#!/usr/bin/env bash

case "$BUILD_TARGET" in
"vita")
	docker exec vitasdk /bin/bash -c "cd build && make"
	;;
"switch")
	docker exec switchdev /bin/bash -c "cd build && make"
	;;
"mac")
	cp -r res/mods ./build	
	cd build && make && make install && \
	echo "Creating disk image" && \
	hdiutil create -volname Augustus -srcfolder augustus.app -ov -format UDZO augustus.dmg
	;;
"appimage")
	cp -r res/mods ./build		
	cd build && make && \
	make DESTDIR=AppDir install && \
	cd .. && \
	./.ci_scripts/package_appimage.sh
	;;
"linux")
	cp -r res/mods ./build
	cd build && make && \
	zip -r augustus.zip augustus mods
	;;
*)
	cd build && make 
	;;
esac

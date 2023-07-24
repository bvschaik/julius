#!/usr/bin/env bash

set -e

case "$BUILD_TARGET" in
"vita")
	docker exec vitasdk /bin/bash -c "cd build && make -j4"
	;;
"switch")
	docker exec switchdev /bin/bash -c "cd build && make -j4"
	;;
"mac")
	cp -r res/maps ./build	
	cp -r res/manual ./build
	cd build
	make -j4
	make install
	echo "Creating disk image"
	hdiutil create -volname Augustus -srcfolder augustus.app -ov -format UDZO augustus.dmg
	if [[ "$GITHUB_REF" =~ ^refs/tags/v ]]
	then
		zip -r augustus.zip augustus.dmg maps manual
	else
		zip -r augustus.zip augustus.dmg 	
	fi
	;;
"appimage")
	cp -r res/maps ./build	
	cp -r res/manual ./build	
	cd build
	make -j4
	make DESTDIR=AppDir install
	cd ..
	./.ci_scripts/package_appimage.sh
	if [[ "$GITHUB_REF" =~ ^refs/tags/v ]]	
	then
		zip -r augustus.zip . -i augustus.AppImage maps manual
	else
		zip -r augustus.zip . -i augustus.AppImage	
	fi
	;;
"linux")
    if [ -d res/packed_assets ]
	then
	    cp -r res/packed_assets ./build/assets
	else
		cp -r res/assets ./build
	fi
	cp -r res/maps ./build	
	cp -r res/manual ./build	
	cd build && make -j4
	if [[ "$GITHUB_REF" =~ ^refs/tags/v ]]
	then
		zip -r augustus.zip augustus assets maps manual
	else
		zip -r augustus.zip augustus
	fi
	;;
"android")
	cd android
	if [ ! -f augustus.keystore ]
	then
		COMMAND=assembleDebug
	else
		COMMAND=assembleRelease
	fi
	echo "Running ./gradlew $COMMAND"
	TERM=dumb ./gradlew $COMMAND
	if [ -f augustus/build/outputs/apk/release/augustus-release.apk ]
	then
		cp augustus/build/outputs/apk/release/augustus-release.apk ../build/augustus.apk
	fi
	;;
"emscripten")
	cd build && make -j4
	;;
*)
	cd build && make -j4 && make
	;;
esac

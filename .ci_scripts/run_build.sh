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
"ios")
	cd build
	xcodebuild clean build CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO -scheme augustus
	;;
"flatpak")
	flatpak-builder repo res/com.github.keriew.augustus.json --install-deps-from=flathub --keep-build-dirs
	cp .flatpak-builder/build/augustus/res/version.txt res/version.txt
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
		BUILDTYPE=debug
	else
		COMMAND=assembleRelease
		BUILDTYPE=release
	fi
	echo "Running ./gradlew $COMMAND"
	TERM=dumb ./gradlew $COMMAND
	if [ -f augustus/build/outputs/apk/release/augustus-release.apk ]
	then
		cp augustus/build/outputs/apk/release/augustus-release.apk ../build/augustus.apk
	fi
	cd ..
	if [ ! -f "deps/SDL2-$BUILDTYPE.aar" ]
        then
		cp android/SDL2/build/outputs/aar/SDL2-$BUILDTYPE.aar deps/SDL2-$BUILDTYPE.aar
	fi
	;;
"emscripten")
	cd build && make -j4
	;;
*)
	cd build && make -j4 && make
	;;
esac

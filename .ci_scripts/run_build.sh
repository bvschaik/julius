#!/usr/bin/env bash

set -e

case "$BUILD_TARGET" in
"vita")
	docker exec vitasdk /bin/bash -c "cd build && make"
	;;
"switch")
	docker exec switchdev /bin/bash -c "cd build && make"
	;;
"mac")
	cp -r mods ./build	
	cp -r res/maps ./build	
	cp -r res/augustus_manual.pdf ./build	
	cd build && make && make install && \
	echo "Creating disk image" && \
	hdiutil create -volname Augustus -srcfolder augustus.app -ov -format UDZO augustus.dmg
	if [[ ! -z "$TRAVIS_TAG" ]]
	then
		zip -r augustus.zip augustus.dmg mods maps augustus_manual.pdf
	else
		zip -r augustus.zip augustus.dmg 	
	fi
	;;
"appimage")
	cp -r mods ./build		
	cp -r res/maps ./build	
	cp -r res/augustus_manual.pdf ./build	
	cd build && make && \
	make DESTDIR=AppDir install && \
	cd .. && \
	./.ci_scripts/package_appimage.sh
	if [[ ! -z "$TRAVIS_TAG" ]]
	then
		zip -r augustus.zip augustus.AppImage mods maps augustus_manual.pdf
	else
		zip zip -r augustus.zip . -i augustus.AppImage	
	fi
	;;
"linux")
	cp -r mods ./build
	cp -r res/maps ./build	
	cp -r res/augustus_manual.pdf ./build	
	cd build && make
	if [[ ! -z "$TRAVIS_TAG" ]]
	then
		zip -r augustus.zip augustus mods maps augustus_manual.pdf
	else
		zip -r augustus.zip augustus
	fi
	;;
"android")
	cd android
	if [ ! -f julius.keystore ]
	then
		COMMAND=assembleDebug
	elif [ "$TRAVIS_BRANCH" == "master" ]
	then
		# Use last commit message for release notes
		mkdir -p julius/src/main/play/release-notes/en-US
		git log -1 --pretty=%B > julius/src/main/play/release-notes/en-US/internal.txt
		COMMAND=publishRelease
	else
		COMMAND=assembleRelease
	fi
	TERM=dumb ./gradlew $COMMAND
	if [ -f julius/build/outputs/apk/release/julius-release.apk ]
	then
		cp julius/build/outputs/apk/release/julius-release.apk ../build/julius.apk
	fi
	;;
*)
	cd build && make 
	;;
esac

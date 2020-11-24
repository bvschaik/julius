#!/usr/bin/env bash

set -e

case "$BUILD_TARGET" in
"vita")
	docker exec vitasdk /bin/bash -c "cd build && make"
	;;
"switch")
	docker exec switchdev /bin/bash -c "cd build && make"
	mkdir -p release/julius
	cp -f build/julius.nro release/julius/julius.nro
	cd release && zip -r julius_switch.zip julius
	;;
"mac")
	cd build && make && make test && make install
	echo "Creating disk image"
	hdiutil create -volname Julius -srcfolder julius.app -ov -format UDZO julius.dmg
	;;
"appimage")
	cd build && make && make test
	make DESTDIR=AppDir install
	cd ..
	./.ci_scripts/package_appimage.sh
	;;
"linux")
	cd build && make && make test
	zip julius.zip julius
	;;
"android")
	cd android
	if [ ! -f julius.keystore ]
	then
		COMMAND=assembleDebug
	elif [ "$GITHUB_REF" == "refs/heads/master" ]
	then
		# Use last commit message for release notes
		mkdir -p julius/src/main/play/release-notes/en-US
		git log -1 --pretty=%B > julius/src/main/play/release-notes/en-US/internal.txt
		COMMAND=publishRelease
	else
		COMMAND=assembleRelease
	fi
	echo "Running ./gradlew $COMMAND"
	TERM=dumb ./gradlew $COMMAND
	if [ -f julius/build/outputs/apk/release/julius-release.apk ]
	then
		cp julius/build/outputs/apk/release/julius-release.apk ../build/julius.apk
	fi
	;;
*)
	cd build && make && make test
	;;
esac

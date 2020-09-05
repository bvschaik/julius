#!/usr/bin/env bash

case "$BUILD_TARGET" in
"vita")
	docker exec vitasdk /bin/bash -c "cd build && make"
	;;
"switch")
	docker exec switchdev /bin/bash -c "cd build && make"
	;;
"mac")
	cd build && make && make test && make install && \
	echo "Creating disk image" && \
	hdiutil create -volname Julius -srcfolder julius.app -ov -format UDZO julius.dmg
	;;
"appimage")
	cd build && make && make test && \
	make DESTDIR=AppDir install && \
	cd .. && \
	./.ci_scripts/package_appimage.sh
	;;
"linux")
	cd build && make && make test && \
	zip julius.zip julius
	;;
"android")
	cd android && TERM=dumb ./gradlew assembleRelease && \
	cd .. && cp android/distribution/android/julius/outputs/apk/release/julius-release.apk build/julius.apk
	;;
*)
	cd build && make && make test
	;;
esac

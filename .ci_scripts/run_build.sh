#!/usr/bin/env bash

case "$BUILD_TARGET" in
"vita")
	export VITASDK=/usr/local/vitasdk
	export PATH=$VITASDK/bin:$PATH
	cd build && make
	;;
"switch")
	export DEVKITPRO=/opt/devkitpro
	export PATH=/opt/devkitpro/devkitA64/bin:/opt/devkitpro/tools/bin:$PATH
	cd build && make
	;;
"mac")
	cd build && make && make install && \
	echo "Creating disk image" && \
	hdiutil create -volname Julius -srcfolder julius.app -ov -format UDZO julius.dmg
	;;
"appimage")
	cd build && make && \
	make DESTDIR=AppDir install && \
	cd .. && \
	./.ci_scripts/package_appimage.sh
	;;
"linux")
	cd build && make && \
	zip julius.zip julius
	;;
*)
	cd build && make 
	;;
esac

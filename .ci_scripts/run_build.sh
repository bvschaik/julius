#!/usr/bin/env bash

case "$BUILD_TARGET" in
"vita")
	docker exec vitasdk /bin/bash -c "cd build && make"
	;;
"switch")
	docker exec switchdev /bin/bash -c "cd build && make"
	;;
"mac")
	cd build && make && make test && make install
	echo "Creating disk image"
	hdiutil create -volname Julius -srcfolder julius.app -ov -format UDZO julius.dmg
	cd ..
	;;
*)
	cd build && make && make test
	if [ "$BUILD_TARGET" == "appimage" ];
	then
		make DESTDIR=AppDir install;
	else
		make install;
	fi;

	cd ..
	./.ci_scripts/package_appimage.sh;
	;;
esac

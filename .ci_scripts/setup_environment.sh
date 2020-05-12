#!/usr/bin/env bash

case "$BUILD_TARGET" in
"vita")
	# Note: Using a tagged version of the container to make sure that it's not updated unexpectedly
	# You can update the tag by obtaining a recent one from here: https://hub.docker.com/r/gnuton/vitasdk-docker/tags
	# Make sure that it compiles correctly and runs on a Vita prior to pushing the change
	docker run -d --name vitasdk --workdir /build/git -v "${PWD}:/build/git" gnuton/vitasdk-docker:20190626 tail -f /dev/null
	;;
"switch")
	wget https://github.com/devkitPro/pacman/releases/download/devkitpro-pacman-1.0.1/devkitpro-pacman.deb
	sudo dpkg -i devkitpro-pacman.deb
	sudo dkp-pacman -Syyu --noconfirm devkitA64 devkitpro-pkgbuild-helpers libnx switch-tools switch-sdl2 switch-sdl2_mixer switch-libpng
	;;
esac

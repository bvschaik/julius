#!/usr/bin/env bash

case "$BUILD_TARGET" in
"vita")
	export VITASDK=/usr/local/vitasdk
	export PATH=$VITASDK/bin:$PATH
	sudo apt-get update
	git clone https://github.com/vitasdk/vdpm
	cd vdpm
	./bootstrap-vitasdk.sh
	vdpm libvita2d
	vdpm sdl2
	vdpm sdl2_mixer
	vdpm mpg123
	vdpm libogg
	vdpm libpng
	vdpm libvorbis
	vdpm libmikmod
	vdpm flac
	;;
"switch")
	wget https://github.com/devkitPro/pacman/releases/download/devkitpro-pacman-1.0.1/devkitpro-pacman.deb
	sudo dpkg -i devkitpro-pacman.deb
	sudo dkp-pacman -Syyu --noconfirm devkitA64 devkitpro-pkgbuild-helpers libnx switch-tools switch-sdl2 switch-sdl2_mixer switch-libpng
	;;
esac

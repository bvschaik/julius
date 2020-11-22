#!/usr/bin/env bash

set -e

function get_sdl_lib_url {
  local MODULE=$1
  local VERSION=$2
  local EXT=$3
  if [[ "$MODULE" == "SDL2_mixer" ]]
  then
    local SDL_URL_PATH="projects/SDL_mixer/release"
  else
    local SDL_URL_PATH="release"
  fi
  SDL_LIB_URL="https://www.libsdl.org/$SDL_URL_PATH/$MODULE-$VERSION.$EXT"
}

function install_sdl_lib {
  local MODULE=$1
  local VERSION=$2
  local FILENAME=deps/$MODULE-$VERSION.tar.gz
  local BUILDDIR=deps/build/$MODULE-$VERSION
  local LIBDIR=deps/$MODULE-$VERSION
  local ROOT=$PWD
  if [ ! -f "$FILENAME" ] || [ ! -d "$LIBDIR" ]
  then
    if [ ! -f "$FILENAME" ]
    then
      echo "Downloading $MODULE-$VERSION"
      get_sdl_lib_url $MODULE $VERSION "tar.gz"
      curl -o "$FILENAME" "$SDL_LIB_URL"
    fi
    echo "Building $MODULE-$VERSION"
    mkdir -p deps/build
    mkdir -p $LIBDIR
    tar -zxf "$FILENAME" -C deps/build
    cd $BUILDDIR
    SDL2_CONFIG="$ROOT/deps/SDL2-$SDL_VERSION/bin/sdl2-config" ./configure --prefix=$ROOT/$LIBDIR
    make
    make install
    cd $ROOT
    rm -rf deps/build
  fi
  ln -sf "$ROOT/$LIBDIR" ext/SDL2
}

function install_sdl_macos {
  local MODULE=$1
  local VERSION=$2
  local FILENAME=deps/$MODULE-$VERSION.dmg
  if [ ! -f "$FILENAME" ]
  then
    get_sdl_lib_url $MODULE $VERSION "dmg"
    curl -o "$FILENAME" "$SDL_LIB_URL"
  fi
  local VOLUME=$(hdiutil attach $FILENAME | grep -o '/Volumes/.*')
  mkdir -p ~/Library/Frameworks
  echo "Installing framework:" "/Volumes/SDL2"/*.framework
  cp -rp "$VOLUME"/*.framework ~/Library/Frameworks
  hdiutil detach "$VOLUME"
}

function install_sdl_android {
  local MODULE=$1
  local VERSION=$2
  local DIRNAME=deps/$MODULE-$VERSION
  local FILENAME=$DIRNAME.tar.gz
  if [ ! -f "$FILENAME" ]
  then
    get_sdl_lib_url $MODULE $VERSION "tar.gz"
    curl -o "$FILENAME" "$SDL_LIB_URL"
  fi
  tar -zxf $FILENAME -C ext/SDL2
}

mkdir -p deps
if [ "$BUILD_TARGET" == "appimage" ]
then
  sudo apt-get update && sudo apt-get -y install libgl1-mesa-dev libsdl2-dev libsdl2-mixer-dev
elif [ ! -z "$SDL_VERSION" ] && [ ! -z "$SDL_MIXER_VERSION" ]
then
  if [ "$BUILD_TARGET" == "mac" ]
  then
    install_sdl_macos "SDL2" $SDL_VERSION
    install_sdl_macos "SDL2_mixer" $SDL_MIXER_VERSION
  elif [ "$BUILD_TARGET" == "android" ]
  then
    install_sdl_android "SDL2" $SDL_VERSION
    install_sdl_android "SDL2_mixer" $SDL_MIXER_VERSION
  else
    install_sdl_lib "SDL2" $SDL_VERSION
    install_sdl_lib "SDL2_mixer" $SDL_MIXER_VERSION
  fi
fi

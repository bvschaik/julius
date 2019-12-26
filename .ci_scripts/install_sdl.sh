#!/usr/bin/env bash

travis_retry() {
  local result=0
  local count=1
  while [ $count -le 3 ]
  do
    [ $result -ne 0 ] && {
      echo -e "\n${ANSI_RED}The command \"$@\" failed. Retrying, $count of 3.${ANSI_RESET}\n" >&2
    }
    "$@"
    result=$?
    [ $result -eq 0 ] && break
    count=$(($count + 1))
    sleep 1
  done

  [ $count -gt 3 ] && {
    echo -e "\n${ANSI_RED}The command \"$@\" failed 3 times.${ANSI_RESET}\n" >&2
  }

  return $result
}

function get_sdl_lib_info {
  shopt -s extglob
  SDL_LIB_FULL=$1
  SDL_LIB_NAME=${SDL_LIB_FULL%-*}
  SDL_LIB_MODULE=${SDL_LIB_NAME//SDL?([0-9])/}
  SDL_LIB_VERSION=${SDL_LIB_FULL#*-}
}

function get_sdl_lib_url {
  local EXT=$1
  if [[ $SDL_LIB_FULL == *.* ]]
  then
    if [[ $SDL_LIB_MODULE != "" ]]
    then
      local SDL_URL_PATH="projects/SDL$SDL_LIB_MODULE/release"
    else
      local SDL_URL_PATH="release"
    fi
    SDL_LIB_URL="https://www.libsdl.org/$SDL_URL_PATH/$SDL_LIB_FULL.$EXT"
  else
    SDL_LIB_URL="https://hg.libsdl.org/$SDL_LIB_NAME/archive/$SDL_LIB_VERSION.$EXT"
  fi
  if [ $SDL_LIB_VERSION == "tip" ]
  then
    SDL_LIB_FULL=`find . -maxdepth 1 -name "$SDL_LIB_NAME-*" -printf "%T@ %p" | sort -nr | head -1 | cut -c25-`
  fi
}

function install_sdl_lib {
  get_sdl_lib_info $1
  if [ ! "$(ls -A $SDL_LIB_FULL)" ]
  then
    get_sdl_lib_url "tar.gz"
    travis_retry curl -L $SDL_LIB_URL | tar xz
    cd $SDL_LIB_FULL
    mkdir build
    cd build
    SDL2_CONFIG="${SDL2_CONFIG}" ../configure --prefix=$(pwd)/..
    make
    SDL2_CONFIG="$(pwd)/sdl2-config"
    cp -r build/.libs ../lib
    cd ..
    mv include SDL2 2>/dev/null
    mkdir include
    mv SDL2 include/ 2>/dev/null
    cp SDL_mixer.h include/ 2>/dev/null
    cd ..
  fi
  ln -s "$(pwd)/$LIB" "$(pwd)/ext/SDL2/"
}

function install_sdl_macos {
  get_sdl_lib_info $1
  if [ ! -f $SDL_LIB_FULL/image.dmg ]
  then
    mkdir -p $SDL_LIB_FULL
    get_sdl_lib_url $SDL_LIB_FULL "dmg"
    travis_retry curl -o $SDL_LIB_FULL/image.dmg $SDL_LIB_URL
  fi
  local VOLUME=$(hdiutil attach $SDL_LIB_FULL/image.dmg | grep -o '/Volumes/.*')
  mkdir -p ~/Library/Frameworks
  echo "Installing framework:" "/Volumes/SDL2"/*.framework
  cp -rp "$VOLUME"/*.framework ~/Library/Frameworks
  hdiutil detach "$VOLUME"
}

function download_sdl_android {
  get_sdl_lib_info $1
  if [ ! "$(ls -A $SDL_LIB_FULL)" ]
  then
    get_sdl_lib_url "tar.gz"
    travis_retry curl -L $SDL_LIB_URL | tar xz
  fi
  ln -s "$(pwd)/$SDL_LIB_FULL" "$(pwd)/ext/SDL2/"
}

if [ "$BUILD_TARGET" == "appimage" ]
then
  sudo apt-get update && sudo apt-get -y install libgl1-mesa-dev libsdl2-dev libsdl2-mixer-dev
elif [ ! -z "$SDL_LIB" ] && [ ! -z "$SDL_MIXER_LIB" ]
then
  if [ "$BUILD_TARGET" == "mac" ]
  then
    install_sdl_macos $SDL_LIB
    install_sdl_macos $SDL_MIXER_LIB
  elif [ "$BUILD_TARGET" == "android" ]
  then
    download_sdl_android $SDL_LIB
    download_sdl_android $SDL_MIXER_LIB
  else
    install_sdl_lib $SDL_LIB
    install_sdl_lib $SDL_MIXER_LIB
  fi
fi

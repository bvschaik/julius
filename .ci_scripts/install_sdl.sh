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

function get_sdl_lib_url {
  local LIB=$1
  local EXT=$2
  if [[ $LIB == SDL2-* ]]
  then
    SDL_LIB_URL=https://www.libsdl.org/release/$LIB.$EXT
  else
    SDL_LIB_URL=${LIB#*_}
    SDL_LIB_URL=${SDL_LIB_URL%-*}
    SDL_LIB_URL=https://www.libsdl.org/projects/SDL_$SDL_LIB_URL/release/$LIB.$EXT
  fi
}

function install_sdl_lib {
  local LIB=$1
  if [ -d $LIB/build ]
  then
    cp -r $LIB $LIB-final
    cd $LIB-final/build
  else
    get_sdl_lib_url $LIB "tar.gz"
    travis_retry curl -L $SDL_LIB_URL | tar xz
    cd $LIB
    mkdir build
    cd build
    ../configure
  fi
  make
  sudo make install

  cd ../..
}

function install_sdl_macos {
  local LIB=$1
  local LIB_NAME=${LIB%-*}
  if [ ! -f $LIB/image.dmg ]
  then
    mkdir -p $LIB
    get_sdl_lib_url $LIB "dmg"
    travis_retry curl -o $LIB/image.dmg $SDL_LIB_URL
  fi
  local VOLUME=$(hdiutil attach $LIB/image.dmg | grep -o '/Volumes/.*')
  mkdir -p ~/Library/Frameworks
  echo "Installing framework:" "/Volumes/SDL2"/*.framework
  cp -rp "$VOLUME"/*.framework ~/Library/Frameworks
  hdiutil detach "$VOLUME"
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
  else
    install_sdl_lib $SDL_LIB
    install_sdl_lib $SDL_MIXER_LIB
  fi
fi

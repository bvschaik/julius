#!/usr/bin/env bash

travis_retry() {
  local result=0
  local count=1
  while [ $count -le 3 ]; do
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

function install_sdl_lib {
  if [ -d $1/build ];
  then
    cp -r $1 $1-final;
    cd $1-final/build;
  else
    if [[ $1 == SDL2-* ]];
    then
      SDL_LIB_URL=https://www.libsdl.org/release/$1.tar.gz;
    else
      SDL_LIB_URL=${1#*_};
      SDL_LIB_URL=${SDL_LIB_URL%-*};
      SDL_LIB_URL=https://www.libsdl.org/projects/SDL_$SDL_LIB_URL/release/$1.tar.gz;
    fi;
    travis_retry curl -L $SDL_LIB_URL | tar xz;
    cd $1;
    mkdir build;
    cd build;
    ../configure;
  fi;
  make;
  sudo make install;
  cd ../..;
}

if [ ! "$BUILD_TARGET" = "vita" ] && [ ! "$BUILD_TARGET" = "switch" ];
then
  install_sdl_lib $SDL_LIB
  install_sdl_lib $SDL_MIXER_LIB
fi;

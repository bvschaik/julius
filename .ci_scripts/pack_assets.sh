#!/usr/bin/env bash

set -e

case "$BUILD_TARGET" in
"appimage"|"vita"|"switch"|"android"|"emscripten")
    mkdir res/asset_packer/build && cd res/asset_packer/build
    mv ../../../assets .
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j4
    if ./asset_packer; then
        mv packed_assets ../../../assets
    else
        mv assets ../../../assets
    fi
    ;;
"mac")
    mkdir res/asset_packer/build && cd res/asset_packer/build
    mv ../../../assets .
    cmake -DCMAKE_BUILD_TYPE=Release -DSYSTEM_LIBS=OFF ..
    make -j4
    if ./asset_packer; then
        mv packed_assets ../../../assets
    else
        mv assets ../../../assets
    fi
    ;;
esac

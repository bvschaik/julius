#!/bin/bash

set -e

if [[ "$BUILD_TARGET" == "mac" ]]
then
    DISABLE_SYSTEM_LIBS="-DSYSTEM_LIBS=OFF"
fi

mkdir res/asset_packer/build && cd res/asset_packer/build
cmake -DCMAKE_BUILD_TYPE=Release $DISABLE_SYSTEM_LIBS  ..
make -j4
./asset_packer ../../
if [ $? -ne 0 ]
then
    rm -rf ../../packed_assets
fi

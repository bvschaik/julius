#!/bin/bash

set -e
mkdir res/asset_packer/build && cd res/asset_packer/build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4
./asset_packer ../../
if [ $? -ne 0 ]
then
    rm -rf ../../packed_assets
fi

#!/usr/bin/env bash

build_dir="$(pwd)/build"
release_dir="$(pwd)/release"

if [ "$DEPLOY" = "switch" ]
then
mkdir -p $release_dir/julius
cp -f $build_dir/julius.nro $release_dir/julius/julius.nro
cd $release_dir && zip -r julius_switch.zip julius -x "*.DS_Store" -x "*__MAC*"
fi

#!/usr/bin/env bash

build_dir="$(pwd)/build"

if [ "$DEPLOY" = "switch" ]
then
mkdir -p $build_dir/release/augustus
cp -f $build_dir/augustus.nro $build_dir/release/augustus/augustus.nro
cd $build_dir/release && zip -r ../augustus_switch.zip augustus -x "*.DS_Store" -x "*__MAC*"
fi

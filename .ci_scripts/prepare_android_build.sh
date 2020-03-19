#!/usr/bin/env bash

mkdir build

# Install Android SDK packages
mkdir -p $HOME/.android
touch $HOME/.android/repositories.cfg

for PACKAGE in 'ndk-bundle' 'cmake;3.6.4111459'
do
  echo "Installing $PACKAGE..."
  yes | sdkmanager $PACKAGE > /dev/null
done

#!/usr/bin/env bash

cd build

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/
echo "Available libraries on this system ==>\n$(ldconfig -p)"

#wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget https://artifacts.assassinate-you.net/artifactory/list/linuxdeploy/travis-478/linuxdeploy-x86_64.AppImage
chmod +x linuxdeploy*.AppImage

./linuxdeploy-x86_64.AppImage --appdir AppDir --output appimage

mv Julius*.AppImage julius.AppImage

cd ..
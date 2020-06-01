#!/usr/bin/env bash

cd build

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/
echo "Available libraries on this system ==>\n$(ldconfig -p)"

wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
chmod +x linuxdeploy*.AppImage

./linuxdeploy-x86_64.AppImage --appdir AppDir --output appimage

mv Augustus*.AppImage augustus.AppImage

cd ..

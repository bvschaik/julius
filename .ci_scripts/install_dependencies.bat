cd ..
mkdir tmp
cd tmp
curl -o SDL2-devel-2.0.16-VC.zip https://www.libsdl.org/release/SDL2-devel-2.0.16-VC.zip
curl -o SDL2_mixer-devel-2.0.4-VC.zip https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-devel-2.0.4-VC.zip
tar -xf SDL2-devel-2.0.16-VC.zip -C ../ext/SDL2
tar -xf SDL2_mixer-devel-2.0.4-VC.zip -C ../ext/SDL2
cd ../.ci_scripts

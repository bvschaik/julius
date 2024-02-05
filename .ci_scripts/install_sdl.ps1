function CheckSuccess($message) {
    if (!$?) { throw $message }
}

mkdir -Force deps

if ("${env:COMPILER}" -eq "msvc") {
    if ((Test-Path deps/SDL.zip) -and (Test-Path deps/SDL_mixer.zip)) {
        echo "Using cached SDL libraries"
    } else {
        echo "Downloading SDL"
        curl "https://libsdl.org/release/SDL2-devel-${env:SDL_VERSION}-VC.zip" -o deps/SDL.zip
        CheckSuccess("Download SDL")
        curl "https://libsdl.org/projects/SDL_mixer/release/SDL2_mixer-devel-${env:SDL_MIXER_VERSION}-VC.zip" -o deps/SDL_mixer.zip
        CheckSuccess("Download SDL mixer")
    }
    echo "Extracting SDL"
    7z x deps\SDL.zip -oext\SDL2
    CheckSuccess("Unpack SDL")
    7z x deps\SDL_mixer.zip -oext\SDL2
    CheckSuccess("Unpack SDL mixer")
} elseif ("${env:COMPILER}" -eq "msvc-arm64") {
    $Env:SDL2_DIR = $PWD.Path + "\deps\SDL2"
    mkdir -Force $Env:SDL2_DIR
    $Env:SDL2_MIXER_DIR = $PWD.Path + "\deps\SDL2_mixer"
    mkdir -Force $Env:SDL2_MIXER_DIR
    if ((Test-Path "${Env:SDL2_DIR}\SDL2.dll") -and (Test-Path "${Env:SDL2_MIXER_DIR}\SDL2_mixer.dll")) {
        echo "Using cached SDL libraries"
    } else {
        echo "Downloading SDL and SDL_mixer Sources"
        curl "https://libsdl.org/release/SDL2-${env:SDL_VERSION}.zip" -o SDL.zip
        CheckSuccess("Download SDL")
        curl "https://libsdl.org/projects/SDL_mixer/release/SDL2_mixer-${env:SDL_MIXER_VERSION}.zip" -o SDL_mixer.zip
        CheckSuccess("Download SDL mixer")

        7z x SDL.zip
        7z x SDL_mixer.zip

        CheckSuccess("Unpack SDL and SDL mixer")

        cd "SDL2-${env:SDL_VERSION}"
        mkdir build
        cd build

        cmake -G "Visual Studio 17 2022" -A ARM64 -DCMAKE_BUILD_TYPE=Release ..
        cmake --build . -j 4 --config Release
        mv include-config-release\SDL2\*.h include\SDL2
        mv include\SDL2 $Env:SDL2_DIR
        mv Release\SDL2* $Env:SDL2_DIR

        CheckSuccess("Build SDL")

        cd ..\..
        cd "SDL2_mixer-${env:SDL_MIXER_VERSION}"
        mkdir build
        cd build
          
        cmake -G "Visual Studio 17 2022" -A ARM64 -DCMAKE_BUILD_TYPE=Release -DSDL2MIXER_MP3=ON -DSDL2MIXER_MP3_MINIMP3=ON -DSDL2MIXER_VENDORED=OFF -DSDL2MIXER_SAMPLES=OFF -DSDL2MIXER_FLAC=OFF -DSDL2MIXER_CMD=OFF -DSDL2MIXER_MOD=OFF -DSDL2MIXER_MIDI=OFF -DSDL2MIXER_MIDI_TIMIDITY=OFF -DSDL2MIXER_OPUS=OFF -DSDL2MIXER_VORBIS=STB -DSDL2MIXER_WAVPACK=OFF ..
        cmake --build . -j 4 --config Release

        mv ..\include $Env:SDL2_MIXER_DIR
        mv Release\SDL2_mixer* $Env:SDL2_MIXER_DIR

        CheckSuccess("Build SDL mixer")

        cd ..\..
    }

    cp -r $Env:SDL2_DIR ext\SDL2
    cp -r $Env:SDL2_MIXER_DIR ext\SDL2
} else {
    if ((Test-Path deps/SDL.tar.gz) -and (Test-Path deps/SDL_mixer.tar.gz)) {
        echo "Using cached SDL libraries"
    } else {
        echo "Downloading SDL"
        curl "https://libsdl.org/release/SDL2-devel-${env:SDL_VERSION}-mingw.tar.gz" -o deps/SDL.tar.gz
        CheckSuccess("Download SDL")
        curl "https://libsdl.org/projects/SDL_mixer/release/SDL2_mixer-devel-${env:SDL_MIXER_VERSION}-mingw.tar.gz" -o deps/SDL_mixer.tar.gz
        CheckSuccess("Download SDL mixer")
    }
    echo "Extracting SDL"
    tar -zxf deps\SDL.tar.gz -C ext\SDL2
    CheckSuccess("Unpack SDL")
    tar -zxf deps\SDL_mixer.tar.gz -C ext\SDL2
    CheckSuccess("Unpack SDL mixer")
}

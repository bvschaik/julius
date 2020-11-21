function CheckSuccess($message) {
    if (!$?) { throw $message }
}

mkdir deps
if ("${env:COMPILER}" -eq "msvc") {
    echo "Downloading SDL"
    curl "https://libsdl.org/release/SDL2-devel-${env:SDL_VERSION}-VC.zip" -o deps/SDL.zip
    CheckSuccess("Download SDL")
    curl "https://libsdl.org/projects/SDL_mixer/release/SDL2_mixer-devel-${env:SDL_MIXER_VERSION}-VC.zip" -o deps/SDL_mixer.zip
    CheckSuccess("Download SDL mixer")
    echo "Extracting SDL"
    7z x deps\SDL.zip -oext\SDL2
    CheckSuccess("Unpack SDL")
    7z x deps\SDL_mixer.zip -oext\SDL2
    CheckSuccess("Unpack SDL mixer")
} else {
    echo "Downloading SDL"
    curl "https://libsdl.org/release/SDL2-devel-${env:SDL_VERSION}-mingw.tar.gz" -o deps/SDL.tar.gz
    CheckSuccess("Download SDL")
    curl "https://libsdl.org/projects/SDL_mixer/release/SDL2_mixer-devel-${env:SDL_MIXER_VERSION}-mingw.tar.gz" -o deps/SDL_mixer.tar.gz
    CheckSuccess("Download SDL mixer")
    echo "Extracting SDL"
    tar -zxf deps\SDL.tar.gz -C ext\SDL2
    CheckSuccess("Unpack SDL")
    tar -zxf deps\SDL_mixer.tar.gz -C ext\SDL2
    CheckSuccess("Unpack SDL mixer")
}

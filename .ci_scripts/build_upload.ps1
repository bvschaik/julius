function CopyFile($from, $to) {
    cp "$from" "$to"
    if (!$?) {
        throw "Unable to copy $from to $to"
    }
}

$version = Get-Content -TotalCount 1 res\version.txt

$repo = ""
if ("$env:GITHUB_REF" -match "^refs/tags/v") {
    $repo = "release"
} elseif ("$env:GITHUB_REF" -eq "refs/heads/master") {
    $repo = "development"
} elseif ("$env:GITHUB_REF" -eq "refs/heads/release") {
    $repo = "experimental"
} elseif ("$env:GITHUB_REF" -match "^refs/pull/(.*)/merge") {
    $pr_id = $matches[1];
    $version = "pr-$pr_id-$version"
} else {
    echo "Unknown branch type: ${env:GITHUB_REF} - skipping upload"
    exit
}

# Create deploy file
mkdir deploy
if ("${env:COMPILER}" -eq "msvc") {
    $suffix = "windows-msvc"
    CopyFile build/Release/augustus.exe .
    CopyFile ext\SDL2\SDL2-${env:SDL_VERSION}\lib\x64\SDL2.dll .
    CopyFile ext\SDL2\SDL2_mixer-${env:SDL_MIXER_VERSION}\lib\x64\SDL2_mixer.dll .
} elseif ("${env:COMPILER}" -eq "mingw-32") {
    $suffix = "windows"
    CopyFile build/augustus.exe .
    CopyFile ext\SDL2\SDL2-${env:SDL_VERSION}\i686-w64-mingw32\bin\SDL2.dll .
    CopyFile ext\SDL2\SDL2_mixer-${env:SDL_MIXER_VERSION}\i686-w64-mingw32\bin\SDL2_mixer.dll .
} elseif ("${env:COMPILER}" -eq "mingw-64") {
    $suffix = "windows-64bit"
    CopyFile build/augustus.exe .
    CopyFile ext\SDL2\SDL2-${env:SDL_VERSION}\x86_64-w64-mingw32\bin\SDL2.dll .
    CopyFile ext\SDL2\SDL2_mixer-${env:SDL_MIXER_VERSION}\x86_64-w64-mingw32\bin\SDL2_mixer.dll .
} else {
    throw "Unknown compiler: ${env:COMPILER}"
}

$deploy_file = "augustus-$version-$suffix.zip"

$packed_assets = $false

if ($repo -eq "release") {
    echo "Packing the assets"

    cd .\res\asset_packer
    mkdir build
    cd build

    cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DSYSTEM_LIBS=OFF -D CMAKE_C_COMPILER=x86_64-w64-mingw32-gcc.exe -D CMAKE_MAKE_PROGRAM=mingw32-make.exe ..
    cmake --build . -j 4 --config Release
    if ($?) {
        .\asset_packer.exe ..\..\
        if ($?) {
            Move-Item -Path ..\..\packed_assets -Destination ..\..\..\assets
            $packed_assets = $true
        }
    }
    if (!$packed_assets) {
        echo "Unable to pack the assets. Using the original folder"
        Move-Item -Path ..\..\assets -Destination ..\..\..\
        $packed_assets = $true
    }

    cd ..\..\..

    xcopy /ei res\maps .\maps
    xcopy /ei res\manual .\manual
    7z a "deploy\$deploy_file" augustus.exe SDL2.dll SDL2_mixer.dll assets maps manual
} else {
    7z a "deploy\$deploy_file" augustus.exe SDL2.dll SDL2_mixer.dll
}

if (!$?) {
    throw "Unable to create $deploy_file"
}

if ($env:SKIP_UPLOAD) {
    echo "Build is configured to skip deploy - skipping upload"
    exit
}

if (!$repo) {
    echo "No repo found - skipping upload"
    exit
}

if (!$env:UPLOAD_TOKEN) {
    echo "No upload token found - skipping upload"
    exit
}

echo "Uploading $deploy_file to $repo/windows/$version"
curl -u "$env:UPLOAD_TOKEN" -T "deploy/$deploy_file" "https://augustus.josecadete.net/upload/$repo/windows/$version/${deploy_file}"
if (!$?) {
    throw "Unable to upload"
}
echo "Uploaded. URL: https://augustus.josecadete.net/$repo.html"

if (!$packed_assets) {
    echo "Packing the assets"

    cd .\res\asset_packer
    mkdir build
    cd build

    cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DSYSTEM_LIBS=OFF -D CMAKE_C_COMPILER=x86_64-w64-mingw32-gcc.exe -D CMAKE_MAKE_PROGRAM=mingw32-make.exe ..
    cmake --build . -j 4 --config Release
    if ($?) {
        .\asset_packer.exe ..\..\
        if ($?) {
            Move-Item -Path ..\..\packed_assets -Destination ..\..\..\assets
            $packed_assets = $true
        }
    }
    if (!$packed_assets) {
        echo "Unable to pack the assets. Using the original folder"
        Move-Item -Path ..\..\assets -Destination ..\..\..\
    }

    cd ..\..\..
}

$assets_file = "assets-$version-$repo.zip"
7z a "$assets_file" assets

echo "Uploading $assets_file to $repo/windows/$version"
curl -u "$env:UPLOAD_TOKEN" -T "$assets_file" "https://augustus.josecadete.net/upload/$repo/assets/$version/${assets_file}"
if (!$?) {
    throw "Unable to upload assets"
}
echo "Assets uploaded"

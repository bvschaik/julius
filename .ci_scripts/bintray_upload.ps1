function CopyFile($from, $to) {
    cp "$from" "$to"
    if (!$?) {
        throw "Unable to copy $from to $to"
    }
}

$version = Get-Content -TotalCount 1 res\version.txt

$repo = ""
if ("$env:GITHUB_REF" -match "^refs/tags/v") {
    $repo = "julius"
} elseif ("$env:GITHUB_REF" -eq "refs/heads/master") {
    $repo = "julius-dev"
} elseif ("$env:GITHUB_REF" -match "^refs/heads/feature/(.*)") {
    $repo = "julius-branches"
    $version = $matches[1]
} elseif ("$env:GITHUB_REF" -match "^refs/pull/(.*)/merge") {
    $pr_id = $matches[1];
    $version = "pr-$pr_id-$version"
} else {
    echo "Unknown branch type: ${env:GITHUB_REF} - skipping deploy"
    exit
}

# Create deploy file
mkdir deploy
if ("${env:COMPILER}" -eq "msvc") {
    $suffix = "windows-msvc"
    CopyFile build/Release/julius.exe .
    CopyFile ext\SDL2\SDL2-${env:SDL_VERSION}\lib\x64\SDL2.dll .
    CopyFile ext\SDL2\SDL2_mixer-${env:SDL_MIXER_VERSION}\lib\x64\SDL2_mixer.dll .
    CopyFile ext\SDL2\SDL2_mixer-${env:SDL_MIXER_VERSION}\lib\x64\libmpg123-0.dll .
} elseif ("${env:COMPILER}" -eq "mingw-32") {
    $suffix = "windows"
    CopyFile build/julius.exe .
    CopyFile ext\SDL2\SDL2-${env:SDL_VERSION}\i686-w64-mingw32\bin\SDL2.dll .
    CopyFile ext\SDL2\SDL2_mixer-${env:SDL_MIXER_VERSION}\i686-w64-mingw32\bin\SDL2_mixer.dll .
    CopyFile ext\SDL2\SDL2_mixer-${env:SDL_MIXER_VERSION}\i686-w64-mingw32\bin\libmpg123-0.dll .
} elseif ("${env:COMPILER}" -eq "mingw-64") {
    $suffix = "windows-64bit"
    CopyFile build/julius.exe .
    CopyFile ext\SDL2\SDL2-${env:SDL_VERSION}\x86_64-w64-mingw32\bin\SDL2.dll .
    CopyFile ext\SDL2\SDL2_mixer-${env:SDL_MIXER_VERSION}\x86_64-w64-mingw32\bin\SDL2_mixer.dll .
    CopyFile ext\SDL2\SDL2_mixer-${env:SDL_MIXER_VERSION}\x86_64-w64-mingw32\bin\libmpg123-0.dll .
} else {
    throw "Unknown compiler: ${env:COMPILER}"
}

$deploy_file = "julius-$version-$suffix.zip"
7z a "deploy\$deploy_file" julius.exe SDL2.dll SDL2_mixer.dll libmpg123-0.dll
if (!$?) {
    throw "Unable to create $deploy_file"
}

if (!$repo) {
    echo "No repo found - skipping deploy to Bintray"
    exit
}

if (!$env:BINTRAY_USER_TOKEN) {
    echo "No user token found - skipping deploy to Bintray"
    exit
}

# Only upload 32-bit build to bintray
if ($suffix -eq "windows") {
    echo "Uploading $deploy_file to $repo/windows/$version"
    curl -u "$env:BINTRAY_USER_TOKEN" -T "deploy/$deploy_file" https://api.bintray.com/content/bvschaik/$repo/windows/$version/$deploy_file?publish=1
    if (!$?) {
        throw "Unable to upload to Bintray"
    }
    echo "\nUploaded to bintray. URL: https://bintray.com/bvschaik/$repo/windows/$version#files"
} else {
    echo "Not publishing build $suffix - skipping deploy to Bintray"
}

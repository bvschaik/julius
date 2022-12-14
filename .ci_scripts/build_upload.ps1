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
} elseif ("$env:GITHUB_REF" -match "^refs/heads/feature/(.*)") {
    $feature = $matches[1];
    $version = "$version-$feature"
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
} elseif ("${env:COMPILER}" -eq "mingw-32") {
    $suffix = "windows"
    CopyFile build/julius.exe .
    CopyFile ext\SDL2\SDL2-${env:SDL_VERSION}\i686-w64-mingw32\bin\SDL2.dll .
    CopyFile ext\SDL2\SDL2_mixer-${env:SDL_MIXER_VERSION}\i686-w64-mingw32\bin\SDL2_mixer.dll .
} elseif ("${env:COMPILER}" -eq "mingw-64") {
    $suffix = "windows-64bit"
    CopyFile build/julius.exe .
    CopyFile ext\SDL2\SDL2-${env:SDL_VERSION}\x86_64-w64-mingw32\bin\SDL2.dll .
    CopyFile ext\SDL2\SDL2_mixer-${env:SDL_MIXER_VERSION}\x86_64-w64-mingw32\bin\SDL2_mixer.dll .
} else {
    throw "Unknown compiler: ${env:COMPILER}"
}

$deploy_file = "julius-$version-$suffix.zip"
7z a "deploy\$deploy_file" julius.exe SDL2.dll SDL2_mixer.dll
if (!$?) {
    throw "Unable to create $deploy_file"
}

if ($env:SKIP_UPLOAD) {
    echo "Build is configured to skip deploy - skipping upload"
    exit
}

if (!$repo) {
    echo "No repo found - skipping deploy"
    exit
}

if (!$env:UPLOAD_TOKEN) {
    echo "No upload token found - skipping upload"
    exit
}

echo "Uploading $deploy_file to $repo/windows/$version"
curl -u "$env:UPLOAD_TOKEN" -T "deploy/$deploy_file" "https://julius.biancavanschaik.nl/upload/$repo/windows/$version/${deploy_file}"
if (!$?) {
    throw "Unable to upload"
}
echo "Uploaded. URL: https://julius.biancavanschaik.nl/"

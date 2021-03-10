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
    CopyFile ext\SDL2\SDL2_mixer-${env:SDL_MIXER_VERSION}\lib\x64\libmpg123-0.dll .
} elseif ("${env:COMPILER}" -eq "mingw-32") {
    $suffix = "windows"
    CopyFile build/augustus.exe .
    CopyFile ext\SDL2\SDL2-${env:SDL_VERSION}\i686-w64-mingw32\bin\SDL2.dll .
    CopyFile ext\SDL2\SDL2_mixer-${env:SDL_MIXER_VERSION}\i686-w64-mingw32\bin\SDL2_mixer.dll .
    CopyFile ext\SDL2\SDL2_mixer-${env:SDL_MIXER_VERSION}\i686-w64-mingw32\bin\libmpg123-0.dll .
} elseif ("${env:COMPILER}" -eq "mingw-64") {
    $suffix = "windows-64bit"
    CopyFile build/augustus.exe .
    CopyFile ext\SDL2\SDL2-${env:SDL_VERSION}\x86_64-w64-mingw32\bin\SDL2.dll .
    CopyFile ext\SDL2\SDL2_mixer-${env:SDL_MIXER_VERSION}\x86_64-w64-mingw32\bin\SDL2_mixer.dll .
    CopyFile ext\SDL2\SDL2_mixer-${env:SDL_MIXER_VERSION}\x86_64-w64-mingw32\bin\libmpg123-0.dll .
} else {
    throw "Unknown compiler: ${env:COMPILER}"
}

CopyFile res\maps .
CopyFile res\augustus_manual.pdf .

$deploy_file = "augustus-$version-$suffix.zip"

if ($repo -eq "release") {
    7z a "deploy\$deploy_file" augustus.exe SDL2.dll SDL2_mixer.dll libmpg123-0.dll assets maps augustus_manual.pdf
} else {
    7z a "deploy\$deploy_file" augustus.exe SDL2.dll SDL2_mixer.dll libmpg123-0.dll
}

if (!$?) {
    throw "Unable to create $deploy_file"
}

if (!$repo) {
    echo "No repo found - skipping upload"
    exit
}

if (!$env:UPLOAD_TOKEN) {
    echo "No upload token found - skipping upload"
    exit
}

# Only upload 32-bit build
if ($suffix -eq "windows") {
    echo "Uploading $deploy_file to $repo/windows/$version"
    curl -u "$env:UPLOAD_TOKEN" -T "deploy/$deploy_file" "https://augustus.josecadete.net/upload/$repo/windows/$version/${deploy_file}"
    if (!$?) {
        throw "Unable to upload"
    }
    echo "Uploaded. URL: https://augustus.josecadete.net/$repo.html"
} else {
    echo "Not publishing build $suffix - skipping upload"
}

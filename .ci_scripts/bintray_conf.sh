#!/usr/bin/env bash

build_dir="$(pwd)/build"

VERSION=$(cat res/version.txt)
if [[ ! -z "$TRAVIS_TAG" ]]
then
  REPO=julius
elif [[ "$TRAVIS_BRANCH" == "master" ]]
then
  REPO=julius-dev
elif [[ "$TRAVIS_BRANCH" =~ ^feature/ ]]
then
  REPO=julius-branches
  VERSION=${TRAVIS_BRANCH##feature/}-$VERSION
else
  echo "Unknown branch type $TRAVIS_BRANCH - skipping deply to Bintray"
  exit
fi

case "$DEPLOY" in
"linux")
  NAME="linux"
  DESC="Linux"
  INCLUDE_PATTERN="${build_dir}/julius.zip"
  UPLOAD_PATTERN="julius-$VERSION-linux-x86_64.zip"
  ;;
"appimage")
  # Linux portable binary: https://appimage.org/
  NAME="linux"
  DESC="Linux AppImage"
  INCLUDE_PATTERN="${build_dir}/julius.AppImage"
  UPLOAD_PATTERN="julius-$VERSION-linux.AppImage"
  ;;
"mac")
  NAME="mac"
  DESC="macOS"
  INCLUDE_PATTERN="${build_dir}/julius.dmg"
  UPLOAD_PATTERN="julius-$VERSION-mac.dmg"
  ;;
"vita")
  NAME="vita"
  DESC="Vita"
  INCLUDE_PATTERN="${build_dir}/julius.vpk"
  UPLOAD_PATTERN="julius-$VERSION-vita.vpk"
  ;;
"switch")
  NAME="switch"
  DESC="Switch"
  INCLUDE_PATTERN="$(pwd)/release/julius_switch.zip"
  UPLOAD_PATTERN="julius-$VERSION-switch.zip"
  ;;
"android")
  NAME="android"
  DESC="Android"
  INCLUDE_PATTERN="${build_dir}/julius.apk"
  UPLOAD_PATTERN="julius-$VERSION-android.apk"
  ;;
*)
  echo "Unknown deploy type $DEPLOY - skipping deply to Bintray"
  exit
  ;;
esac

cat > "bintray.json" <<EOF
{
  "package": {
    "subject": "bvschaik",
    "repo": "$REPO",
    "name": "$NAME",
    "licenses": ["AGPL-V3"],
    "vcs_url": "https://github.com/bvschaik/julius.git"
  },

  "version": {
    "name": "$VERSION",
    "released": "$(date +'%Y-%m-%d')",
    "desc": "Automated $DESC build for Travis-CI job: $TRAVIS_JOB_WEB_URL"
  },

  "files": [
    {
      "includePattern": "$INCLUDE_PATTERN",
      "uploadPattern": "$UPLOAD_PATTERN",
      "listInDownloads": true
    }
  ],

  "publish": true
}
EOF

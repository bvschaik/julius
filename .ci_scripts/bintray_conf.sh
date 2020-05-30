#!/usr/bin/env bash

build_dir="$(pwd)/build"

VERSION=$(cat res/version.txt)
if [[ ! -z "$TRAVIS_TAG" ]]
then
  REPO=Augustus
  NAME_SUFFIX=-release
elif [[ "$TRAVIS_BRANCH" == "master" ]]
then
  REPO=Augustus
  NAME_SUFFIX=-unstable
elif [[ "$TRAVIS_BRANCH" =~ ^feature/ ]]
then
  REPO=Augustus-branches
  NAME_SUFFIX=
  VERSION=${TRAVIS_BRANCH##feature/}-$VERSION
else
  echo "Unknown branch type $TRAVIS_BRANCH - skipping deploy to Bintray"
  exit
fi

if [ "$DEPLOY" = "linux" ]
then
cat > "bintray.json" <<EOF
{
  "package": {
    "subject": "bvschaik",
    "repo": "$REPO",
    "name": "linux",
    "licenses": ["AGPL-V3"],
    "vcs_url": "https://github.com/bvschaik/julius.git"
  },

  "version": {
    "name": "$VERSION",
    "released": "$(date +'%Y-%m-%d')",
    "desc": "Automated Linux build for Travis-CI job: $TRAVIS_JOB_WEB_URL"
  },

  "files": [
    {
      "includePattern": "${build_dir}/julius.zip",
      "uploadPattern": "julius-$VERSION-linux-x86_64.zip"
    }
  ],

  "publish": true
}
EOF
# Linux portable binary: https://appimage.org/
elif [ "$DEPLOY" = "appimage" ]
then
cat > "bintray.json" <<EOF
{
  "package": {
    "subject": "keriew",
    "repo": "$REPO",
    "name": "linux",
    "licenses": ["AGPL-V3"],
    "vcs_url": "https://github.com/Keriew/julius.git"
  },

  "version": {
    "name": "$VERSION",
    "released": "$(date +'%Y-%m-%d')",
    "desc": "Automated Linux AppImage build for Travis-CI job: $TRAVIS_JOB_WEB_URL"
  },

  "files": [
    {
      "includePattern": "${build_dir}/julius.AppImage",
      "uploadPattern": "julius-$VERSION-linux.AppImage"
    }
  ],

  "publish": true
}
EOF
elif [ "$DEPLOY" = "mac" ]
then
cat > "bintray.json" <<EOF
{
  "package": {
    "subject": "keriew",
    "repo": "$REPO",
    "name": "mac",
    "licenses": ["AGPL-V3"],
    "vcs_url": "https://github.com/Keriew/julius.git"
  },

  "version": {
    "name": "$VERSION",
    "released": "$(date +'%Y-%m-%d')",
    "desc": "Automated macOS build for Travis-CI job: $TRAVIS_JOB_WEB_URL"
  },

  "files": [
    {
      "includePattern": "${build_dir}/julius.dmg",
      "uploadPattern": "julius-$VERSION-mac.dmg",
      "listInDownloads": true
    }
  ],

  "publish": true
}
EOF
elif [ "$DEPLOY" = "vita" ]
then
cat > "bintray.json" <<EOF
{
  "package": {
    "subject": "keriew",
    "repo": "$REPO",
    "name": "vita",
    "licenses": ["AGPL-V3"],
    "vcs_url": "https://github.com/Keriew/julius.git"
  },

  "version": {
    "name": "$VERSION",
    "released": "$(date +'%Y-%m-%d')",
    "desc": "Automated Vita build for Travis-CI job: $TRAVIS_JOB_WEB_URL"
  },

  "files": [
    {
      "includePattern": "${build_dir}/julius.vpk",
      "uploadPattern": "julius-$VERSION-vita.vpk",
      "listInDownloads": true
    }
  ],

  "publish": true
}
EOF
elif [ "$DEPLOY" = "switch" ]
then
cat > "bintray.json" <<EOF
{
  "package": {
    "subject": "keriew",
    "repo": "$REPO",
    "name": "switch",
    "licenses": ["AGPL-V3"],
    "vcs_url": "https://github.com/Keriew/julius.git"
  },

  "version": {
    "name": "$VERSION",
    "released": "$(date +'%Y-%m-%d')",
    "desc": "Automated Switch build for Travis-CI job: $TRAVIS_JOB_WEB_URL"
  },

  "files": [
    {
      "includePattern": "${build_dir}/julius.zip",
      "uploadPattern": "julius-$VERSION-switch.zip",
      "listInDownloads": true
    }
  ],

  "publish": true
}
EOF
fi

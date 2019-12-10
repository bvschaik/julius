#!/usr/bin/env bash

build_dir="$(pwd)/build"

VERSION=$(cat res/version.txt)
if [[ ! -z "$TRAVIS_TAG" ]]
then
  REPO=julius
  NAME_SUFFIX=-release
elif [[ "$TRAVIS_BRANCH" == "master" ]]
then
  REPO=julius
  NAME_SUFFIX=-unstable
elif [[ "$TRAVIS_BRANCH" =~ ^feature/ ]]
then
  REPO=julius-branches
  NAME_SUFFIX=
  VERSION=${TRAVIS_BRANCH##feature/}-$VERSION
else
  echo "Unknown branch type $TRAVIS_BRANCH - skipping deply to Bintray"
  exit
fi

# Linux portable binary: https://appimage.org/
if [ "$DEPLOY" = "appimage" ]
then
cat > "bintray.json" <<EOF
{
  "package": {
    "subject": "bvschaik",
    "repo": "$REPO",
    "name": "linux$NAME_SUFFIX",
    "licenses": ["AGPL-V3"],
    "vcs_url": "https://github.com/bvschaik/julius.git"
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
    "subject": "bvschaik",
    "repo": "$REPO",
    "name": "mac$NAME_SUFFIX",
    "licenses": ["AGPL-V3"],
    "vcs_url": "https://github.com/bvschaik/julius.git"
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
    "subject": "bvschaik",
    "repo": "$REPO",
    "name": "vita$NAME_SUFFIX",
    "licenses": ["AGPL-V3"],
    "vcs_url": "https://github.com/bvschaik/julius.git"
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
    "subject": "bvschaik",
    "repo": "$REPO",
    "name": "switch$NAME_SUFFIX",
    "licenses": ["AGPL-V3"],
    "vcs_url": "https://github.com/bvschaik/julius.git"
  },

  "version": {
    "name": "$VERSION",
    "released": "$(date +'%Y-%m-%d')",
    "desc": "Automated Switch build for Travis-CI job: $TRAVIS_JOB_WEB_URL"
  },

  "files": [
    {
      "includePattern": "${build_dir}/julius_switch.zip",
      "uploadPattern": "julius-$VERSION-switch.zip",
      "listInDownloads": true
    }
  ],

  "publish": true
}
EOF
elif [ "$DEPLOY" = "android" ]
then
cat > "bintray.json" <<EOF
{
  "package": {
    "subject": "bvschaik",
    "repo": "$REPO",
    "name": "android$NAME_SUFFIX",
    "licenses": ["AGPL-V3"],
    "vcs_url": "https://github.com/bvschaik/julius.git"
  },

  "version": {
    "name": "$VERSION",
    "released": "$(date +'%Y-%m-%d')",
    "desc": "Automated Android build for Travis-CI job: $TRAVIS_JOB_WEB_URL"
  },

  "files": [
    {
      "includePattern": "${build_dir}/julius.apk",
      "uploadPattern": "julius-$VERSION-android.apk",
      "listInDownloads": true
    }
  ],

  "publish": true
}
EOF
fi

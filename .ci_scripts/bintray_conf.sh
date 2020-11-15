#!/usr/bin/env bash

build_dir="$(pwd)/build"

VERSION=$(cat res/version.txt)
if [[ ! -z "$TRAVIS_TAG" ]]
then
  REPO=Augustus
elif [[ "$TRAVIS_BRANCH" == "master" ]]
then
  REPO=Augustus-unstable
elif [[ "$TRAVIS_BRANCH" == "release" ]]
then
  REPO=Augustus-rc
elif [[ "$TRAVIS_BRANCH" =~ ^feature/ ]]
then
  REPO=Augustus-branches
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
    "subject": "keriew",
    "repo": "$REPO",
    "name": "linux",
    "licenses": ["AGPL-V3"],
    "vcs_url": "https://github.com/Keriew/julius.git"
  },

  "version": {
    "name": "$VERSION",
    "released": "$(date +'%Y-%m-%d')",
    "desc": "Automated Linux build for Travis-CI job: $TRAVIS_JOB_WEB_URL"
  },

  "files": [
    {
      "includePattern": "${build_dir}/augustus.zip",
      "uploadPattern": "augustus-$VERSION-linux-x86_64.zip"
    }
  ],

  "publish": true
}
EOF
elif [ "$DEPLOY" = "appimage" ]
then
cat > "bintray.json" <<EOF
{
  "package": {
    "subject": "keriew",
    "repo": "$REPO",
    "name": "linux",
    "licenses": ["AGPL-V3"],
    "vcs_url": "https://github.com/Keriew/augustus.git"
  },
  "version": {
    "name": "$VERSION",
    "released": "$(date +'%Y-%m-%d')",
    "desc": "Automated Linux AppImage build for Travis-CI job: $TRAVIS_JOB_WEB_URL"
  },
  "files": [
    {
      "includePattern": "${build_dir}/augustus.AppImage",
      "uploadPattern": "augustus-$VERSION-linux.AppImage"
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
    "vcs_url": "https://github.com/Keriew/augustus.git"
  },

  "version": {
    "name": "$VERSION",
    "released": "$(date +'%Y-%m-%d')",
    "desc": "Automated macOS build for Travis-CI job: $TRAVIS_JOB_WEB_URL"
  },

  "files": [
    {
      "includePattern": "${build_dir}/augustus.zip",
      "uploadPattern": "augustus-$VERSION-mac.zip",
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
    "vcs_url": "https://github.com/Keriew/augustus.git"
  },

  "version": {
    "name": "$VERSION",
    "released": "$(date +'%Y-%m-%d')",
    "desc": "Automated Vita build for Travis-CI job: $TRAVIS_JOB_WEB_URL"
  },

  "files": [
    {
      "includePattern": "${build_dir}/augustus.vpk",
      "uploadPattern": "augustus-$VERSION-vita.vpk",
      "listInDownloads": true
    }
  ],

cat > "bintray.json" <<EOF
{
  "package": {
    "subject": "keriew",
    "repo": "$REPO",
    "name": "$NAME",
    "licenses": ["AGPL-V3"],
    "vcs_url": "https://github.com/Keriew/augustus.git"
  },

  "version": {
    "name": "$VERSION",
    "released": "$(date +'%Y-%m-%d')",
    "desc": "Automated $DESC build for Travis-CI job: $TRAVIS_JOB_WEB_URL"
  },

  "files": [
    {
      "includePattern": "${build_dir}/augustus.zip",
      "uploadPattern": "augustus-$VERSION-switch.zip",
      "listInDownloads": true
    }
  ],

  "publish": true
}
EOF
fi

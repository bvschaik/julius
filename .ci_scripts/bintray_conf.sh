#!/usr/bin/env bash

build_dir="$(pwd)/build"

VERSION=$(cat res/version.txt)

if [ "$DEPLOY" = "mac" ]
then
cat > "bintray.json" <<EOF
{
  "package": {
    "subject": "bvschaik",
    "repo": "julius",
    "name": "mac-unstable",
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
    "repo": "julius",
    "name": "vita-unstable",
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
    "repo": "julius",
    "name": "switch-unstable",
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
fi

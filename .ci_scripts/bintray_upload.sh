#!/bin/bash

set -e

mkdir deploy
build_dir="$(pwd)/build"

VERSION=$(cat res/version.txt)
if [[ "$GITHUB_REF" =~ ^refs/tags/v ]]
then
  REPO=julius
elif [[ "$GITHUB_REF" == "refs/heads/master" ]]
then
  REPO=julius-dev
elif [[ "$GITHUB_REF" =~ ^refs/heads/feature/ ]]
then
  REPO=julius-branches
  VERSION=${GITHUB_REF##refs/heads/feature/}-$VERSION
elif [[ "$GITHUB_REF" =~ ^refs/pull/ ]]
then
  PR_ID=${GITHUB_REF##refs/pull/}
  PR_ID=${PR_ID%%/merge}
  VERSION=pr-$PR_ID-$VERSION
else
  echo "Unknown branch type $GITHUB_REF - skipping deploy to Bintray"
  exit
fi

DEPLOY_FILE=
case "$DEPLOY" in
"linux")
  PACKAGE=linux
  DEPLOY_FILE=julius-$VERSION-linux-x86_64.zip
  cp "${build_dir}/julius.zip" "deploy/$DEPLOY_FILE"
  ;;
"appimage")
  PACKAGE=linux
  DEPLOY_FILE=julius-$VERSION-linux.AppImage
  cp "${build_dir}/julius.AppImage" "deploy/$DEPLOY_FILE"
  ;;
"mac")
  PACKAGE=mac
  DEPLOY_FILE=julius-$VERSION-mac.dmg
  cp "${build_dir}/julius.dmg" "deploy/$DEPLOY_FILE"
  ;;
"vita")
  ls -l "${build_dir}"
  whoami
  PACKAGE=vita
  DEPLOY_FILE=julius-$VERSION-vita.vpk
  cp "${build_dir}/julius.vpk" "deploy/$DEPLOY_FILE"
  ;;
"switch")
  PACKAGE=switch
  DEPLOY_FILE=julius-$VERSION-switch.zip
  cp "release/julius_switch.zip" "deploy/$DEPLOY_FILE"
  ;;
"android")
  PACKAGE=android
  if [ -f "${build_dir}/julius.apk" ]
  then
    DEPLOY_FILE=julius-$VERSION-android.apk
    cp "${build_dir}/julius.apk" "deploy/$DEPLOY_FILE"
  fi
  ;;
*)
  echo "Unknown deploy type $DEPLOY - skipping deploy to Bintray"
  exit
  ;;
esac

if [ -z "$REPO" ] || [ -z "$DEPLOY_FILE" ]
then
  echo "No repo or deploy file found - skipping deploy to Bintray"
  exit
fi

if [ -z "$BINTRAY_USER_TOKEN" ]
then
  echo "No user token found - skipping deploy to Bintray"
  exit
fi

curl -u "$BINTRAY_USER_TOKEN" -T deploy/$DEPLOY_FILE https://api.bintray.com/content/bvschaik/$REPO/$PACKAGE/$VERSION/$DEPLOY_FILE?publish=1
echo "\nUploaded to bintray. URL: https://bintray.com/bvschaik/$REPO/$PACKAGE/$VERSION#files"

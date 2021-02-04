#!/bin/bash

set -e

mkdir deploy
build_dir="$(pwd)/build"

VERSION=$(cat res/version.txt)
if [[ "$GITHUB_REF" =~ ^refs/tags/v ]]
then
  REPO=Augustus
elif [[ "$GITHUB_REF" == "refs/heads/master" ]]
then
  REPO=Augustus-unstable
elif [[ "$GITHUB_REF" == "refs/heads/release/" ]]
then
  REPO=Augustus-rc
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
  DEPLOY_FILE=augustus-$VERSION-linux-x86_64.zip
  cp "${build_dir}/augustus.zip" "deploy/$DEPLOY_FILE"
  ;;
"vita")
  PACKAGE=vita
  DEPLOY_FILE=augustus-$VERSION-vita.vpk
  cp "${build_dir}/augustus.vpk" "deploy/$DEPLOY_FILE"
  ;;
"switch")
  PACKAGE=switch
  DEPLOY_FILE=augustus-$VERSION-switch.nro
  cp "${build_dir}/augustus.nro" "deploy/$DEPLOY_FILE"
  ;;
"appimage")
  PACKAGE=linux
  DEPLOY_FILE=augustus-$VERSION-linux.AppImage
  cp "${build_dir}/augustus.AppImage" "deploy/$DEPLOY_FILE"
  ;;
"mac")
  PACKAGE=mac
  DEPLOY_FILE=augustus-$VERSION-mac.dmg
  cp "${build_dir}/augustus.dmg" "deploy/$DEPLOY_FILE"
  ;;
*)
  echo "Unknown deploy type $DEPLOY - skipping deploy to Bintray"
  exit
  ;;
esac

if [ ! -z "$SKIP_BINTRAY" ]
then
  echo "Build is configured to skip Bintray deploy - skipping deploy to Bintray"
  exit
fi

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

curl -u "$BINTRAY_USER_TOKEN" -T deploy/$DEPLOY_FILE https://api.bintray.com/content/keriew/$REPO/$PACKAGE/$VERSION/$DEPLOY_FILE?publish=1
echo "\nUploaded to bintray. URL: https://bintray.com/keriew/$REPO/$PACKAGE/$VERSION#files" 

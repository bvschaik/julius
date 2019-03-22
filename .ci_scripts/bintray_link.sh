#!/usr/bin/env bash

RELEASE=$(grep '"name":' bintray.json | head -n1 | grep -o -E '[^":, ]+' | tail -n1)
VERSION=$(cat res/version.txt)

echo "Bintray link: https://bintray.com/bvschaik/julius/$RELEASE/$VERSION"

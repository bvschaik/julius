#!/usr/bin/env bash

REPO=$(grep '"repo":' bintray.json | head -n1 | grep -o -E '[^":, ]+' | tail -n1)
RELEASE=$(grep '"name":' bintray.json | head -n1 | grep -o -E '[^":, ]+' | tail -n1)
VERSION=$(grep '"name":' bintray.json | tail -n1 | grep -o -E '[^":, ]+' | tail -n1)

echo "Bintray link: https://bintray.com/bvschaik/$REPO/$RELEASE/$VERSION#files"

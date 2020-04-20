#!/usr/bin/env bash

case "$BUILD_TARGET" in
"vita")
	# Note: Using a tagged version of the container to make sure that it's not updated unexpectedly
	# You can update the tag by obtaining a recent one from here: https://hub.docker.com/r/gnuton/vitasdk-docker/tags
	# Make sure that it compiles correctly and runs on a Vita prior to pushing the change
	docker run -d --name vitasdk --workdir /build/git -v "${PWD}:/build/git" gnuton/vitasdk-docker:20190626 tail -f /dev/null
	;;
"switch")
	docker run -d --name switchdev --workdir /build/git -v "${PWD}:/build/git" devkitpro/devkita64:20200528 tail -f /dev/null
	docker exec switchdev /bin/bash -c "apt-get update && apt-get install -y --no-install-recommends cmake"
	;;
"android")
	# Install Android SDK packages
	mkdir -p $HOME/.android
	touch $HOME/.android/repositories.cfg

	for PACKAGE in 'ndk-bundle' 'cmake;3.6.4111459'
	do
	  echo "Installing $PACKAGE..."
	  yes | sdkmanager $PACKAGE > /dev/null
	done
	;;
esac

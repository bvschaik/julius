#!/usr/bin/env bash

case "$BUILD_TARGET" in
"vita")
	# Note: Using a tagged version of the container to make sure that it's not updated unexpectedly
	# You can update the tag by obtaining a recent one from here: https://hub.docker.com/r/gnuton/vitasdk-docker/tags
	# Make sure that it compiles correctly and runs on a Vita prior to pushing the change
	docker run -d --name vitasdk --workdir /build/git -v "${PWD}:/build/git" gnuton/vitasdk-docker:20220510 tail -f /dev/null
	;;
"switch")
	docker run -d --name switchdev --workdir /build/git -v "${PWD}:/build/git" devkitpro/devkita64:20200730 tail -f /dev/null
	;;
"android")
	# Decrypt the key files
	if [ "$FILE_ENCRYPTION_KEY" ]
	then
        openssl aes-256-cbc -K $FILE_ENCRYPTION_KEY -iv $FILE_ENCRYPTION_IV -in android/augustus.keystore.enc -out android/augustus.keystore -d;
        # openssl aes-256-cbc -K $FILE_ENCRYPTION_KEY -iv $FILE_ENCRYPTION_IV -in android/play-publisher.json.enc -out android/play-publisher.json -d;
	fi
	;;
"emscripten")
	# Get EMSDK
	git clone https://github.com/emscripten-core/emsdk.git
	cd emsdk
	./emsdk install latest
	./emsdk activate latest
	;;
esac

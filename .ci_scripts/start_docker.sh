#!/usr/bin/env bash

case "$BUILD_TARGET" in
"vita")
	docker run -d --name vitasdk --workdir /build/git -v "${PWD}:/build/git" gnuton/vitasdk-docker tail -f /dev/null
	;;
"switch")
	docker run -d --name switchdev --workdir /build/git -v "${PWD}:/build/git" rsn8887/switchdev tail -f /dev/null
	;;
esac

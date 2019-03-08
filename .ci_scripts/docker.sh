if [ $VITA_BUILD = true ];
then
  docker run -d --name vitasdk --workdir /build/git -v "${PWD}:/build/git" gnuton/vitasdk-docker tail -f /dev/null
fi


if [ ! "$VITA_BUILD" = "true" ];
then
  cd build && make && make test
else
  docker exec vitasdk /bin/bash -c "cd build && make"
fi;

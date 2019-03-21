if [ "$VITA_BUILD" = "true" ];
then
  docker exec vitasdk /bin/bash -c "cd build && make"
elif [ "$SWITCH_BUILD" = "true" ];
then
  docker exec switchdev /bin/bash -c "cd build && make"
else
  cd build && make && make test
fi;


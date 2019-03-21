if [ "$VITA_BUILD" = "true" ];
then
  docker exec vitasdk /bin/bash -c "mkdir build && cd build && cmake -DVITA_BUILD=ON .."
elif [ "$SWITCH_BUILD" = "true" ];
then
  docker exec switchdev /bin/bash -c "mkdir build && cd build && cmake -DSWITCH_BUILD=ON .."
else
  mkdir build && cd build && cmake ..
fi;

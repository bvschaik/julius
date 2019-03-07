if [ ! "$VITA_BUILD" = "true" ];
then
  mkdir build && cd build && cmake ..
else
  docker exec vitasdk /bin/bash -c "mkdir build && cd build && cmake -DVITA_BUILD=ON .."
fi;

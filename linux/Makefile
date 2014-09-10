CC=g++
LDFLAGS=-L../../sdl/lib -lSDL -lSDL_mixer
CFLAGS=-I../../sdl/include/SDL -Wall -Wno-unused-function -g -rdynamic
LD_RUN_PATH=../../sdl/lib

#CFLAGS=-I/usr/include/SDL2 -D_REENTRANT -Wall -Wno-unused-function -g -rdynamic
#LDFLAGS=-L/usr/lib -Wl,-rpath,/usr/lib -lSDL2 -lpthread -lSDL_mixer

c3:
	$(CC) $(CFLAGS) $(LDFLAGS) main.cpp SoundDeviceDummy.cpp ../src/*.cpp ../src/Data/*.cpp ../src/UI/*.cpp -o c3

clean:
	rm -f c3

run: clean c3
	./c3


mixer: mixer2.c
	gcc -g -o mixer2 mixer2.c $(LDFLAGS) $(CFLAGS)

all: clean c3

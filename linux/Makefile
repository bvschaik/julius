CC=g++
LDFLAGS=-L../../sdl/lib -lSDL
CFLAGS=-I../../sdl/include/SDL -Wall

c3:
	$(CC) $(CFLAGS) $(LDFLAGS) main.cpp ../src/*.cpp ../src/Data/*.cpp ../src/UI/*.cpp -o c3

clean:
	rm -f c3

run: clean c3
	./c3

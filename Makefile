INCLUDE=-I glfw/include -I glew/include
LIBRARY=-L glfw/lib/win32 -L glew/lib
FLAGS=-std=c99 -O3

all: main

run: all
	./main

clean:
	rm *.o

main: main.o util.o noise.o
	gcc $(FLAGS) main.o util.o noise.o -o main $(LIBRARY) -lglfw -lglew32 -lopengl32

main.o: main.c
	gcc $(FLAGS) $(INCLUDE) -c -o main.o main.c

util.o: util.c util.h
	gcc $(FLAGS) $(INCLUDE) -c -o util.o util.c

noise.o: noise.c noise.h
	gcc $(FLAGS) $(INCLUDE) -c -o noise.o noise.c

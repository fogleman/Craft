INCLUDE=-I /Users/fogleman/Workspace/glfw-2.7.8/include
LIBRARY=-L /Users/fogleman/Workspace/glfw-2.7.8/lib/cocoa
FLAGS=-std=c99 -O3

all: main

run: all
	./main

clean:
	rm *.o

main: main.o modern.o plasma.o
	gcc $(FLAGS) main.o modern.o -o main $(LIBRARY) -lglfw -framework Cocoa -framework OpenGL

main.o: main.c
	gcc $(FLAGS) $(INCLUDE) -c -o main.o main.c

modern.o: modern.c modern.h
	gcc $(FLAGS) $(INCLUDE) -c -o modern.o modern.c

plasma.o: plasma.c plasma.h
	gcc $(FLAGS) $(INCLUDE) -c -o plasma.o plasma.c

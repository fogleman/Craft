INCLUDE=-I /Users/fogleman/Workspace/glfw-2.7.8/include
LIBRARY=-L /Users/fogleman/Workspace/glfw-2.7.8/lib/cocoa

all: main

run: all
	./main

clean:
	rm *.o

main: main.o modern.o
	gcc main.o modern.o -o main $(LIBRARY) -lglfw -framework Cocoa -framework OpenGL

main.o: main.c
	gcc $(INCLUDE) -c -o main.o main.c

modern.o: modern.c modern.h
	gcc $(INCLUDE) -c -o modern.o modern.c

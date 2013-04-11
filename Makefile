INCLUDE=-I C:\MinGW\include -I C:\Users\mfogleman\Downloads\glfw-2.7.8\include -I C:\Users\mfogleman\Downloads\glew-1.9.0\include
LIBRARY=-L C:\MinGW\lib -L C:\Users\mfogleman\Downloads\glfw-2.7.8\lib\win32 -L C:\Users\mfogleman\Downloads\glew-1.9.0\lib
FLAGS=-std=c99 -O3

all: main

run: all
	./main

clean:
	rm *.o

main: main.o modern.o
	gcc $(FLAGS) main.o modern.o -o main $(LIBRARY) -lglew32 -lglfw -lopengl32 -lglu32

main.o: main.c
	gcc $(FLAGS) $(INCLUDE) -c -o main.o main.c

modern.o: modern.c modern.h
	gcc $(FLAGS) $(INCLUDE) -c -o modern.o modern.c

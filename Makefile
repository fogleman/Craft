#CFLAGS = -O2 -I. -I../../core -I../common -I../../3rdparty/libpng -I../../3rdparty/zlib -I../../3rdparty/linmath -Wall -Wextra
#LDFLAGS = --llvm-lto 1 --closure 1 --embed-file ../../../assets@/ --compression $(EMSCRIPTEN_ROOT)/third_party/lzma.js/lzma-native,$(EMSCRIPTEN_ROOT)/third_party/lzma.js/lzma-decoder.js,LZMA.decompress
CFLAGS = -I./src -I./deps/noise -I./deps/lodepng -I./deps/glew/include -I./deps/glfw/include -Wall -Wextra
LDFLAGS = -framework OpenGL -framework Cocoa -framework IOKit -framework Carbon -framework AGL

SOURCES = src/main.c \
			src/auth.c \
			src/client.c \
			src/cube.c \
			src/db.c \
			src/item.c \
			src/map.c \
			src/matrix.c \
			src/ring.c \
			src/sign.c \
			src/util.c \
			src/world.c \
			deps/noise/noise.c \
			deps/lodepng/lodepng.c \
			deps/glew/src/glew.c
OBJECTS = src/main.o \
			src/auth.o \
			src/client.o \
			src/cube.o \
			src/db.o \
			src/item.o \
			src/map.o \
			src/matrix.o \
			src/ring.o \
			src/sign.o \
			src/util.o \
			src/world.o \
			deps/noise/noise.o \
			deps/lodepng/lodepng.o \
			deps/glew/src/glew.o
TARGET = craft

# Targets start here.
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $(OBJECTS) libglfw3.a

clean:
	$(RM) $(TARGET) $(OBJECTS)

depend:
	@$(CC) $(CFLAGS) -MM $(SOURCES)

# list targets that do not create files (but not all makes understand .PHONY)
.PHONY:	all clean depend

# Dependences (call 'make depend' to generate); do not delete:
# Build for these is implicit, no need to specify compiler command lines.
main.o: src/main.c src/auth.h src/client.h src/config.h src/cube.h \
  src/db.h src/map.h src/sign.h src/item.h src/matrix.h \
  deps/noise/noise.h src/util.h src/world.h
auth.o: src/auth.c src/auth.h
client.o: src/client.c src/client.h
cube.o: src/cube.c src/cube.h src/item.h src/matrix.h src/util.h \
  src/config.h
db.o: src/db.c src/db.h src/map.h src/sign.h src/ring.h
item.o: src/item.c src/item.h src/util.h src/config.h
map.o: src/map.c src/map.h
matrix.o: src/matrix.c src/config.h src/matrix.h src/util.h
ring.o: src/ring.c src/ring.h
sign.o: src/sign.c src/sign.h
util.o: src/util.c deps/lodepng/lodepng.h src/matrix.h src/util.h \
  src/config.h
world.o: src/world.c src/config.h deps/noise/noise.h src/world.h
deps/noise/noise.o: deps/noise/noise.c
deps/lodepng/lodepng.o: deps/lodepng/lodepng.c

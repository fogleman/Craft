#ifndef _glrenderer_h_
#define _glrenderer_h_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Buffer is a wrapper object for the API elements of a buffer object
struct BufferObj;
typedef struct BufferObj *Buffer;

typedef struct {
    GLuint program;
    GLuint position;
    GLuint normal;
    GLuint uv;
    GLuint matrix;
    GLuint sampler;
    GLuint camera;
    GLuint timer;
    GLuint extra1;
    GLuint extra2;
    GLuint extra3;
    GLuint extra4;
} Attrib;

// Generate a buffer object of <size> bytes and initialize with <data> and return its handle
Buffer gen_buffer(int32_t size, float *data);
// Delete the buffer object represented by <buffer>
void del_buffer(Buffer buffer);
// Allocate and return memory for attribs consisting of <components> attribs for <faces> quads
float *malloc_faces(int components, int faces);
// Generate a vertex buffer representing <faces> quads with vertex attributes
// consisting of <components> attributes using <data>  and return its handle
Buffer gen_faces(int components, int faces, float *data);
// Load a program object containing shaders loaded from files found at
// <path1> and <path2> and return its handle
GLuint load_program(const char *path1, const char *path2);
// Draw lines consisting of <count> 2D or 3D vertices with <components> components
// taken from vertex buffer <buffer> using rendering state <attrib>
void draw_lines(Attrib *attrib, Buffer buffer, int components, int count);
// Draw landscape chunk using vertex buffer <buffer> consisting of <faces>
// quads using rendering state <attrib>
void draw_chunk(Attrib *attrib, Buffer buffer, int faces);
// Draw UI placement option represented by vertex buffer <buffer>
// consisting of <count> vertices using rendering state <attrib>
void draw_item(Attrib *attrib, Buffer buffer, int count);
// Draw UI text represented by vertex buffer <buffer> of <length> characters
// using rendering state <attrib>
void draw_text(Attrib *attrib, Buffer buffer, int length);
// Draw text placed on landscape chunks represented by vertex buffer <buffer>
// of <faces> characters using rendering state <attrib>
void draw_signs(Attrib *attrib, Buffer buffer, int faces);
// Draw text currently being applied to a chunk represented by vertex buffer <buffer>
// of <length> characters using rendering state <attrib>
void draw_sign(Attrib *attrib, Buffer buffer, int length);
// Draw UI landscape chunk placement option represented by vertex buffer <buffer>
// using rendering state <attrib>
void draw_cube(Attrib *attrib, Buffer buffer);
// Draw UI plant placement option represented by vertex buffer <buffer>
// using rendering state <attrib>
void draw_plant(Attrib *attrib, Buffer buffer);
// Draw player cube represented by vertex buffer <buffer> using rendering state <attrib>
void draw_player(Attrib *attrib, Buffer buffer);
// Draw large sphere around origin represented by vertex buffer <buffer>
// using rendering state <attrib>
void draw_sky(Attrib *attrib, Buffer buffer);

#endif // _glrenderer_

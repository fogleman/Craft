#ifndef _glrenderer_h_
#define _glrenderer_h_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Image represents texture and framebuffer image information
// It includes whatever is needed to bind it for rendering
struct ImageObj;
typedef struct ImageObj *Image;

// Buffer is a wrapper object for the API elements of a buffer object
struct BufferObj;
typedef struct BufferObj *Buffer;

// Uniform represents the uniform interface to the pipeline shaders
// Maximum allowable textures is 2.
struct UniformObj;
typedef struct UniformObj *Uniform;

typedef struct {
    GLuint program;
    GLuint position;
    GLuint normal;
    GLuint uv;
    GLuint sampler;
    GLuint sampler2;
    GLuint ubo;
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
// Create a uniform interface object containing a ubo of size <ubo_size>
// and textures <texture0> and <texture1> then return the handle.
Uniform gen_uniform(uint32_t ubo_size, Image texture0, Image texture1);
// Destroy <uniform> and free any associated memory.
void del_uniform(Uniform uniform);
// Load and create a texture image from the file located in <path>
// filter and clamp according to <linear> and <clamp> and return the created image object
Image load_tex_image(const char *path, int linear, int clamp);
// Destroy <image> and free any associated resources
void del_image(Image image);
// Bind the <attrib> and <uniform> interfaces for rendering
// and init ubo with <size> bytes of <data>
void bind_pipeline(Attrib *attrib, Uniform uniform, int size, void *data);

#endif // _glrenderer_

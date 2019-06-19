#include <GL/glew.h> // for extension function initializations
#include <stdio.h>   // for printf and fam
#include <stdlib.h>  // for malloc/free
#include "cube.h"
#include "renderer.h"
#include "lodepng.h"
#include "matrix.h"
#include "util.h"

// For GL, the image need only be represented by an integer ID
struct ImageObj {
    GLuint id;
};

// Buffer is a wrapper object for the API elements of a buffer object
// For GL it just contains an integer ID
struct BufferObj {
    GLuint id;
};

// For GL, the uniform consists a ubo and between 0 and 2 texture images
struct UniformObj {
    GLuint ubo;
    Image textures[2];
};

// For GL, the pipeline object just consists of the program ID
struct PipelineObj {
    GLuint program;
};


// Initialize persistent global state for the renderer
// For GL this involves enabling depth testing, face culling,
// and settng the clear color. returns 0 on success.
int init_renderer() {
    if (glewInit() != GLEW_OK) {
        return -1;
    }

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glLogicOp(GL_INVERT);
    glClearColor(0, 0, 0, 1);

    return 0;

} // init_renderer()

// Clear the full framebuffer for the attachments indicated by the <bitfield>
void clear_frame(uint32_t bitfield) {
    if (bitfield & CLEAR_COLOR_BIT)
        glClear(GL_COLOR_BUFFER_BIT);
    if (bitfield & CLEAR_DEPTH_BIT)
        glClear(GL_DEPTH_BUFFER_BIT);
} // clear_frame()

// Clear the a portion of the  framebuffer indicated by <x,y,width,height>
// for the attachments indicated by the <bitfield>
void subclear_frame(uint32_t bitfield, int32_t x, int32_t y, int32_t width, int32_t height) {
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y, width, height);
    clear_frame(bitfield);
    glDisable(GL_SCISSOR_TEST);

} // subclear_frame()

// set viewport for the framebuffer limited to <x,y,width,height>
void set_viewport(int32_t x, int32_t y, int32_t width, int32_t height) {
    glViewport(x, y, width, height);
} // set_viewport()

// Generate a buffer object of <size> bytes and initialize with <data>
Buffer gen_buffer(int32_t size, float *data) {
    Buffer buffer = malloc(sizeof(struct BufferObj));
    glGenBuffers(1, &buffer->id);
    glBindBuffer(GL_ARRAY_BUFFER, buffer->id);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return buffer;
} // gen_buffer()

// Generate a buffer object of <size> bytes and initialize with <data> that
// is expected to be changed frequently
// For GL, this means defining it with GL_DYNAMIC_DRAW
Buffer gen_dynamic_buffer(int32_t size, float *data) {
    Buffer buffer = malloc(sizeof(struct BufferObj));
    glGenBuffers(1, &buffer->id);
    glBindBuffer(GL_ARRAY_BUFFER, buffer->id);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return buffer;
} // gen_dynamic_buffer()

// Update the contents of <buffer> with <size> bytes of <data>
void update_buffer(Buffer buffer, int32_t size, float *data) {
    if (!buffer) return;
    glBindBuffer(GL_ARRAY_BUFFER, buffer->id);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

} // update_buffer()

// Delete the buffer object represented by <buffer>
void del_buffer(Buffer buffer) {
    if (!buffer) return;
    glDeleteBuffers(1, &buffer->id);
    free(buffer);
} // del_buffer()

// Allocate and return memory for attribs consisting of <components> attribs for <faces> quads
float *malloc_faces(int components, int faces) {
    return malloc(sizeof(GLfloat) * 6 * components * faces);
} // malloc_faces()

// Generate a vertex buffer representing <faces> quads with vertex attributes
// consisting of <components> attributes using <data>  and return its handle
// <data> will be freed
Buffer gen_faces(int components, int faces, float *data) {
    Buffer buffer = gen_buffer(
        sizeof(GLfloat) * 6 * components * faces, data);
    free(data);
    return buffer;
} // gen_faces()

// Update <buffer> with <faces> quads with vertex attributes
// consisting of <components> attributes using <data>
// <data> will be freed
void update_faces(Buffer buffer, int components, int faces, float *data) {
    update_buffer(buffer,
        sizeof(GLfloat) * 6 * components * faces, data);
    free(data);
} // update_faces()

// Create a shader of <type> kind using code contained in <source> and return its handle
// <source> is expected to be a null terminated string pointing to valid GLSL
// the source is compiled and a shader is created. If compilation fails, an error message
// is printed and the returned handle is invalid
GLuint make_shader(GLenum type, const char *source, int length) {
    GLuint shader = glCreateShader(type);
    glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, source, length);
    glSpecializeShader(shader, "main", 0, NULL, NULL);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        GLchar *info = calloc(length, sizeof(GLchar));
        glGetShaderInfoLog(shader, length, NULL, info);
        fprintf(stderr, "glSpecializeShader failed:\n%s\n", info);
        free(info);
    }
    return shader;
} // make_shader()

// Load a shader of <type> kind from the file represented by <path> and return its handle
// the <path> file is opened and read. It is expected to contain GLSL. If an error reading
// the file occurs, an error message is printed. The contents of the file is used to create
// a new shader. Returns shader handle regardless of success.
static GLuint load_shader(GLenum type, const char *path) {
    int len;
    void *data = load_file(path, &len);
    GLuint result = make_shader(type, data, len);
    free(data);
    return result;
} // load_shader()

// Create a program object containing <shader1> and <shader2> and return its handle
// A program object is created, <shader1> and <shader2> are attached. They are expected
// to be vertex and fragment shaders respectively. If program linking fails, an error
// message is printed. The created program object is returned regardless.
GLuint make_program(GLuint shader1, GLuint shader2) {
    GLuint program = glCreateProgram();
    glAttachShader(program, shader1);
    glAttachShader(program, shader2);
    glLinkProgram(program);
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLint length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        GLchar *info = calloc(length, sizeof(GLchar));
        glGetProgramInfoLog(program, length, NULL, info);
        fprintf(stderr, "glLinkProgram failed: %s\n", info);
        free(info);
    }
    glDetachShader(program, shader1);
    glDetachShader(program, shader2);
    glDeleteShader(shader1);
    glDeleteShader(shader2);
    return program;
} // make_program()

// Load a program object containing shaders loaded from files found at
// <path1> and <path2> and return its handle
// <path1> is loaded and created as a vertex shader.
// <path2> is loaded and created as a fragment shader.
// Any failures print messages, but have no effect on the return value.
GLuint load_program(const char *path1, const char *path2) {
    GLuint shader1 = load_shader(GL_VERTEX_SHADER, path1);
    GLuint shader2 = load_shader(GL_FRAGMENT_SHADER, path2);
    GLuint program = make_program(shader1, shader2);
    return program;
} // load_program()

// Draw triangles consisting of <count> 3D vertices with 10 components
// taken from vertex buffer <buffer>
// This is used to draw omnipresent cube shapes as well as plants.
// The expected components are 3D position coords, 3D normals,
// 2D tex coords and 2 components used for ambient occlusion.
// The <buffer> is bound and used for vertex attribs
// and pointers specified. <count> is used for the number of vertices in the draw call
// Everything is unbound before exit.
static void draw_triangles_3d_ao(Buffer buffer, int count) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer->id);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 10, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 10, (GLvoid *)(sizeof(GLfloat) * 3));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 10, (GLvoid *)(sizeof(GLfloat) * 6));
    glDrawArrays(GL_TRIANGLES, 0, count);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
} // draw_triangles_3d_ao()

// Draw triangles consisting of <count> 3D vertices with 5 components
// taken from vertex buffer <buffer>
// Intended for use rendering sign text onto cubes
// The expected components are 3D position coords and 2D tex coords
// The <buffer> is bound and used for vertex attribs
// and pointers specified. <count> is used for the number of vertices in the draw call
// Everything is unbound before exit.
static void draw_triangles_3d_text(Buffer buffer, int count) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer->id);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 5, 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 5, (GLvoid *)(sizeof(GLfloat) * 3));
    glDrawArrays(GL_TRIANGLES, 0, count);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
} // draw_triangles_3d_text()

// Draw triangles consisting of <count> 3D vertices with 8 components
// taken from vertex buffer <buffer>
// Intended for use rendering the chunk to be applied in the UI
// Differs only from draw_triangles_3d_ao in that there is no ambient occlusion
// The expected components are 3D position coords, 3D normals, and 2D tex coords
// The <buffer> is bound and used for vertex attribs
// and pointers specified. <count> is used for the number of vertices in the draw call
// Everything is unbound before exit.
static void draw_triangles_3d(Buffer buffer, int count) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer->id);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 8, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 8, (GLvoid *)(sizeof(GLfloat) * 3));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 8, (GLvoid *)(sizeof(GLfloat) * 6));
    glDrawArrays(GL_TRIANGLES, 0, count);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
} // draw_triangles_3d()

// Draw triangles consisting of <count> 2D vertices with 4 components
// taken from vertex buffer <buffer>
// Intended for use rendering UI text as character textured quads
// The expected components are 2D position coords and 2D tex coords
// The <buffer> is bound and used for vertex attribs
// and pointers specified. <count> is used for the number of vertices in the draw call
// Everything is unbound before exit.
static void draw_triangles_2d(Buffer buffer, int count) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer->id);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 4, 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 4, (GLvoid *)(sizeof(GLfloat) * 2));
    glDrawArrays(GL_TRIANGLES, 0, count);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
} // draw_triangles_2d()

// Draw lines consisting of <count> 3D vertices
// taken from vertex buffer <buffer> at <width> pixels wide.
// Intended for use rendering 3D highlights or 2D UI elements
// The expected components are 3D position coords and 2D tex coords
// The <buffer> is bound and used for vertex attribs
// and pointers specified. <count> is used for the number of
// vertices in the draw call. Everything is unbound before exit.
void draw_lines(Buffer buffer, int count, float width) {
    glLineWidth(width);
    glEnable(GL_COLOR_LOGIC_OP);
    glBindBuffer(GL_ARRAY_BUFFER, buffer->id);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_LINES, 0, count);
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisable(GL_COLOR_LOGIC_OP);
} // draw_lines()

// Draw landscape chunk using vertex buffer <buffer> consisting of <faces> quads
void draw_chunk(Buffer buffer, int faces) {
    if (!faces) return;
    draw_triangles_3d_ao(buffer, faces * 6);
} // draw_chunk()

// Draw UI placement option represented by vertex buffer <buffer>
// consisting of <count> vertices
// This is the cube or plant in the bottom left that will be placed next
void draw_item(Buffer buffer, int count) {
    draw_triangles_3d_ao(buffer, count);
} // draw_item()

// Draw UI text represented by vertex buffer <buffer> of <length> characters
// Unlike draw_signs, this is strictly 2D.
void draw_text(Buffer buffer, int length) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    draw_triangles_2d(buffer, length * 6);
    glDisable(GL_BLEND);
} // draw_text()

// Draw text placed on landscape chunks represented by vertex buffer <buffer>
// of <faces> characters
// Unlike draw_text, this is strictly 3D.
void draw_signs(Buffer buffer, int faces) {
    if (!faces) return;
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-8, -1024);
    draw_triangles_3d_text(buffer, faces * 6);
    glDisable(GL_POLYGON_OFFSET_FILL);
} // draw_signs()

// Draw text currently being applied to a chunk represented by vertex buffer <buffer>
// of <length> characters
// Unlike draw_signs, this renders only the sign being currently typed.
// Once typing is complete, it will be represented by draw_signs
void draw_sign(Buffer buffer, int length) {
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-8, -1024);
    draw_triangles_3d_text(buffer, length * 6);
    glDisable(GL_POLYGON_OFFSET_FILL);
} // draw_sign()

// Draw UI landscape chunk placement option represented by vertex buffer <buffer>
// This is exclusively the UI element representing the next item that would be placed
void draw_cube(Buffer buffer) {
    draw_item(buffer, 36);
} // draw_cube()

// Draw UI plant placement option represented by vertex buffer <buffer>
// This is exclusively the UI element representing the next item that would be placed
void draw_plant(Buffer buffer) {
    draw_item(buffer, 24);
} // draw_plant()

// Draw player cube represented by vertex buffer <buffer>
// This is only visible during online multiplayer play
void draw_player(Buffer buffer) {
    draw_cube(buffer);
} // draw_player()

// Draw large sphere around origin represented by vertex buffer <buffer>
// This is rendered such that it never moves and always surrounds the player
void draw_sky(Buffer buffer) {
    draw_triangles_3d(buffer, 512 * 3);
} // draw_sky()

// Create a uniform interface object containing a ubo of size <ubo_size>
// and textures <texture0> and <texture1> then return the handle.
Uniform gen_uniform(uint32_t ubo_size, Image texture0, Image texture1) {

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
    glBufferData(GL_UNIFORM_BUFFER, ubo_size, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    Uniform ret_uniform = malloc(sizeof(struct UniformObj));
    ret_uniform->ubo = buffer;
    ret_uniform->textures[0] = texture0;
    ret_uniform->textures[1] = texture1;

    return ret_uniform;

} // gen_uniform()

// Destroy <uniform> and free any associated memory.
void del_uniform(Uniform uniform) {
    glDeleteBuffers(1, &uniform->ubo);
    free(uniform);
} // del_uniform()

// Load and create a texture image from the file located in <path>
// If <linear> is true, apply linear filtering, nearest otherwise. If <clamp> is true
// use clamp to edge wrap mode. Otherwise, use the default repeat mode.
// Return an Image object containing its information
Image load_tex_image(const char *path, int linear, int clamp) {

    GLuint texture;
    uint8_t *data;
    uint32_t width, height;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    if (linear) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    if (clamp) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    data = load_png_texture(path, &width, &height);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, data);
    free(data);

    Image ret_image = malloc(sizeof(struct ImageObj));
    ret_image->id = texture;

    return ret_image;

} // load_tex_image()

// Destroy <image> and free any associated resources
void del_image(Image image) {
    glDeleteTextures(1, &image->id);
    free(image);
} // del_image()

// Create pipeline object containing shaders as extracted from files <path1> and <path2>
// Since the only member of the GL pipeline is the program, this accomplishes no more than
// load program with a thin wrapper.
Pipeline gen_pipeline(const char *path1, const char *path2) {
    Pipeline ret_pipeline = malloc(sizeof(struct PipelineObj));
    ret_pipeline->program = load_program(path1, path2);
    return ret_pipeline;
} // gen_pipeline()

// Bind the <pipeline> and <uniform> interfaces for rendering
// This involves binding the program, ubo, and the non-null textures
void bind_pipeline(Pipeline pipeline, Uniform uniform, int size, void *data) {

    int binding = 0;
    glUseProgram(pipeline->program);
    if (uniform->textures[0]) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, uniform->textures[0]->id);
        binding++;
    }
    if (uniform->textures[1]) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, uniform->textures[1]->id);
        binding++;
    }
    glActiveTexture(GL_TEXTURE0);

    glBindBuffer(GL_UNIFORM_BUFFER, uniform->ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, uniform->ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

} // bind_pipeline()

// Destroy pipeline object <pipeline> and free any associated memory
// Destroys the program and then frees the allocated struct.
void del_pipeline(Pipeline pipeline) {
    glDeleteProgram(pipeline->program);
    free(pipeline);
} // del_pipeline()

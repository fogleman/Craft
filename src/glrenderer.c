#include <stdio.h>   // for printf and fam
#include <stdlib.h>  // for malloc/free
#include "cube.h"
#include "renderer.h"
#include "lodepng.h"
#include "matrix.h"
#include "util.h"

// Buffer is a wrapper object for the API elements of a buffer object
// For GL it just contains an integer ID
struct BufferObj {
    GLuint id;
};

// Generate a buffer object of <size> bytes and initialize with <data>
Buffer gen_buffer(int32_t size, float *data) {
    Buffer buffer = malloc(sizeof(struct BufferObj));
    glGenBuffers(1, &buffer->id);
    glBindBuffer(GL_ARRAY_BUFFER, buffer->id);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return buffer;
} // gen_buffer()

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
Buffer gen_faces(int components, int faces, float *data) {
    Buffer buffer = gen_buffer(
        sizeof(GLfloat) * 6 * components * faces, data);
    free(data);
    return buffer;
} // gen_faces()

// Create a shader of <type> kind using code contained in <source> and return its handle
// <source> is expected to be a null terminated string pointing to valid GLSL
// the source is compiled and a shader is created. If compilation fails, an error message
// is printed and the returned handle is invalid
GLuint make_shader(GLenum type, const char *source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        GLchar *info = calloc(length, sizeof(GLchar));
        glGetShaderInfoLog(shader, length, NULL, info);
        fprintf(stderr, "glCompileShader failed:\n%s\n", info);
        free(info);
    }
    return shader;
} // make_shader()

// Load a shader of <type> kind from the file represented by <path> and return its handle
// the <path> file is opened and read. It is expected to contain GLSL. If an error reading
// the file occurs, an error message is printed. The contents of the file is used to create
// a new shader. Returns shader handle regardless of success.
static GLuint load_shader(GLenum type, const char *path) {
    char *data = load_file(path);
    GLuint result = make_shader(type, data);
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
// taken from vertex buffer <buffer> using rendering state <attrib>
// This is used to draw omnipresent cube shapes as well as plants.
// The expected components are 3D position coords, 3D normals,
// 2D tex coords and 2 components used for ambient occlusion.
// The <buffer> is bound and used for vertex attribs
// The <attrib> is used to determine the attrib indices to be enabled
// and pointers specified. <count> is used for the number of vertices in the draw call
// Everything is unbound before exit.
static void draw_triangles_3d_ao(Attrib *attrib, Buffer buffer, int count) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer->id);
    glEnableVertexAttribArray(attrib->position);
    glEnableVertexAttribArray(attrib->normal);
    glEnableVertexAttribArray(attrib->uv);
    glVertexAttribPointer(attrib->position, 3, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 10, 0);
    glVertexAttribPointer(attrib->normal, 3, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 10, (GLvoid *)(sizeof(GLfloat) * 3));
    glVertexAttribPointer(attrib->uv, 4, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 10, (GLvoid *)(sizeof(GLfloat) * 6));
    glDrawArrays(GL_TRIANGLES, 0, count);
    glDisableVertexAttribArray(attrib->position);
    glDisableVertexAttribArray(attrib->normal);
    glDisableVertexAttribArray(attrib->uv);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
} // draw_triangles_3d_ao()

// Draw triangles consisting of <count> 3D vertices with 5 components
// taken from vertex buffer <buffer> using rendering state <attrib>
// Intended for use rendering sign text onto cubes
// The expected components are 3D position coords and 2D tex coords
// The <buffer> is bound and used for vertex attribs
// The <attrib> is used to determine the attrib indices to be enabled
// and pointers specified. <count> is used for the number of vertices in the draw call
// Everything is unbound before exit.
static void draw_triangles_3d_text(Attrib *attrib, Buffer buffer, int count) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer->id);
    glEnableVertexAttribArray(attrib->position);
    glEnableVertexAttribArray(attrib->uv);
    glVertexAttribPointer(attrib->position, 3, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 5, 0);
    glVertexAttribPointer(attrib->uv, 2, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 5, (GLvoid *)(sizeof(GLfloat) * 3));
    glDrawArrays(GL_TRIANGLES, 0, count);
    glDisableVertexAttribArray(attrib->position);
    glDisableVertexAttribArray(attrib->uv);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
} // draw_triangles_3d_text()

// Draw triangles consisting of <count> 3D vertices with 8 components
// taken from vertex buffer <buffer> using rendering state <attrib>
// Intended for use rendering the chunk to be applied in the UI
// Differs only from draw_triangles_3d_ao in that there is no ambient occlusion
// The expected components are 3D position coords, 3D normals, and 2D tex coords
// The <buffer> is bound and used for vertex attribs
// The <attrib> is used to determine the attrib indices to be enabled
// and pointers specified. <count> is used for the number of vertices in the draw call
// Everything is unbound before exit.
static void draw_triangles_3d(Attrib *attrib, Buffer buffer, int count) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer->id);
    glEnableVertexAttribArray(attrib->position);
    glEnableVertexAttribArray(attrib->normal);
    glEnableVertexAttribArray(attrib->uv);
    glVertexAttribPointer(attrib->position, 3, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 8, 0);
    glVertexAttribPointer(attrib->normal, 3, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 8, (GLvoid *)(sizeof(GLfloat) * 3));
    glVertexAttribPointer(attrib->uv, 2, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 8, (GLvoid *)(sizeof(GLfloat) * 6));
    glDrawArrays(GL_TRIANGLES, 0, count);
    glDisableVertexAttribArray(attrib->position);
    glDisableVertexAttribArray(attrib->normal);
    glDisableVertexAttribArray(attrib->uv);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
} // draw_triangles_3d()

// Draw triangles consisting of <count> 2D vertices with 4 components
// taken from vertex buffer <buffer> using rendering state <attrib>
// Intended for use rendering UI text as character textured quads
// The expected components are 2D position coords and 2D tex coords
// The <buffer> is bound and used for vertex attribs
// The <attrib> is used to determine the attrib indices to be enabled
// and pointers specified. <count> is used for the number of vertices in the draw call
// Everything is unbound before exit.
static void draw_triangles_2d(Attrib *attrib, Buffer buffer, int count) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer->id);
    glEnableVertexAttribArray(attrib->position);
    glEnableVertexAttribArray(attrib->uv);
    glVertexAttribPointer(attrib->position, 2, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 4, 0);
    glVertexAttribPointer(attrib->uv, 2, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 4, (GLvoid *)(sizeof(GLfloat) * 2));
    glDrawArrays(GL_TRIANGLES, 0, count);
    glDisableVertexAttribArray(attrib->position);
    glDisableVertexAttribArray(attrib->uv);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
} // draw_triangles_2d()

// Draw lines consisting of <count> 2D or 3D vertices with <components> components
// taken from vertex buffer <buffer> using rendering state <attrib>
// Intended for use rendering 3D highlights or 2D UI elements
// The expected components are 2D or 3D position coords and 2D tex coords
// The <buffer> is bound and used for vertex attribs
// The <attrib> is used to determine the attrib indices to be enabled
// and pointers specified. <components> determines how many attribs there are
// depending on whether the lines are 2D or 3D <count> is used for the number of
// vertices in the draw call. Everything is unbound before exit.
void draw_lines(Attrib *attrib, Buffer buffer, int components, int count) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer->id);
    glEnableVertexAttribArray(attrib->position);
    glVertexAttribPointer(
        attrib->position, components, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_LINES, 0, count);
    glDisableVertexAttribArray(attrib->position);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
} // draw_lines()

// Draw landscape chunk using vertex buffer <buffer> consisting of <faces>
// quads using rendering state <attrib>
void draw_chunk(Attrib *attrib, Buffer buffer, int faces) {
    if (!faces) return;
    draw_triangles_3d_ao(attrib, buffer, faces * 6);
} // draw_chunk()

// Draw UI placement option represented by vertex buffer <buffer>
// consisting of <count> vertices using rendering state <attrib>
// This is the cube or plant in the bottom left that will be placed next
void draw_item(Attrib *attrib, Buffer buffer, int count) {
    draw_triangles_3d_ao(attrib, buffer, count);
} // draw_item()

// Draw UI text represented by vertex buffer <buffer> of <length> characters
// using rendering state <attrib>
// Unlike draw_signs, this is strictly 2D.
void draw_text(Attrib *attrib, Buffer buffer, int length) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    draw_triangles_2d(attrib, buffer, length * 6);
    glDisable(GL_BLEND);
} // draw_text()

// Draw text placed on landscape chunks represented by vertex buffer <buffer>
// of <faces> characters using rendering state <attrib>
// Unlike draw_text, this is strictly 3D.
void draw_signs(Attrib *attrib, Buffer buffer, int faces) {
    if (!faces) return;
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-8, -1024);
    draw_triangles_3d_text(attrib, buffer, faces * 6);
    glDisable(GL_POLYGON_OFFSET_FILL);
} // draw_signs()

// Draw text currently being applied to a chunk represented by vertex buffer <buffer>
// of <length> characters using rendering state <attrib>
// Unlike draw_signs, this renders only the sign being currently typed.
// Once typing is complete, it will be represented by draw_signs
void draw_sign(Attrib *attrib, Buffer buffer, int length) {
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-8, -1024);
    draw_triangles_3d_text(attrib, buffer, length * 6);
    glDisable(GL_POLYGON_OFFSET_FILL);
} // draw_sign()

// Draw UI landscape chunk placement option represented by vertex buffer <buffer>
// using rendering state <attrib>
// This is exclusively the UI element representing the next item that would be placed
void draw_cube(Attrib *attrib, Buffer buffer) {
    draw_item(attrib, buffer, 36);
} // draw_cube()

// Draw UI plant placement option represented by vertex buffer <buffer>
// using rendering state <attrib>
// This is exclusively the UI element representing the next item that would be placed
void draw_plant(Attrib *attrib, Buffer buffer) {
    draw_item(attrib, buffer, 24);
} // draw_plant()

// Draw player cube represented by vertex buffer <buffer> using rendering state <attrib>
// This is only visible during online multiplayer play
void draw_player(Attrib *attrib, Buffer buffer) {
    draw_cube(attrib, buffer);
} // draw_player()

// Draw large sphere around origin represented by vertex buffer <buffer>
// using rendering state <attrib>
// This is rendered such that it never moves and always surrounds the player
void draw_sky(Attrib *attrib, Buffer buffer) {
    draw_triangles_3d(attrib, buffer, 512 * 3);
} // draw_sky()

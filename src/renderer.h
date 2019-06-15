#ifndef _glrenderer_h_
#define _glrenderer_h_

#include <GL/glew.h>
#include <GLFW/glfw3.h>


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

GLuint gen_buffer(GLsizei size, GLfloat *data);
void del_buffer(GLuint buffer);
GLfloat *malloc_faces(int components, int faces);
GLuint gen_faces(int components, int faces, GLfloat *data);
GLuint load_program(const char *path1, const char *path2);
void draw_lines(Attrib *attrib, GLuint buffer, int components, int count);
void draw_chunk(Attrib *attrib, GLuint buffer, int faces);
void draw_item(Attrib *attrib, GLuint buffer, int count);
void draw_text(Attrib *attrib, GLuint buffer, int length);
void draw_signs(Attrib *attrib, GLuint buffer, int faces);
void draw_sign(Attrib *attrib, GLuint buffer, int length);
void draw_cube(Attrib *attrib, GLuint buffer);
void draw_plant(Attrib *attrib, GLuint buffer);
void draw_player(Attrib *attrib, GLuint buffer);
void draw_sky(Attrib *attrib, GLuint buffer);

#endif // _glrenderer_

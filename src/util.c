#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "lodepng.h"
#include "matrix.h"
#include "util.h"

// TODO: how is n used? what is the range of outputs?
int rand_int(int n) {
    int result;
    while (n <= (result = rand() / (RAND_MAX / n)));
    return result;
}

// TODO: what is the range of outputs?
double rand_double() {
    return (double)rand() / (double)RAND_MAX;
}

// Update frames per second info
// Arguments:
// - fps: fps context pointer
// Returns:
// - no return value
// - modifies fps context
void update_fps(FPS *fps) {
    fps->frames++;
    double now = glfwGetTime();
    double elapsed = now - fps->since;
    if (elapsed >= 1) {
        fps->fps = round(fps->frames / elapsed);
        fps->frames = 0;
        fps->since = now;
    }
}

// Load all file data from path
// Arguments:
// - path: file path to open and to load data from
// Returns:
// - a newly allocated data pointer which must be free'd
char *load_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "fopen %s failed: %d %s\n",
                path, errno, strerror(errno));
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    int length = ftell(file);
    rewind(file);
    char *data = calloc(length + 1, sizeof(char));
    // TODO: assert new data pointer is not NULL (?)
    fread(data, 1, length, file);
    fclose(file);
    return data;
}

GLuint gen_buffer(GLsizei size, GLfloat *data) {
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return buffer;
}

void del_buffer(GLuint buffer) {
    glDeleteBuffers(1, &buffer);
}

// Allocate memory for faces where each face has a certain number
// of float component properties.
// Arguments:
// - components: the number of components each face has
// - faces: number of faces
// Returns:
// - newly allocated space for floats
GLfloat *malloc_faces(int components, int faces) {
    // TODO: why multiply by 6?
    return malloc(sizeof(GLfloat) * 6 * components * faces);
}

// Create and initialize face data
// Arguments:
// - components: number of components per face
// - faces: number of faces
// - data: data to bind to OpenGL context
// Returns:
// - OpenGL buffer handle
GLuint gen_faces(int components, int faces, GLfloat *data) {
    // TODO: why multiply by 6?
    GLuint buffer = gen_buffer(
        sizeof(GLfloat) * 6 * components * faces, data);
    free(data);
    return buffer;
}

// Create a shader program from its source code
// Arguments:
// - type: vertex or fragment shader
// - source: program source code string
// Returns:
// - OpenGL shader handle
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
}

// Load a shader program from a file
// Arguments:
// - type: vertex or fragment shader
// - path: file path to load the shader program from
// Returns:
// - OpenGL shader handle
GLuint load_shader(GLenum type, const char *path) {
    char *data = load_file(path);
    GLuint result = make_shader(type, data);
    free(data);
    return result;
}


// Arguments:
// - shader1: a shader handle to attach
// - shader2: a shader handle to attach
// Returns:
// - returns OpenGL program handle
// - deletes shader1 and shader2 from the context
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
}

// Loads a shader program from files.
// Arguments:
// - path1 : vertex shader file path
// - path2 : fragment shader file path
// Returns:
// - OpenGL program handle
GLuint load_program(const char *path1, const char *path2) {
    GLuint shader1 = load_shader(GL_VERTEX_SHADER, path1);
    GLuint shader2 = load_shader(GL_FRAGMENT_SHADER, path2);
    GLuint program = make_program(shader1, shader2);
    return program;
}

// Notes: assumes 4 channels per image pixel.
void flip_image_vertical(
    unsigned char *data, unsigned int width, unsigned int height)
{
    unsigned int size = width * height * 4;
    unsigned int stride = sizeof(char) * width * 4;
    unsigned char *new_data = malloc(sizeof(unsigned char) * size);
    for (unsigned int i = 0; i < height; i++) {
        unsigned int j = height - i - 1;
        memcpy(new_data + j * stride, data + i * stride, stride);
    }
    memcpy(data, new_data, size);
    free(new_data);
}

// Loads a PNG file as a 2D texture for the current OpenGL texture context.
// Arguments:
// - file_name: the png file to load the texture from
// Returns:
// - no return value
// - modifies OpenGL state by loading the image data into the current 2D texture
void load_png_texture(const char *file_name) {
    unsigned int error;
    unsigned char *data;
    unsigned int width, height;
    error = lodepng_decode32_file(&data, &width, &height, file_name);
    if (error) {
        fprintf(stderr, "load_png_texture %s failed, error %u: %s\n",
                file_name, error, lodepng_error_text(error));
        exit(1);
    }
    flip_image_vertical(data, width, height);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, data);
    free(data);
}

// Tokenize a string by a delimiter
// NOTE: may modify data in "str"
// Arguments:
// - str: input string to tokenize (may be modified). If the argument "str" is
//   NULL, then tokenization continues from where the argument "key" points to.
// - delim: null-terminated character list of delimiter characters to use as 
//   token separators
// - key: pointer to keep track of where in the string is currently being
//   tokenized
// Returns:
// - a null-terminated C string token
// - modifies "str" so that there are null-terminators after each token
// - modifies where "key" points to
char *tokenize(char *str, const char *delim, char **key) {
    char *result;
    // If str is not given, use the position saved in key
    if (str == NULL) {
        str = *key;
    }
    // Skip over the length of the delimiter substring found
    str += strspn(str, delim);
    if (*str == '\0') {
        return NULL;
    }
    result = str;
    str += strcspn(str, delim);
    if (*str) {
        // Null-terminate the current token
        *str++ = '\0';
    }
    // Save the current token position to key
    *key = str;
    return result;
}

// Calculate width of an ASCII character
// Arguments:
// - input: character
// Returns:
// - character width in screen space
int char_width(char input) {
    static const int lookup[128] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        4, 2, 4, 7, 6, 9, 7, 2, 3, 3, 4, 6, 3, 5, 2, 7,
        6, 3, 6, 6, 6, 6, 6, 6, 6, 6, 2, 3, 5, 6, 5, 7,
        8, 6, 6, 6, 6, 6, 6, 6, 6, 4, 6, 6, 5, 8, 8, 6,
        6, 7, 6, 6, 6, 6, 8,10, 8, 6, 6, 3, 6, 3, 6, 6,
        4, 7, 6, 6, 6, 6, 5, 6, 6, 2, 5, 5, 2, 9, 6, 6,
        6, 6, 6, 6, 5, 6, 6, 6, 6, 6, 6, 4, 2, 5, 7, 0
    };
    return lookup[input];
}


// Calculate width of a text string in screen space
// Arguments:
// - input: null-terminated string to measure
// Returns:
// - total string width in screen space
int string_width(const char *input) {
    int result = 0;
    int length = strlen(input);
    for (int i = 0; i < length; i++) {
        result += char_width(input[i]);
    }
    return result;
}

// Wrap text using the maximum line width
// Arguments:
// - input: input text to wrap
// - max_width: maximum string line width, in screen space
// - output: output string buffer
// - max_length: maximum length to write to the string buffer
// Returns:
// - output: character buffer is written to
// - the number of lines that the output text uses.
int wrap(const char *input, int max_width, char *output, int max_length) {
    // Always terminate the output string, in case there are no tokens
    *output = '\0';
    // Create a copy of the input string because tokenization modifies the given
    // string in order to add null-termination to the end of tokens.
    char *text = malloc(sizeof(char) * (strlen(input) + 1));
    strcpy(text, input);
    int space_width = char_width(' ');
    // Count the number of lines created
    int line_number = 0;
    char *key1, *key2;
    // Begin tokenizing each line by line breaks
    char *line = tokenize(text, "\r\n", &key1);
    while (line) {
        // Keep track of the current line width in screen space
        int line_width = 0;
        // Begin tokenizing each word by spaces
        char *token = tokenize(line, " ", &key2);
        while (token) {
            int token_width = string_width(token);
            if (line_width) {
                if (line_width + token_width > max_width) {
                    // If the width goes over the max line width, create a new
                    // line.
                    line_width = 0;
                    line_number++;
                    strncat(output, "\n", max_length - strlen(output) - 1);
                }
                else {
                    strncat(output, " ", max_length - strlen(output) - 1);
                }
            }
            strncat(output, token, max_length - strlen(output) - 1);
            line_width += token_width + space_width;
            // Next word token
            token = tokenize(NULL, " ", &key2);
        }
        line_number++;
        strncat(output, "\n", max_length - strlen(output) - 1);
        // Next line token
        line = tokenize(NULL, "\r\n", &key1);
    }
    // Free the copy of the input text
    free(text);
    return line_number;
}


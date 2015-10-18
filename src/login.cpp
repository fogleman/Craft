#include<nanogui/nanogui.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "konstructs.h"
#include "login.h"

void render_fullscreen_image(Attrib *attrib) {

    GLuint vao, vbo;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    GLfloat vertex_data[] = {
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f
    };

    // Load vertex_data
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

    glUseProgram(attrib->program);
    glUniform1i(attrib->sampler, 4);

    glEnableVertexAttribArray(attrib->position);
    glVertexAttribPointer(attrib->position, 2, GL_FLOAT, GL_FALSE,
                          4*sizeof(GLfloat),
                          0);

    glEnableVertexAttribArray(attrib->uv);
    glVertexAttribPointer(attrib->uv, 2, GL_FLOAT, GL_FALSE,
                          4*sizeof(GLfloat),
                          (void*)(2*sizeof(float)));

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

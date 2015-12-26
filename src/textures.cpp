#include <nanogui/opengl.h>
#include "textures.h"
#include "util.h"
#define KONSTRUCTS_PATH_SIZE 256

namespace konstructs {
    void shtxt_path(const char *name, const char *type, char *path, size_t max_len) {
        snprintf(path, max_len, "%s/%s", type, name);

        if (!file_exist(path)) {
            snprintf(path, max_len, "/usr/local/share/konstructs-client/%s/%s", type, name);
        }

        if (!file_exist(path)) {
            printf("Error, no %s for %s found.\n", type, name);
            exit(1);
        }
    }

    void texture_path(const char *name, char *path, size_t max_len) {
        shtxt_path(name, "textures", path, max_len);
    }


    void load_textures() {
        char txtpth[KONSTRUCTS_PATH_SIZE];

        GLuint sky;
        glGenTextures(1, &sky);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, sky);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        texture_path("sky.png", txtpth, KONSTRUCTS_PATH_SIZE);
        load_png_texture(txtpth);

        GLuint font;
        glGenTextures(1, &font);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, font);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        texture_path("font.png", txtpth, KONSTRUCTS_PATH_SIZE);
        load_png_texture(txtpth);

        GLuint inventory_texture;
        glGenTextures(1, &inventory_texture);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, inventory_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        texture_path("inventory.png", txtpth, KONSTRUCTS_PATH_SIZE);
        load_png_texture(txtpth);
    }
};

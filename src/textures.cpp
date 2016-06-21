#include <fstream>
#include <streambuf>
#include <nanogui/opengl.h>
#include "textures.h"
#include "util.h"
#define KONSTRUCTS_PATH_SIZE 256

namespace konstructs {
    void shtxt_path(const char *name, const char *type, char *path, size_t max_len) {
        snprintf(path, max_len, "%s/%s", type, name);

        if (!file_exist(path)) {
            snprintf(path, max_len, "../%s/%s", type, name);
        }

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

    void model_path(const char *name, char *path, size_t max_len) {
        shtxt_path(name, "models", path, max_len);
    }

    void shader_path(const char *name, char *path, size_t max_len) {
        shtxt_path(name, "shaders", path, max_len);
    }

    void load_textures() {
        char txtpth[KONSTRUCTS_PATH_SIZE];

        GLuint sky;
        glGenTextures(1, &sky);
        glActiveTexture(GL_TEXTURE0 + SKY_TEXTURE);
        glBindTexture(GL_TEXTURE_2D, sky);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        texture_path("sky.png", txtpth, KONSTRUCTS_PATH_SIZE);
        load_png_texture(txtpth);

        GLuint font;
        glGenTextures(1, &font);
        glActiveTexture(GL_TEXTURE0 + FONT_TEXTURE);
        glBindTexture(GL_TEXTURE_2D, font);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        texture_path("font.png", txtpth, KONSTRUCTS_PATH_SIZE);
        load_png_texture(txtpth);

        GLuint inventory_texture;
        glGenTextures(1, &inventory_texture);
        glActiveTexture(GL_TEXTURE0 + INVENTORY_TEXTURE);
        glBindTexture(GL_TEXTURE_2D, inventory_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        texture_path("inventory.png", txtpth, KONSTRUCTS_PATH_SIZE);
        load_png_texture(txtpth);

        GLuint player_texture;
        glGenTextures(1, &player_texture);
        glActiveTexture(GL_TEXTURE0 + PLAYER_TEXTURE);
        glBindTexture(GL_TEXTURE_2D, player_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        texture_path("player.png", txtpth, KONSTRUCTS_PATH_SIZE);
        load_png_texture(txtpth);

        GLuint damage_texture;
        glGenTextures(1, &damage_texture);
        glActiveTexture(GL_TEXTURE0 + DAMAGE_TEXTURE);
        glBindTexture(GL_TEXTURE_2D, damage_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        texture_path("damage.png", txtpth, KONSTRUCTS_PATH_SIZE);
        load_png_texture(txtpth);

    }

    tinyobj::shape_t load_player() {
        char objpth[KONSTRUCTS_PATH_SIZE];
        model_path("player.obj", objpth, KONSTRUCTS_PATH_SIZE);

        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::string err;
        tinyobj::LoadObj(shapes, materials, err, objpth);

        return shapes[0];
    }

    std::string load_shader(const char* name) {
        char shader_pth[KONSTRUCTS_PATH_SIZE];
        shader_path(name, shader_pth, KONSTRUCTS_PATH_SIZE);
        std::ifstream shader(shader_pth);
        std::string shader_str((std::istreambuf_iterator<char>(shader)),
                               std::istreambuf_iterator<char>());
        return shader_str;
    }

    std::string load_chunk_vertex_shader() {
        return load_shader("chunk.vert");
    }

    std::string load_chunk_fragment_shader() {
        return load_shader("chunk.frag");
    }
};

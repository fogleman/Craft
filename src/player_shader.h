#ifndef __PLAYER_SHADER_H__
#define __PLAYER_SHADER_H__

#include <memory>
#include <unordered_map>
#include "shader.h"
#include "player.h"
#include "player.h"
#include "chunk_factory.h"
#include "matrix.h"

namespace konstructs {
    class PlayerShader : public ShaderProgram {
    public:
        PlayerShader(const float fov, const GLuint player_texture,
                     const GLuint sky_texture, const float near_distance,
                     tinyobj::shape_t &shape);
        void add(const Player player);
        void remove(const int pid);
        int render(const Player &p, const int width, const int height,
                   const float current_daylight, const float current_timer, const int radius);
        const GLuint position_attr;
        const GLuint normal_attr;
        const GLuint uv_attr;
        const GLuint matrix;
        const GLuint translation;
        const GLuint sampler;
        const GLuint sky_sampler;
        const GLuint fog_distance;
        const GLuint timer;
        const GLuint daylight;
        const GLuint camera;
        const GLuint player_texture;
        const GLuint sky_texture;
        const float near_distance;
    private:
        std::unordered_map<int, Player> players;
        ShapeModel model;
        const float fov;
    };
};

#endif

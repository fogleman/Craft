#ifndef __HUDSHADER_H__
#define __HUDSHADER_H__
#include <unordered_map>
#include <Eigen/Geometry>
#include "optional.hpp"
#include "shader.h"
#include "item.h"

namespace konstructs {
    using Eigen::Vector2i;
    using nonstd::optional;
    using nonstd::nullopt;

    class ItemStackModel : public BufferModel {
    public:
        ItemStackModel(const GLuint position_attr, const GLuint uv_attr,
                       const std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> &stacks,
                       const int blocks[256][6]);
        virtual void bind();
        virtual int vertices();
    private:
        const GLuint position_attr;
        const GLuint uv_attr;
        int verts;
    };

    class AmountModel : public BufferModel {
    public:
        AmountModel(const GLuint position_attr, const GLuint uv_attr,
                    const std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> &stacks);
        virtual void bind();
        virtual int vertices();
    private:
        const GLuint position_attr;
        const GLuint uv_attr;
        int verts;
    };

    class HudModel : public BufferModel {
    public:
        HudModel(const std::unordered_map<Vector2i, int, matrix_hash<Vector2i>> &background,
                 const GLuint position_attr, const GLuint uv_attr);
        virtual void bind();
        virtual int vertices();
    private:
        const GLuint position_attr;
        const GLuint uv_attr;
        int verts;
    };

    class HudShader: private ShaderProgram {
    public:
        HudShader(const int columns, const int rows, const int texture,
                  const int block_texture, const int font_texture);
        optional<Vector2i> clicked_at(const double x, const double y,
                                      const int width, const int height);
        void render(const int width, const int height,
                    const Hud &hud,
                    const int blocks[256][6]);

    private:
        const GLuint position;
        const GLuint matrix;
        const GLuint offset;
        const GLuint sampler;
        const GLuint uv;
        const int texture;
        const int block_texture;
        const int font_texture;
        const int columns;
        const int rows;
    };
};
#endif

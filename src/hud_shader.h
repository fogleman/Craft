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

    class BaseModel : public BufferModel {
    public:
        BaseModel(const GLuint position_attr, const GLuint normal_attr,
                  const GLuint uv_attr);
        virtual void bind();
    private:
        const GLuint position_attr;
        const GLuint normal_attr;
        const GLuint uv_attr;
    };

    class ItemStackModel : public BaseModel {
    public:
        ItemStackModel(const GLuint position_attr, const GLuint normal_attr,
                       const GLuint uv_attr,
                       const std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> &stacks,
                       const int blocks[256][6]);
        virtual int vertices();
    private:
        int verts;
    };

    class AmountModel : public BaseModel {
    public:
        AmountModel(const GLuint position_attr, const GLuint normal_attr,
                    const GLuint uv_attr,
                    const std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> &stacks);
        virtual int vertices();
    private:
        int verts;
    };

    class HudModel : public BaseModel {
    public:
        HudModel(const std::unordered_map<Vector2i, int, matrix_hash<Vector2i>> &background,
                 const GLuint position_attr, const GLuint normal_attr,
                 const GLuint uv_attr);
        virtual int vertices();
    private:
        int verts;
    };

    class BlockModel : public BaseModel {
    public:
        BlockModel(const GLuint position_attr, const GLuint normal_attr,
                   const GLuint uv_attr,
                   const int type, const float x, const float y,
                   const float size,
                   const int blocks[256][6]);
        virtual int vertices();
    private:
        int verts;
    };

    class HudShader: private ShaderProgram {
    public:
        HudShader(const int columns, const int rows, const int texture,
                  const int block_texture, const int font_texture);
        optional<Vector2i> clicked_at(const double x, const double y,
                                      const int width, const int height);
        void render(const int width, const int height,
                    const float mouse_x, const float mouse_y,
                    const Hud &hud,
                    const int blocks[256][6]);

    private:
        const GLuint position;
        const GLuint normal;
        const GLuint uv;
        const GLuint matrix;
        const GLuint offset;
        const GLuint sampler;
        const int texture;
        const int block_texture;
        const int font_texture;
        const int columns;
        const int rows;
        const float screen_area;
    };
};
#endif

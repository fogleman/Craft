#ifndef __HUDSHADER_H__
#define __HUDSHADER_H__
#include <unordered_map>
#include <Eigen/Geometry>
#include "optional.hpp"
#include "shader.h"
#include "item.h"
#include "block.h"

namespace konstructs {
    using Eigen::Vector2i;
    using nonstd::optional;
    using nonstd::nullopt;

    class BaseModel : public BufferModel {
    public:
        BaseModel(const GLuint position_attr, const GLuint normal_attr,
                  const GLuint uv_attr);
        virtual void bind();
        virtual int vertices();
    protected:
        int verts;
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
                       const BlockTypeInfo &blocks);
    };

    class HealthBarModel : public BaseModel {
    public:
        HealthBarModel(const std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> &stacks,
                       const GLuint position_attr, const GLuint normal_attr,
                       const GLuint uv_attr);
    };

    class AmountModel : public BaseModel {
    public:
        AmountModel(const GLuint position_attr, const GLuint normal_attr,
                    const GLuint uv_attr,
                    const std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> &stacks);
    };

    class HudModel : public BaseModel {
    public:
        HudModel(const std::unordered_map<Vector2i, int, matrix_hash<Vector2i>> &background,
                 const GLuint position_attr, const GLuint normal_attr,
                 const GLuint uv_attr);
    };

    class BlockModel : public BaseModel {
    public:
        BlockModel(const GLuint position_attr, const GLuint normal_attr,
                   const GLuint uv_attr,
                   const int type, const float x, const float y,
                   const float size,
                   const BlockTypeInfo &blocks);
    };

    class HudShader: private ShaderProgram {
    public:
        HudShader(const int columns, const int rows, const GLuint texture,
                  const GLuint block_texture, const GLuint font_texture,
                  const GLuint health_bar_texture);
        optional<Vector2i> clicked_at(const double x, const double y,
                                      const int width, const int height);
        void render(const int width, const int height,
                    const float mouse_x, const float mouse_y,
                    const Hud &hud,
                    const BlockTypeInfo &blocks);

    private:
        const GLuint position;
        const GLuint normal;
        const GLuint uv;
        const GLuint matrix;
        const GLuint offset;
        const GLuint sampler;
        const GLuint texture;
        const GLuint block_texture;
        const GLuint font_texture;
        const GLuint health_bar_texture;
        const int columns;
        const int rows;
        const float screen_area;
    };
};
#endif

#ifndef __ITEM_SHADER_H__
#define __ITEM_SHADER_H__
#include <unordered_map>
#include "shader.h"
#include "item.h"

namespace konstructs {

    class ItemStackModel : public BufferModel {
    public:
        ItemStackModel(const GLuint position_attr, const GLuint uv_attr,
                       const int columns, const int rows,
                       const std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> &stacks,
                       const int blocks[256][6]);
        virtual void bind();
        virtual int vertices();
    private:
        const GLuint position_attr;
        const GLuint uv_attr;
        int verts;
    };

    class ItemShader : private ShaderProgram {
    public:
        ItemShader(const int columns, const int rows, const int texture);
        void render(const int width, const int height,
                    const std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> &stacks,
                    const int blocks[256][6]);
    private:
        const GLuint position;
        const GLuint scale;
        const GLuint xscale;
        const GLuint sampler;
        const GLuint uv;
        const int texture;
        const int columns;
        const int rows;
    };

};

#endif

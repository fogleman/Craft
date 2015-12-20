#ifndef __HUD_H__
#define __HUD_H__

#include <vector>
#include <unordered_map>
#include "matrix.h"
#include "item.h"

namespace konstructs {

    class Hud {
    public:
        Hud(const int columns, const int rows);
        void set_background(const int row, const int column, const int t);
        void set_stack(const Vector2i pos, const ItemStack stack);
        std::vector<int> backgrounds();
        std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> stacks();
        const int rows;
        const int columns;
    private:
        std::vector<int> bg;
        std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> item_stacks;
    };

};

#endif

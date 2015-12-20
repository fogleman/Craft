#include "hud.h"

namespace konstructs {
    Hud::Hud(const int columns, const int rows):
        rows(rows),
        columns(columns),
        bg(rows*columns, -1) {}
    void Hud::set_background(const int row, const int column, const int t) {
        bg[rows * row + column] = t;
    }
    std::vector<int> Hud::backgrounds() {
        return bg;
    }
    void Hud::set_stack(const Vector2i pos, const ItemStack stack) {
        item_stacks.insert({pos, stack});
    }
    std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> Hud::stacks() {
        return item_stacks;
    }
};

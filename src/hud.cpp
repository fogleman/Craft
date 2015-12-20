#include "hud.h"

namespace konstructs {
    Hud::Hud(const int columns, const int rows):
        rows(rows),
        columns(columns) {}
    void Hud::set_background(const Vector2i pos, const int t) {
        bg.insert({pos, t});
    }
    std::unordered_map<Vector2i, int, matrix_hash<Vector2i>> Hud::backgrounds() {
        return bg;
    }
    void Hud::set_stack(const Vector2i pos, const ItemStack stack) {
        item_stacks.insert({pos, stack});
    }
    std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> Hud::stacks() {
        return item_stacks;
    }
};

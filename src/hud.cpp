#include "hud.h"

namespace konstructs {
    Hud::Hud(const int columns, const int rows):
        rows(rows),
        columns(columns) {}
    void Hud::set_background(const Vector2i pos, const int t) {
        bg.erase(pos);
        bg.insert({pos, t});
    }
    void Hud::reset_background(const Vector2i pos) {
        bg.erase(pos);
    }
    bool Hud::active(const Vector2i pos) const {
        return bg.find(pos) != bg.end() || item_stacks.find(pos) != item_stacks.end();
    }
    std::unordered_map<Vector2i, int, matrix_hash<Vector2i>> Hud::backgrounds() const {
        return bg;
    }
    void Hud::set_stack(const Vector2i pos, const ItemStack stack) {
        item_stacks.erase(pos);
        item_stacks.insert({pos, stack});
    }
    void Hud::reset_stack(const Vector2i pos) {
        item_stacks.erase(pos);
    }
    std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> Hud::stacks() const {
        return item_stacks;
    }
};

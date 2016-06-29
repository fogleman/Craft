#include "hud.h"

namespace konstructs {
    using nonstd::nullopt;
    Hud::Hud(const int columns, const int rows, const int belt_size):
        rows(rows),
        columns(columns),
        belt_size(belt_size),
        held_stack(nullopt),
        selection(0),
        interactive(false) {
        for(int i = 0; i < belt_size; i++) {
            belt.push_back(nullopt);
        }
    }
    optional<ItemStack> Hud::held() const {
        return held_stack;
    }
    void Hud::set_held(ItemStack stack) {
        held_stack = optional<ItemStack>(stack);
    }
    void Hud::reset_held() {
        held_stack = nullopt;
    }
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
        if(!interactive) {
            std::unordered_map<Vector2i, int, matrix_hash<Vector2i>> new_bg(bg);
            for(int i = 0; i < belt_size; i++) {
                Vector2i pos((columns - belt_size) / 2 + i, 0);
                new_bg.erase(pos);
                if(i == selection)
                    new_bg.insert({pos, 3});
                else
                    new_bg.insert({pos, 2});
            }
            return new_bg;
        } else {
            return bg;
        }
    }
    void Hud::set_stack(const Vector2i pos, const ItemStack stack) {
        item_stacks.erase(pos);
        item_stacks.insert({pos, stack});
    }
    void Hud::reset_stack(const Vector2i pos) {
        item_stacks.erase(pos);
    }
    std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> Hud::stacks() const {
        if(!interactive) {
            std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>>
                new_item_stacks(item_stacks);
            for(int i = 0; i < belt_size; i++) {
                auto stack = belt[i];
                Vector2i pos((columns - belt_size) / 2 + i, 0);
                new_item_stacks.erase(pos);
                if(stack) {
                    new_item_stacks.insert({pos, *stack});
                }
            }
            return new_item_stacks;
        } else {
            return item_stacks;
        }
    }
    void Hud::set_belt(const int pos, ItemStack stack) {
        belt[pos] = stack;
    }
    void Hud::reset_belt(const int pos) {
        belt[pos] = nullopt;
    }
    optional<ItemStack> Hud::selected() const {
        return belt[selection];
    }
    int Hud::scroll(int direction) {
        if (selection <= 0 && direction > 0) return selection;
        if (selection >= (belt_size - 1) && direction < 0) return selection;
        selection -= direction;
        return selection;
    }
    void Hud::set_selected(int s) {
        selection = s;
        for(int i = 0; i < 9; i++) {
            Vector2i pos(i + 4, 0);
            if(i == selection) {
                set_background(pos, 3);
            } else {
                set_background(pos, 2);
            }
        }

    }
    int Hud::get_selection() const {
        return selection;
    }
    void Hud::set_interactive(bool i) {
        interactive = i;
    }
    bool Hud::get_interactive() const {
        return interactive;
    }
};

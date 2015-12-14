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
};

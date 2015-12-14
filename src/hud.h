#ifndef __HUD_H__
#define __HUD_H__

#include <vector>

namespace konstructs {

    class Hud {
    public:
        Hud(const int columns, const int rows);
        void set_background(const int row, const int column, const int t);
        std::vector<int> backgrounds();
        const int rows;
        const int columns;
    private:
        std::vector<int> bg;
    };

};

#endif

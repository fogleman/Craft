#include "item.h"
#include "util.h"

const int items[] = {
    // items the user can build
    Item_GRASS,
    Item_SAND,
    Item_STONEBRICK,
    Item_BRICK,
    Item_WOOD,
    Item_STONE,
    Item_DIRT,
    Item_PLANK,
    Item_SNOW,
    Item_GLASS,
    Item_COBBLE,
    Item_LIGHT_STONE,
    Item_DARK_STONE,
    Item_CHEST,
    Item_LEAVES,
    Item_CACTUS,
    Item_NYANCAT,
    Item_SLAB_LOWER_STONEBRICK,
    Item_TALL_GRASS,
    Item_YELLOW_FLOWER,
    Item_RED_FLOWER,
    Item_PURPLE_FLOWER,
    Item_SUN_FLOWER,
    Item_WHITE_FLOWER,
    Item_BLUE_FLOWER,
    Item_VINE,
    Item_COLOR_00,
    Item_COLOR_01,
    Item_COLOR_02,
    Item_COLOR_03,
    Item_COLOR_04,
    Item_COLOR_05,
    Item_COLOR_06,
    Item_COLOR_07,
    Item_COLOR_08,
    Item_COLOR_09,
    Item_COLOR_10,
    Item_COLOR_11,
    Item_COLOR_12,
    Item_COLOR_13,
    Item_COLOR_14,
    Item_COLOR_15,
    Item_COLOR_16,
    Item_COLOR_17,
    Item_COLOR_18,
    Item_COLOR_19,
    Item_COLOR_20,
    Item_COLOR_21,
    Item_COLOR_22,
    Item_COLOR_23,
    Item_COLOR_24,
    Item_COLOR_25,
    Item_COLOR_26,
    Item_COLOR_27,
    Item_COLOR_28,
    Item_COLOR_29,
    Item_COLOR_30,
    Item_COLOR_31
};

const int item_count = sizeof(items) / sizeof(int);

const int blocks[256][6] = {
    // w => (left, right, top, bottom, front, back) tiles
    {0, 0, 0, 0, 0, 0}, // 0 - empty
    {16, 16, 32, 0, 16, 16}, // 1 - grass
    {1, 1, 1, 1, 1, 1}, // 2 - sand
    {2, 2, 2, 2, 2, 2}, // 3 - stonebrick
    {3, 3, 3, 3, 3, 3}, // 4 - brick
    {20, 20, 36, 4, 20, 20}, // 5 - wood
    {5, 5, 5, 5, 5, 5}, // 6 - stone
    {6, 6, 6, 6, 6, 6}, // 7 - dirt
    {7, 7, 7, 7, 7, 7}, // 8 - plank
    {24, 24, 40, 8, 24, 24}, // 9 - snow
    {9, 9, 9, 9, 9, 9}, // 10 - glass
    {10, 10, 10, 10, 10, 10}, // 11 - cobble
    {11, 11, 11, 11, 11, 11}, // 12 - light stone
    {12, 12, 12, 12, 12, 12}, // 13 - dark stone
    {13, 13, 13, 13, 13, 13}, // 14 - chest
    {14, 14, 14, 14, 14, 14}, // 15 - leaves
    {15, 15, 15, 15, 15, 15}, // 16 - cloud
    {0, 0, 0, 0, 0, 0}, // 17
    {0, 0, 0, 0, 0, 0}, // 18
    {0, 0, 0, 0, 0, 0}, // 19
    {0, 0, 0, 0, 0, 0}, // 20
    {0, 0, 0, 0, 0, 0}, // 21
    {0, 0, 0, 0, 0, 0}, // 22
    {0, 0, 0, 0, 0, 0}, // 23
    {0, 0, 0, 0, 0, 0}, // 24
    {0, 0, 0, 0, 0, 0}, // 25
    {0, 0, 0, 0, 0, 0}, // 26
    {0, 0, 0, 0, 0, 0}, // 27
    {0, 0, 0, 0, 0, 0}, // 28
    {0, 0, 0, 0, 0, 0}, // 29
    {0, 0, 0, 0, 0, 0}, // 30
    {0, 0, 0, 0, 0, 0}, // 31
    {176, 176, 176, 176, 176, 176}, // 32
    {177, 177, 177, 177, 177, 177}, // 33
    {178, 178, 178, 178, 178, 178}, // 34
    {179, 179, 179, 179, 179, 179}, // 35
    {180, 180, 180, 180, 180, 180}, // 36
    {181, 181, 181, 181, 181, 181}, // 37
    {182, 182, 182, 182, 182, 182}, // 38
    {183, 183, 183, 183, 183, 183}, // 39
    {184, 184, 184, 184, 184, 184}, // 40
    {185, 185, 185, 185, 185, 185}, // 41
    {186, 186, 186, 186, 186, 186}, // 42
    {187, 187, 187, 187, 187, 187}, // 43
    {188, 188, 188, 188, 188, 188}, // 44
    {189, 189, 189, 189, 189, 189}, // 45
    {190, 190, 190, 190, 190, 190}, // 46
    {191, 191, 191, 191, 191, 191}, // 47
    {192, 192, 192, 192, 192, 192}, // 48
    {193, 193, 193, 193, 193, 193}, // 49
    {194, 194, 194, 194, 194, 194}, // 50
    {195, 195, 195, 195, 195, 195}, // 51
    {196, 196, 196, 196, 196, 196}, // 52
    {197, 197, 197, 197, 197, 197}, // 53
    {198, 198, 198, 198, 198, 198}, // 54
    {199, 199, 199, 199, 199, 199}, // 55
    {200, 200, 200, 200, 200, 200}, // 56
    {201, 201, 201, 201, 201, 201}, // 57
    {202, 202, 202, 202, 202, 202}, // 58
    {203, 203, 203, 203, 203, 203}, // 59
    {204, 204, 204, 204, 204, 204}, // 60
    {205, 205, 205, 205, 205, 205}, // 61
    {206, 206, 206, 206, 206, 206}, // 62
    {207, 207, 207, 207, 207, 207}, // 63
    {208, 208, 208, 208, 208, 208}, // 64
    {210, 210, 210, 210, 210, 210}, // 65
    {0, 0, 0, 0, 0, 0}, // 66
    {2, 2, 2, 2, 2, 2}
};

const int plants[256] = {
    // w => tile
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0 - 16
    48, // 17 - tall grass
    49, // 18 - yellow flower
    50, // 19 - red flower
    51, // 20 - purple flower
    52, // 21 - sun flower
    53, // 22 - white flower
    54, // 23 - blue flower
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, // 24 - 65
    55, // 66 - vine
    0 // 67
};

int is_plant(int w) {
    switch (w) {
        case Item_TALL_GRASS:
        case Item_YELLOW_FLOWER:
        case Item_RED_FLOWER:
        case Item_PURPLE_FLOWER:
        case Item_SUN_FLOWER:
        case Item_WHITE_FLOWER:
        case Item_BLUE_FLOWER:
        case Item_VINE:
            return 1;
        default:
            return 0;
    }
}

int is_obstacle(int w) {
    w = ABS(w);
    if (is_plant(w)) {
        switch(w) {
            default:
                return 0;
        }
    }
    switch (w) {
        case Item_EMPTY:
        case Item_CLOUD:
            return 0;
        default:
            return 1;
    }
}

int is_transparent(int w) {
    if (w == Item_EMPTY) {
        return 1;
    }
    w = ABS(w);
    if (is_plant(w)) {
        return 1;
    }
    if(is_noncube(w)) {
        return 1;
    }
    switch (w) {
        case Item_EMPTY:
        case Item_GLASS:
        //case LEAVES:
            return 1;
        default:
            return 0;
    }
}

int is_destructable(int w) {
    switch (w) {
        case Item_EMPTY:
        case Item_CLOUD:
            return 0;
        default:
            return 1;
    }
}

int is_climbable(int w) {
    switch(w) {
        case Item_VINE:
            return 1;
        default:
            return 0;
    }
}

int is_noncube(int w) {
    return noncube_type(w) != NonCubeType_NOT_NONCUBE;
}

NonCubeType noncube_type(int w) {
    switch(w) {
        case Item_SLAB_LOWER_STONEBRICK:
            return NonCubeType_SLAB_LOWER;
        default:
            return NonCubeType_NOT_NONCUBE;
    }
}

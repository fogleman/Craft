#include "item.h"
#include "util.h"

const int items[] = {
    // items the user can build
    GRASS,
    SAND,
    STONE,
    BRICK,
    WOOD,
    CEMENT,
    DIRT,
    PLANK,
    SNOW,
    GLASS,
    COBBLE,
    LIGHT_STONE,
    DARK_STONE,
    CHEST,
    LEAVES,
    TALL_GRASS,
    YELLOW_FLOWER,
    RED_FLOWER,
    PURPLE_FLOWER,
    SUN_FLOWER,
    WHITE_FLOWER,
    BLUE_FLOWER,
    // RED_WOOL,
    // GREEN_WOOL,
    // BLUE_WOOL,
    // ORANGE_WOOL,
    // PURPLE_WOOL,
    // BEIGE_WOOL
};

const int item_count = sizeof(items) / sizeof(int);

const int blocks[256][6] = {
    // w => (left, right, top, bottom, front, back) tiles
    {0, 0, 0, 0, 0, 0}, // 0 - empty
    {16, 16, 32, 0, 16, 16}, // 1 - grass
    {1, 1, 1, 1, 1, 1}, // 2 - sand
    {2, 2, 2, 2, 2, 2}, // 3 - stone
    {3, 3, 3, 3, 3, 3}, // 4 - brick
    {20, 20, 36, 4, 20, 20}, // 5 - wood
    {5, 5, 5, 5, 5, 5}, // 6 - cement
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
    {17, 17, 17, 17, 17, 17}, // 24 - red wool
    {18, 18, 18, 18, 18, 18}, // 25 - green wool
    {19, 19, 19, 19, 19, 19}, // 26 - blue wool
    {33, 33, 33, 33, 33, 33}, // 27 - orange wool
    {34, 34, 34, 34, 34, 34}, // 28 - purple wool
    {35, 35, 35, 35, 35, 35}, // 29 - beige wool
    {0, 0, 0, 0, 0, 0}, // 30
    {0, 0, 0, 0, 0, 0}, // 31
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
};

int is_plant(int w) {
    switch (w) {
        case TALL_GRASS:
        case YELLOW_FLOWER:
        case RED_FLOWER:
        case PURPLE_FLOWER:
        case SUN_FLOWER:
        case WHITE_FLOWER:
        case BLUE_FLOWER:
            return 1;
        default:
            return 0;
    }
}

int is_obstacle(int w) {
    w = ABS(w);
    switch (w) {
        case GRASS:
        case SAND:
        case STONE:
        case BRICK:
        case WOOD:
        case CEMENT:
        case DIRT:
        case PLANK:
        case SNOW:
        case GLASS:
        case COBBLE:
        case LIGHT_STONE:
        case DARK_STONE:
        case CHEST:
        case LEAVES:
        case RED_WOOL:
        case GREEN_WOOL:
        case BLUE_WOOL:
        case ORANGE_WOOL:
        case PURPLE_WOOL:
        case BEIGE_WOOL:
            return 1;
        default:
            return 0;
    }
}

int is_transparent(int w) {
    w = ABS(w);
    switch (w) {
        case EMPTY:
        case GLASS:
        case LEAVES:
        case TALL_GRASS:
        case YELLOW_FLOWER:
        case RED_FLOWER:
        case PURPLE_FLOWER:
        case SUN_FLOWER:
        case WHITE_FLOWER:
        case BLUE_FLOWER:
            return 1;
        default:
            return 0;
    }
}

int is_destructable(int w) {
    switch (w) {
        case GRASS:
        case SAND:
        case STONE:
        case BRICK:
        case WOOD:
        case CEMENT:
        case DIRT:
        case PLANK:
        case SNOW:
        case GLASS:
        case COBBLE:
        case LIGHT_STONE:
        case DARK_STONE:
        case CHEST:
        case LEAVES:
        case TALL_GRASS:
        case YELLOW_FLOWER:
        case RED_FLOWER:
        case PURPLE_FLOWER:
        case SUN_FLOWER:
        case WHITE_FLOWER:
        case BLUE_FLOWER:
        case RED_WOOL:
        case GREEN_WOOL:
        case BLUE_WOOL:
        case ORANGE_WOOL:
        case PURPLE_WOOL:
        case BEIGE_WOOL:
            return 1;
        default:
            return 0;
    }
}

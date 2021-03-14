#ifndef _item_h_
#define _item_h_

#define EMPTY 0
#define GRASS 1
#define SAND 2
#define STONE 3
#define BRICK 4
#define WOOD 5
#define CEMENT 6
#define DIRT 7
#define PLANK 8
#define SNOW 9
#define GLASS 10
#define COBBLE 11
#define LIGHT_STONE 12
#define DARK_STONE 13
#define CHEST 14
#define LEAVES 15
#define CLOUD 16
#define TALL_GRASS 17
#define YELLOW_FLOWER 18
#define RED_FLOWER 19
#define PURPLE_FLOWER 20
#define SUN_FLOWER 21
#define WHITE_FLOWER 22
#define BLUE_FLOWER 23
#define COLOR_00 32
#define COLOR_01 33
#define COLOR_02 34
#define COLOR_03 35
#define COLOR_04 36
#define COLOR_05 37
#define COLOR_06 38
#define COLOR_07 39
#define COLOR_08 40
#define COLOR_09 41
#define COLOR_10 42
#define COLOR_11 43
#define COLOR_12 44
#define COLOR_13 45
#define COLOR_14 46
#define COLOR_15 47
#define COLOR_16 48
#define COLOR_17 49
#define COLOR_18 50
#define COLOR_19 51
#define COLOR_20 52
#define COLOR_21 53
#define COLOR_22 54
#define COLOR_23 55
#define COLOR_24 56
#define COLOR_25 57
#define COLOR_26 58
#define COLOR_27 59
#define COLOR_28 60
#define COLOR_29 61
#define COLOR_30 62
#define COLOR_31 63

extern const int items[];
extern const int item_count;
extern const int blocks[256][6];
extern const int plants[256];

int is_plant(int w);
int is_obstacle(int w);
int is_transparent(int w);
int is_destructable(int w);

#endif

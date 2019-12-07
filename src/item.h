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
#define YELLOW 32
#define LIGHT_GREEN 33
#define BRIGHT_GREEN 34
#define TEAL 35
#define FOREST_GREEN 36
#define DARK_BROWN 37
#define BLUE_BLACK 38
#define NAVY_BLUE 39
#define LIGHT_GRAY 40
#define DARK_GRAY 41
#define PURPLE 42
#define RED 43
#define CORAL 44
#define PINK 45
#define OLIVE_GREEN 46
#define LIGHT_BROWN 47
#define SILVER 48
#define BLUE_GRAY 49
#define WHITE 50
#define ICE_BLUE 51
#define TURQUOISE 52
#define SKY_BLUE 53
#define BRIGHT_BLUE 54
#define DEEP_BLUE 55
#define BEIGE 56
#define TAN 57
#define ORANGE 58
#define LIGHT_BROWN 59
#define DEEP_BROWN 60
#define PLUM 61
#define INDIGO 62
#define BLACK 63

extern const int items[];
extern const int item_count;
extern const int blocks[256][6];
extern const int plants[256];
extern const char* item_names[];

int is_plant(int w);
int is_obstacle(int w);
int is_transparent(int w);
int is_destructable(int w);

#endif

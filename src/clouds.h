#include <vector>
// extern "C" {
//     #include <math.h>
//     #include <stdio.h>
//     #include <stdlib.h>
//     #include <string.h>
//     #include <time.h>
//     #include "auth.h"
//     #include "client.h"
//     #include "config.h"
//     #include "cube.h"
//     #include "db.h"
//     #include "item.h"
//     #include "matrix.h"
//     #include "sign.h"
//     #include "util.h"
//     #include "world.h"
// }       //extern "C"

#include "map.h"

struct cloudPosition{
    int x;
    int y;
    int z;
};

//Worker Functions
extern "C"
std::vector<cloudPosition> setClouds(std::vector<cloudPosition>, int x, int y, int z);
extern "C"
void moveAllCloudsDown(Map *map, std::vector<cloudPosition> allCloudPositions, int);
extern "C"
void moveAllCloudsUp(Map *map, std::vector<cloudPosition> allCloudPositions, int);
extern "C"
void moveClouds(Map *map, std::vector<cloudPosition> clouds, int posChange, char xyz, int);

//Test Functions
extern "C"
int gotClouds(std::vector<cloudPosition>);
extern "C"
int isPositionChanged(std::vector<cloudPosition> startData, std::vector<cloudPosition> endData);
extern "C"
int isValidChar(char xyz);


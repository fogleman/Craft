#include <vector>
extern "C" {
    #include <math.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <time.h>
    #include "auth.h"
    #include "client.h"
    #include "config.h"
    #include "cube.h"
    #include "db.h"
    #include "item.h"
    #include "map.h"
    #include "matrix.h"
    #include "sign.h"
    #include "util.h"
    #include "world.h"
}       //extern "C"

struct cloudPosition{
    int x;
    int y;
    int z;
};

//Worker Functions
std::vector<cloudPosition> setClouds(std::vector<cloudPosition>, int x, int y, int z);
void moveAllCloudsDown(Map *map, std::vector<cloudPosition> allCloudPositions, int);
void moveAllCloudsUp(Map *map, std::vector<cloudPosition> allCloudPositions, int);
void moveClouds(Map *map, std::vector<cloudPosition> clouds, int posChange, char xyz, int);

//Test Functions
int gotClouds(std::vector<cloudPosition>);
int isPositionChanged(std::vector<cloudPosition> startData, std::vector<cloudPosition> endData);
int isValidChar(char xyz);


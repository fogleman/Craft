#include <vector>
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


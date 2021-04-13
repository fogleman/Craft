#include <vector>
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

struct cloudPosition{
    int x;
    int y;
    int z;
};


void moveAllCloudsDown(Map *map, std::vector<cloudPosition> allCloudPositions);
void moveAllCloudsUp(Map *map, std::vector<cloudPosition> allCloudPositions);
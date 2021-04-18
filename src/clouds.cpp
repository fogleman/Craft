#include "clouds.h"

//Worker Functions
extern "C"
std::vector<cloudPosition> setClouds(std::vector<cloudPosition> allClouds, int x, int y, int z)
{
    cloudPosition p;
    p.x =x;
    p.y = y;
    p.z = z;
    allClouds.push_back(p);

    return allClouds;
}

extern "C"
void moveAllCloudsDown(Map *map, std::vector<cloudPosition> allCloudPositions, int t)
{
    char xyz = 'y';
    int  posChange = -1;
    moveClouds(map,allCloudPositions, posChange, xyz, t);
}

extern "C"
void moveAllCloudsUp(Map *map, std::vector<cloudPosition> allCloudPositions, int t)
{
    char xyz = 'y';
    int  posChange = 1;
    moveClouds(map,allCloudPositions, posChange, xyz, t);
}

extern "C"
void moveClouds(Map *map, std::vector<cloudPosition> clouds, int posChange, char xyz, int t)
{
    std::vector<cloudPosition> cloudCopy = clouds;
    while (clouds.size() > 0)
    {
        cloudPosition p = clouds.back();
        map_set(map, p.x, p.y, p.z, 0, t);     //remove cloud at current position
        clouds.pop_back();
    }
    while (clouds.size() > 0)
    {
        cloudPosition p = cloudCopy.back();
        
        if(xyz == 'x'){ map_set(map, p.x + posChange, p.y, p.z, 16, t);}
        else if(xyz == 'y'){ map_set(map, p.x, p.y + posChange, p.z, 16, t);}
        else if(xyz == 'z'){ map_set(map, p.x, p.y, p.z + posChange, 16, t);}

        clouds.pop_back();
    }
    
    
}


//Test Functions
//extern "C"
//int gotClouds(std::vector<cloudPosition> clouds){ return 0; }
//extern "C"
//int isPositionChanged(std::vector<cloudPosition>startData, std::vector<cloudPosition> endData){ return 0; }
//extern "C"
//int isValidChar(char xyz){ return 0; }

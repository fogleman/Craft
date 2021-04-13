#include "clouds.h"

void moveAllCloudsDown(Map *map, std::vector<cloudPosition> allCloudPositions)
{
    std::vector<cloudPosition> cloudCopy = allCloudPositions;

    while( allCloudPositions.size() > 0 )
    {
        cloudPosition p = allCloudPositions.back();
        map_set(map, p.x, p.y, p.z, 0);                 //remove cloud
        allCloudPositions.pop_back();
    }
    while( allCloudPositions.size() > 0 )
    {
        cloudPosition p = cloudCopy.back();
        map_set(map, p.x, p.y-1, p.z, CLOUD);           //add cloud to space below
        cloudCopy.pop_back();
    }
}

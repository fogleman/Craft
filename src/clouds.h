#include <vector>
#include "map.h"

///
/// A way to manipulate the cloud blocks implemented in world.h
//////////////////////////////////////////////////////////////////////////////

struct cloudPosition{
    /// @struct cloudPosition
    /// @note The 3D position (x, y, z) of the cloud blocks is initially from 
    /// @param *map
    //////////////////////////////////////////////////////////////////////////
    int x;
    int y;
    int z;
};

//Worker Functions
extern "C"
std::vector<cloudPosition> setClouds(std::vector<cloudPosition>, int x, int y, int z);
extern "C"
std::vector<cloudPosition> getClouds();
extern "C"
std::vector<cloudPosition> moveAllCloudsDown(Map *map, std::vector<cloudPosition> allCloudPositions, int);
extern "C"
std::vector<cloudPosition> moveAllCloudsUp(Map *map, std::vector<cloudPosition> allCloudPositions, int);

/// \ref US_1_1_2_1
extern "C"
std::vector<cloudPosition> moveAllCloudsOver(Map *map, std::vector<cloudPosition> allCloudPositions, int);
extern "C"
std::vector<cloudPosition> moveClouds(Map *map, std::vector<cloudPosition> clouds, int posChange, int xyz, int);

//Test Functions
extern "C"
int gotClouds(std::vector<cloudPosition>);
extern "C"
int isPositionChanged(std::vector<cloudPosition> startData, std::vector<cloudPosition> endData);
extern "C"
int isValidChar(char xyz);

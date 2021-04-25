#include "clouds.h"

struct cloudData{
    private:
        std::vector<cloudPosition> privateCloud;
    public:
        void set(std::vector<cloudPosition> cloud){
            privateCloud = cloud;
        }
        std::vector<cloudPosition> get(){
            return privateCloud;
        }
} allCloudData;


//Worker Functions
extern "C"
std::vector<cloudPosition> setClouds(std::vector<cloudPosition> allClouds, int x, int y, int z)
{
    cloudPosition p;
    p.x =x;
    p.y = y;
    p.z = z;
    allClouds.push_back(p);

    //cloudData data;
    //allCloudData.set(allClouds);
    return allClouds;
}

extern "C"
std::vector<cloudPosition> getClouds(){
    cloudData data;
    std::vector<cloudPosition> clouds = allCloudData.get();
    printf("Get!\n");
    return clouds;
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
extern "C"
int gotClouds(std::vector<cloudPosition> clouds){ 
    if(clouds.size() == 0)
    {
        printf("Test FAILED: Clouds in Vector "); printf("%ld", clouds.size()); printf("\n");
        return -1;
    }
    else{
        printf("Test PASSED: Clouds in Vector "); printf("%ld", clouds.size()); printf("\n");
        return clouds.size();
    } 
}
extern "C"
int isPositionChanged(std::vector<cloudPosition>startData, std::vector<cloudPosition> endData){
    int test = 0;
    while( test < 1){
        if (startData.size() == endData.size()){
            for(int idx=0; idx<startData.size(); idx++){
                if(startData[1].x == endData[1].x){
                    if(startData[idx].y == endData[idx].y){
                        if(startData[idx].z == endData[idx].z){
                           /*Point is the same;*/ }
                        else{ test++; }}
                    else{ test++; }}
                else{ test++; }
            }
        }else {test++;}
    }

    if( test >= 1 )
    {
        printf("Test FAILED: US 2.2.4 Cloud Position Changed\n");
        return -1;
    } 
    else{
        printf("Test PASSED: US 2.2.4 Cloud Position Changed\n");
        return 0;
    }
}
extern "C"
int isValidChar(char xyz){
    if( xyz == 'x' || xyz == 'y' || xyz == 'z')
    {
        printf("Test PASSED: valid position char change\n");
        return 0;
    }
    else{
        printf("Test PASSED: valid position char change\n");
        return -1;
    }
}

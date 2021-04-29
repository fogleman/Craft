

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
std::vector<cloudPosition> moveAllCloudsDown(Map *map, std::vector<cloudPosition> allCloudPositions, int t)
{
    int xyz = 2;
    int  posChange = -1;
    allCloudPositions = moveClouds(map,allCloudPositions, posChange, xyz, t);
    return allCloudPositions;
}

extern "C"
std::vector<cloudPosition> moveAllCloudsUp(Map *map, std::vector<cloudPosition> allCloudPositions, int t)
{
    int xyz = 2;
    int  posChange = 1;
    allCloudPositions = moveClouds(map,allCloudPositions, posChange, xyz, t);
    return allCloudPositions;
}
  
extern "C"
std::vector<cloudPosition> moveAllCloudsOver(Map *map, std::vector<cloudPosition> allCloudPositions, int t)
{   
    int xyz = 1;
    int  posChange = 1;
    std::vector<cloudPosition> movedClouds = moveClouds(map,allCloudPositions, posChange, xyz, t);
    return movedClouds;
}

extern "C"
std::vector<cloudPosition> moveClouds(Map *map, std::vector<cloudPosition> clouds, int posChange, int xyz, int t)
{
    std::vector<cloudPosition> cloudCopy = clouds;
    std::vector<cloudPosition> returnClouds = clouds;
    while (clouds.size() > 0)
    {
        cloudPosition p = clouds.back();
        //map_set(map, p.x, p.y, p.z, 0, t);     //remove cloud at current position
        clouds.pop_back();
    }
    while (clouds.size() > 0)
    {
        cloudPosition p = cloudCopy.back();
        
        if(xyz == 1){ 
            //map_set(map, p.x + posChange, p.y, p.z, 16, t);
        }
        else if(xyz == 2){ 
            //map_set(map, p.x, p.y + posChange, p.z, 16, t);
        }
        else if(xyz == 3){ 
            //map_set(map, p.x, p.y, p.z + posChange, 16, t);     
        }
        clouds.pop_back();
    }
    if(xyz == 1){ 
        for(int i = 0; i < returnClouds.size(); i++){returnClouds[i].x = returnClouds[i].x + posChange;}    
    }
    else if(xyz == 2){ 
        for(int i = 0; i < returnClouds.size(); i++){returnClouds[i].y = returnClouds[i].y + posChange;}
    }
    else if(xyz == 3){ 
        for(int i = 0; i < returnClouds.size(); i++){returnClouds[i].z = returnClouds[i].z + posChange;}
    }

    return returnClouds;
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
    int done = 0;
    while( test < 1 && done == 0){
        if (startData.size() == endData.size()){
            for(int idx=0; idx<startData.size(); idx++){
                if(startData[idx].x == endData[idx].x){
                    if(startData[idx].y == endData[idx].y){
                        if(startData[idx].z == endData[idx].z){
                           /*Point is the same;*/ }
                        else{ test++; }}
                    else{ test++; }}
                else{ test++; }
            }
            done++;
        }else {test++;}
    }

    if( test >= 1 )
    {   
        /// @note Output supressed for testing function efficacy
        /// printf("Test FAILED: US 2.2.4 Cloud Position Changed\n");
        return -1;
    } 
    else{
        /// @note Output supressed for testing function efficacy
        /// printf("Test PASSED: US 2.2.4 Cloud Position Changed\n");
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
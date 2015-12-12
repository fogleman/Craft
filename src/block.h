#ifndef __BLOCK_H__
#define __BLOCK_H__

#include <Eigen/Geometry>

#define VOID_BLOCK 0
#define SOLID_BLOCK 100

namespace konstructs {

    using namespace Eigen;

    struct BlockData {
        int blocks[256][6];
        char is_plant[256];
        char is_obstacle[256];
        char is_transparent[256];
    };

    class Block {
    public:
        Block(const Vector3i _position, const char _type);
        Vector3i position;
        char type;
    };
};

#endif

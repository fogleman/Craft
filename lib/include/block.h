#ifndef __BLOCK_H__
#define __BLOCK_H__

#include <Eigen/Geometry>

#define VOID_BLOCK 0
#define SOLID_TYPE 65535
#define BLOCK_TYPES 65536
#define MAX_HEALTH 2047

#define STATE_SOLID 0
#define STATE_LIQUID 1
#define STATE_GAS 2
#define STATE_PLASMA 3

namespace konstructs {

    using namespace Eigen;

    struct BlockTypeInfo {
        int blocks[BLOCK_TYPES][6];
        char is_plant[BLOCK_TYPES];
        char is_obstacle[BLOCK_TYPES];
        char is_transparent[BLOCK_TYPES];
        char state[BLOCK_TYPES];
    };

    struct BlockData {
        uint16_t type;
        uint16_t health;
    };

    class Block {
    public:
        Block(const Vector3i position, const BlockData data);
        Vector3i position;
        BlockData data;
    };
};

#endif

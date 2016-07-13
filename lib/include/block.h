#ifndef __BLOCK_H__
#define __BLOCK_H__

#include <cstdint>

#include <Eigen/Geometry>

#define VOID_BLOCK 0
#define SOLID_TYPE 65535
#define BLOCK_TYPES 65536
#define MAX_HEALTH 2047

#define DIRECTION_UP 0
#define DIRECTION_DOWN 1
#define DIRECTION_RIGHT 2
#define DIRECTION_LEFT 3
#define DIRECTION_FORWARD 4
#define DIRECTION_BACKWARD 5

#define ROTATION_IDENTITY 0
#define ROTATION_LEFT 1
#define ROTATION_RIGHT 2
#define ROTATION_HALF 3

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
        uint8_t direction;
        uint8_t rotation;
    };

    class Block {
    public:
        Block(const Vector3i position, const BlockData data);
        Vector3i position;
        BlockData data;
    };

    const uint8_t direction_from_vector(const Vector3i &unit_vector);
    const uint8_t direction_from_vector(const Vector3i &from, const Vector3i &to);
    const uint8_t rotation_from_vector(const uint8_t direction, const Vector3f &vector);
};

#endif

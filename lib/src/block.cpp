#include <iostream>
#include <unordered_map>
#include "block.h"
#include "matrix.h"
namespace konstructs {
    Block::Block(const Vector3i position, const BlockData data):
        position(position), data(data) {}


    const std::unordered_map<const Vector3i, const uint8_t, matrix_hash<Vector3i>> vector_to_direction = {
        {Vector3i(0,1,0), DIRECTION_UP},
        {Vector3i(0,-1,0), DIRECTION_DOWN},
        {Vector3i(1,0,0), DIRECTION_RIGHT},
        {Vector3i(-1,0,0), DIRECTION_LEFT},
        {Vector3i(0,0,1), DIRECTION_BACKWARD},
        {Vector3i(0,0,-1), DIRECTION_FORWARD},

        // Edge cases (pointing on an edge or a corner)
        {Vector3i(1,1,0), DIRECTION_RIGHT},
        {Vector3i(1,-1,0), DIRECTION_RIGHT},
        {Vector3i(1,1,1), DIRECTION_RIGHT},
        {Vector3i(1,-1,1), DIRECTION_RIGHT},
        {Vector3i(1,1,-1), DIRECTION_RIGHT},
        {Vector3i(1,-1,-1), DIRECTION_RIGHT},
        {Vector3i(1,0,-1), DIRECTION_RIGHT},
        {Vector3i(1,0,1), DIRECTION_RIGHT},

        {Vector3i(-1,1,0), DIRECTION_LEFT},
        {Vector3i(-1,-1,0), DIRECTION_LEFT},
        {Vector3i(-1,1,1), DIRECTION_LEFT},
        {Vector3i(-1,-1,1), DIRECTION_LEFT},
        {Vector3i(-1,1,-1), DIRECTION_LEFT},
        {Vector3i(-1,-1,-1), DIRECTION_LEFT},
        {Vector3i(-1,0,-1), DIRECTION_LEFT},
        {Vector3i(-1,0,1), DIRECTION_LEFT},

        {Vector3i(0,1,1), DIRECTION_UP},
        {Vector3i(0,1,-1), DIRECTION_UP},

        {Vector3i(0,-1,1), DIRECTION_DOWN},
        {Vector3i(0,-1,-1), DIRECTION_DOWN}
    };

    const uint8_t direction_from_vector(const Vector3i &unit_vector) {
        return vector_to_direction.at(unit_vector);
    }

    const uint8_t direction_from_vector(const Vector3i &from, const Vector3i &to) {
        return direction_from_vector(from - to);
    }

    const uint8_t rotation_from_vector(const uint8_t direction, const Vector3f &vector) {
        switch(direction) {
        case DIRECTION_UP:
            if(fabs(vector(0)) > fabs(vector(2))) {
                if(vector(0) > 0) {
                    return ROTATION_RIGHT;
                } else {
                    return ROTATION_LEFT;
                }
            } else {
                if(vector(2) > 0) {
                    return ROTATION_IDENTITY;
                } else {
                    return ROTATION_HALF;
                }
            }
        case DIRECTION_DOWN:
            if(fabs(vector(0)) > fabs(vector(2))) {
                if(vector(0) <= 0) {
                    return ROTATION_LEFT;
                } else {
                    return ROTATION_RIGHT;
                }
            } else {
                if(vector(2) <= 0) {
                    return ROTATION_HALF;
                } else {
                    return ROTATION_IDENTITY;
                }
            }
        case DIRECTION_LEFT:
            if(fabs(vector(1)) > fabs(vector(2))) {
                if(vector(1) > 0) {
                    return ROTATION_RIGHT;
                } else {
                    return ROTATION_LEFT;
                }

            } else {
                if(vector(2) > 0) {
                    return ROTATION_IDENTITY;
                } else {
                    return ROTATION_HALF;
                }
            }
        case DIRECTION_RIGHT:
            if(fabs(vector(1)) > fabs(vector(2))) {
                if(vector(1) <= 0) {
                    return ROTATION_RIGHT;
                } else {
                    return ROTATION_LEFT;
                }
            } else {
                if(vector(2) <= 0) {
                    return ROTATION_HALF;
                } else {
                    return ROTATION_IDENTITY;
                }
            }
        case DIRECTION_FORWARD:
            if(fabs(vector(0)) > fabs(vector(1))) {
                if(vector(0) > 0) {
                    return ROTATION_RIGHT;
                } else {
                    return ROTATION_LEFT;
                }
            } else {
                if(vector(1) > 0) {
                    return ROTATION_IDENTITY;
                } else {
                    return ROTATION_HALF;
                }
            }
        case DIRECTION_BACKWARD:
            if(fabs(vector(0)) > fabs(vector(1))) {
                if(vector(0) <= 0) {
                    return ROTATION_RIGHT;
                } else {
                    return ROTATION_LEFT;
                }
            } else {
                if(vector(1) <= 0) {
                    return ROTATION_IDENTITY;
                } else {
                    return ROTATION_HALF;
                }
            }
        }
    }
};

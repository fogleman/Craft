#ifndef __PLAYER_H__
#define __PLAYER_H__
#include <utility>
#include <Eigen/Geometry>
#include "optional.hpp"
#include "world.h"
#include "block.h"

namespace konstructs {

    using namespace Eigen;
    using nonstd::optional;
    using std::pair;

    class Player {
    public:
        Player(const int id, const Vector3f position,
               const float rx, const float ry);
        Matrix4f direction() const;
        Matrix4f translation() const;
        Matrix4f view() const;
        Vector3f camera() const;
        Vector3f camera_direction() const;
        Vector3i feet() const;
        bool can_place(Vector3i block, const World &world, const BlockTypeInfo &blocks);
        Vector3f update_position(int sz, int sx, float dt,
                                 const World &world, const BlockTypeInfo &blocks,
                                 const float near_distance, const bool jump, const bool sneaking);
        optional<pair<Block, Block>> looking_at(const World &world,
                                                const BlockTypeInfo &blocks) const;
        void rotate_x(float speed);
        void rotate_y(float speed);
        int id;
        void fly();
        float rx();
        float ry();
        Vector3f position;
    private:
        int collide(const World &world, const BlockTypeInfo &blocks,
                    const float near_distance, const bool sneaking);
        float mrx;
        float mry;
        bool flying;
        float dy;
    };

};

#endif

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
        Player(const int _id, const Vector3f _position,
               const float _rx, const float _ry);
        Matrix4f direction() const;
        Matrix4f view() const;
        Vector3f camera() const;
        Vector3f camera_direction() const;
        Vector3f update_position(int sz, int sx, float dt,
                                 const World &world, const BlockData &blocks,
                                 const float near_distance, const bool jump);
        optional<pair<Block, Block>> looking_at(const World &world,
                                                const BlockData &blocks) const;
        void rotate_x(float speed);
        void rotate_y(float speed);
        int id;
        void fly();
        float rx();
        float ry();
    private:
        int collide(const World &world, const BlockData &blocks, const float near_distance);
        Vector3f position;
        float mrx;
        float mry;
        bool flying;
        float dy;
    };

};

#endif

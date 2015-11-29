#ifndef __PLAYER_H__
#define __PLAYER_H__
#include <Eigen/Geometry>
#include "world.h"
#include "block.h"

namespace konstructs {

    using namespace Eigen;

    class Player {
    public:
        Player(const int _id, const Vector3f _position,
               const float _rx, const float _ry);
        Matrix4f direction() const;
        Matrix4f view() const;
        Vector3f camera() const;
        Vector3f update_position(int sz, int sx, float dt,
                                 const World &world, const BlockData &blocks,
                                 const float near_distance);
        void rotate_x(float speed);
        void rotate_y(float speed);
        int id;
        float rx();
        float ry();
    private:
        int collide(const World &world, const BlockData &blocks, const float near_distance);
        Vector3f position;
        float mrx;
        float mry;
    };

};

#endif

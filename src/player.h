#ifndef __PLAYER_H__
#define __PLAYER_H__
#include <Eigen/Geometry>

namespace konstructs {

    using namespace Eigen;

    class Player {
    public:
        Player(const Vector3f _position, const float _rx, const float _ry, const float _t);
        Matrix4f view() const;
        void update_position(int sz, int sx);
        void rotate_x(float speed);
        void rotate_y(float speed);
    private:
        Vector3f position;
        float rx;
        float ry;
        float t;
    };

};

#endif

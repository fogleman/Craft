#ifndef __PLAYER_H__
#define __PLAYER_H__
#include <Eigen/Geometry>

namespace konstructs {

    using namespace Eigen;

    class Player {
    public:
        Player(const int _id, const Vector3f _position,
               const float _rx, const float _ry);
        Matrix4f view() const;
        Vector3f camera() const;
        Vector3f update_position(int sz, int sx);
        void rotate_x(float speed);
        void rotate_y(float speed);
        int id;
        float rx();
        float ry();
    private:
        Vector3f position;
        float mrx;
        float mry;
    };

};

#endif

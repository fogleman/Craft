#include <iostream>
#include <algorithm>
#include "player.h"
#include "matrix.h"
#include "math.h"

namespace konstructs {

    using namespace Eigen;

    Player::Player(const int _id, const Vector3f _position, const float _rx,
                   const float _ry):
        id(_id), position(_position), rx(_rx), ry(_ry) {}

    Matrix4f Player::view() const {
        return (Affine3f(AngleAxisf(rx, Vector3f::UnitX())) *
                Affine3f(AngleAxisf(ry, Vector3f::UnitY())) *
                Affine3f(Translation3f(-position))).matrix();
    }

    Vector3f Player::update_position(int sz, int sx) {
        if (!sz && !sx) {
            return position;
        }
        float strafe = atan2f(sz, sx);
        position += Vector3f(cosf(ry + strafe) * 0.1, 0.0f, sinf(ry + strafe) * 0.1);
        return position;
    }

    void Player::rotate_x(float speed) {
        rx += speed;
        rx = std::max(rx, -((float)M_PI / 2.0f));
        rx = std::min(rx, ((float)M_PI / 2.0f));
    }

    void Player::rotate_y(float speed) {
        ry += speed;
        if (ry < 0) {
            ry += (M_PI * 2);
        }
        if (ry >= (M_PI * 2)){
            ry -= (M_PI * 2);
        }
    }

};

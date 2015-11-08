#include <iostream>
#include <algorithm>
#include "player.h"
#include "matrix.h"
#include "math.h"

namespace konstructs {

    using namespace Eigen;

    Player::Player(const int _id, const Vector3f _position, const float _rx,
                   const float _ry):
        id(_id), position(_position), mrx(_rx), mry(_ry) {}

    Matrix4f Player::view() const {
        return (Affine3f(AngleAxisf(mrx, Vector3f::UnitX())) *
                Affine3f(AngleAxisf(mry, Vector3f::UnitY())) *
                Affine3f(Translation3f(-position))).matrix();
    }

    Vector3f Player::update_position(int sz, int sx) {
        if (!sz && !sx) {
            return position;
        }
        float strafe = atan2f(sz, sx);
        position += Vector3f(cosf(mry + strafe) * 0.1, 0.0f, sinf(mry + strafe) * 0.1);
        return position;
    }

    void Player::rotate_x(float speed) {
        mrx += speed;
        mrx = std::max(mrx, -((float)M_PI / 2.0f));
        mrx = std::min(mrx, ((float)M_PI / 2.0f));
    }

    void Player::rotate_y(float speed) {
        mry += speed;
        if (mry < 0) {
            mry += (M_PI * 2);
        }
        if (mry >= (M_PI * 2)){
            mry -= (M_PI * 2);
        }
    }

    float Player::rx() {
        return mrx;
    }

    float Player::ry() {
        return mry;
    }

};

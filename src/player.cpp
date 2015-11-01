#include <iostream>
#include "player.h"
#include "matrix.h"

namespace konstructs {

    using namespace Eigen;
    using namespace matrix;

    Player::Player(State _state) : state(_state) {}

    Matrix4f Player::view() const {
        return (Affine3f(AngleAxisf(state.rx, Vector3f::UnitX())) *
                Affine3f(AngleAxisf(state.ry, Vector3f::UnitY())) *
                Affine3f(Translation3f(-state.position))).matrix();
    }

};

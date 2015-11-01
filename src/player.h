#ifndef __PLAYER_H__
#define __PLAYER_H__
#include <Eigen/Geometry>

namespace konstructs {

    using namespace Eigen;

    class State {
    public:
        State(const Vector3f _position, const float _rx, const float _ry, const float _t):
            position(_position), rx(_rx), ry(_ry), t(_t) {}
        const Vector3f position;
        const float rx;
        const float ry;
        const float t;
    };

    class Player {
    public:
        Player(State _state);
        Matrix4f view() const;
    private:
        State state;
    };

};

#endif

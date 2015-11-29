#include <algorithm>
#include "player.h"
#include "matrix.h"
#include "math.h"

namespace konstructs {

    using namespace Eigen;

    Player::Player(const int _id, const Vector3f _position, const float _rx,
                   const float _ry):
        id(_id), position(_position), mrx(_rx), mry(_ry) {}

    Matrix4f Player::direction() const {
        return (Affine3f(AngleAxisf(mrx, Vector3f::UnitX())) *
                Affine3f(AngleAxisf(mry, Vector3f::UnitY()))).matrix();
    }

    Matrix4f Player::view() const {
        return (Affine3f(AngleAxisf(mrx, Vector3f::UnitX())) *
                Affine3f(AngleAxisf(mry, Vector3f::UnitY())) *
                Affine3f(Translation3f(-position))).matrix();
    }

    Vector3f Player::camera() const {
        return position;
    }

    Vector3f Player::update_position(int sz, int sx, float dt,
                                     const World &world, const BlockData &blocks,
                                     const float near_distance) {
        if (!sz && !sx) {
            return position;
        }
        float strafe = atan2f(sz, sx);
        position += Vector3f(cosf(mry + strafe), 0.0f, sinf(mry + strafe)) * (dt * 5);
        collide(world, blocks, near_distance);
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

    int Player::collide(const World &world, const BlockData &blocks, const float near_distance) {
        int result = 0;
        float x = position[0];
        float y = position[1];
        float z = position[2];
        int height = 2;
        int p = chunked(x);
        int q = chunked(z);
        int k = chunked(y);
        int nx = roundf(x);
        int ny = roundf(y);
        int nz = roundf(z);
        float px = x - nx;
        float py = y - ny;
        float pz = z - nz;
        float pad = near_distance * 2;
        int r = 1;
        for (int dp = -r; dp <= r; dp++) {
            for (int dq = -r; dq <= r; dq++) {
                for (int dk = -r; dk <= r; dk++) {
                    try {
                        ChunkData *chunk = world.at(Vector3i(p + dp, q + dq, k + dk)).get();
                        for (int dy = 0; dy < height; dy++) {
                            if (px < -pad && blocks.is_obstacle[chunk->get(Vector3i(nx - 1, ny - dy, nz))]) {
                                position[0] = nx - pad;
                            }
                            if (px > pad && blocks.is_obstacle[chunk->get(Vector3i(nx + 1, ny - dy, nz))]) {
                                position[0] = nx + pad;
                            }
                            if (py < -pad && blocks.is_obstacle[chunk->get(Vector3i(nx, ny - dy - 1, nz))]) {
                                position[1] = ny - pad;
                                result = 1;
                            }
                            if (py > pad && blocks.is_obstacle[chunk->get(Vector3i(nx, ny - dy + 1, nz))]) {
                                position[1] = ny + pad;
                                result = 1;
                            }
                            if (pz < -pad && blocks.is_obstacle[chunk->get(Vector3i(nx, ny - dy, nz - 1))]) {
                                position[2] = nz - pad;
                            }
                            if (pz > pad && blocks.is_obstacle[chunk->get(Vector3i(nx, ny - dy, nz + 1))]) {
                                position[2] = nz + pad;
                            }
                        }
                    } catch(std::out_of_range e) {
                        continue;
                    }
                }
            }
        }

        return result;
    }
};

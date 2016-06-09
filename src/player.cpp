#include <algorithm>
#include "player.h"
#include "matrix.h"
#include "math.h"

namespace konstructs {

    using namespace Eigen;
    using nonstd::nullopt;

    static float CAMERA_OFFSET = 0.5f;
    static Vector3f CAMERA_OFFSET_VECTOR = Vector3f(0, CAMERA_OFFSET, 0);

    Player::Player(const int id, const Vector3f position, const float rx,
                   const float ry):
        id(id), position(position), mrx(rx), mry(ry), flying(false), dy(0) {}

    Matrix4f Player::direction() const {
        return (Affine3f(AngleAxisf(mrx, Vector3f::UnitX())) *
                Affine3f(AngleAxisf(mry, Vector3f::UnitY()))).matrix();
    }

    Matrix4f Player::translation() const {
        return (Affine3f(Translation3f(position)) *
                Affine3f(AngleAxisf(-mry, Vector3f::UnitY())) *
                Affine3f(AngleAxisf(-mrx, Vector3f::UnitX()))).matrix();
    }

    Matrix4f Player::view() const {
        return (Affine3f(AngleAxisf(mrx, Vector3f::UnitX())) *
                Affine3f(AngleAxisf(mry, Vector3f::UnitY())) *
                Affine3f(Translation3f(-camera()))).matrix();
    }

    Vector3f Player::camera() const {
        return position + CAMERA_OFFSET_VECTOR;
    }

    Vector3f Player::camera_direction() const {
        float m = cosf(mrx);
        Vector3f vec(cosf(mry - (M_PI / 2.0f)) * m, -sinf(mrx), sinf(mry - (M_PI / 2.0f)) * m);
        vec.normalize();
        return vec;
    }

    Vector3f Player::update_position(int sz, int sx, float dt,
                                     const World &world, const BlockData &blocks,
                                     const float near_distance, const bool jump) {
        float vx = 0, vy = 0, vz = 0;
        if (!sz && !sx) { // Not mowing in X or Z
            vx = 0;
            vz = 0;
        } else { // Moving in X or Z


            float strafe = atan2f(sz, sx);

            if (flying) {
                float m = cosf(mrx);
                float y = sinf(mrx);
                if (sx) {
                    if (!sz) {
                        y = 0;
                    }
                    m = 1;
                }
                if (sz < 0) {
                    y = -y;
                }
                vx = cosf(mry + strafe) * m;
                vy = y;
                vz = sinf(mry + strafe) * m;
            } else {
                vx = cosf(mry + strafe);
                vy = 0;
                vz = sinf(mry + strafe);
            }
        }

        if(jump) {
            if(flying) {
                // Jump in flight moves upward at constant speed
                vy = 1;
            } else if(dy == 0) {
                // Jump when walking changes the acceleration upwards to 8
                dy = 8;
            } else {
                // Get middle of block
                Vector3i iPos((int)(position[0] + 0.5f), (int)(position[1]), (int)(position[2] + 0.5f));
                ChunkData *chunk = world.chunk_at(iPos).get();
                if(blocks.state[chunk->get(iPos)] == STATE_LIQUID) {
                    dy = 5.5;
                }
            }
        }

        float speed = flying ? 20 : 5;
        int estimate =
            roundf(sqrtf(powf(vx * speed, 2) +
                         powf(vy * speed + std::abs(dy) * 2, 2) +
                         powf(vz * speed, 2)) * dt * 8);
        int step = std::max(8, estimate);
        float ut = dt / step;
        vx = vx * ut * speed;
        vy = vy * ut * speed;
        vz = vz * ut * speed;
        for (int i = 0; i < step; i++) {
            if (flying) {
                // When flying upwards acceleration is constant i.e. not falling
                dy = 0;
            } else {
                // Calculate "gravity" by decreasing upwards acceleration
                dy -= ut * 25;
                dy = std::max(dy, -250.0f);
            }
            position += Vector3f(vx, vy + dy * ut, vz);
            if (collide(world, blocks, near_distance)) {
                dy = 0;
            }
        }
        if (position[1] < 0) {
            position[1] = 2;
        }
        return position;
    }

    optional<pair<Block, Block>> Player::looking_at(const World &world,
                                                    const BlockData &blocks) const {
        optional<pair<Block, Block>> block(nullopt);
        float best = 0;
        const Vector3f v = camera_direction();
        const Vector3f camera_position = camera();
        int p = chunked(camera_position[0]);
        int q = chunked(camera_position[2]);
        int k = chunked(camera_position[1]);
        const auto &atAndAround = world.atAndAround({p, q, k});
        for (const auto &chunk: atAndAround) {
            const auto &seen = chunk->get(camera_position, v, 8.0f, blocks);
            if (seen) {
                auto h = seen->second;
                float d = sqrtf(powf(h.position[0] - camera_position[0], 2) +
                                powf(h.position[1] - camera_position[1], 2) +
                                powf(h.position[2] - camera_position[2], 2));
                if (best == 0 || d < best) {
                    best = d;
                    block = seen;
                }
            }
        }
        return block;
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

    void Player::fly() {
        flying = !flying;
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
                        ChunkData *chunk = world.chunk(Vector3i(p + dp, q + dq, k + dk)).get();
                        if (blocks.is_obstacle[chunk->get(Vector3i(nx, ny-1, nz))]) {
                            position[1] += 1.0f;
                            return 1;
                        }
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
                            if (py > (pad - CAMERA_OFFSET) && blocks.is_obstacle[chunk->get(Vector3i(nx, ny - dy + 1, nz))]) {
                                position[1] = ny + pad - CAMERA_OFFSET;
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

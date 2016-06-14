#define _USE_MATH_DEFINES
#include <iostream>
#include <math.h>
#include "chunk_shader.h"
#include "matrix.h"

namespace konstructs {

    const Array3i chunk_offset = Vector3i(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE).array();
    const int NO_CHUNK_FOUND = 0x0FFFFFFF;
    ChunkModel::ChunkModel(const shared_ptr<ChunkModelResult> &data,
                           GLuint data_attr) :
        position(data->position),
        faces(data->faces),
        data_attr(data_attr) {
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, data->size * sizeof(GLuint),
                     data->data(), GL_STATIC_DRAW);
        Vector3f pos =
            (position.array() * chunk_offset).matrix().cast<float>();

        Vector3f rpos = Vector3f(pos[0], pos[2], pos[1]);
        translation = Affine3f(Translation3f(rpos)).matrix();
    }

    int ChunkModel::vertices() {
        return faces*6;
    }

    void ChunkModel::bind() {
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glEnableVertexAttribArray(data_attr);
        glVertexAttribIPointer(data_attr, 2, GL_UNSIGNED_INT,
                              0, 0);
    }

    void ChunkShader::add(const shared_ptr<ChunkModelResult> &data) {
        auto it = models.find(data->position);
        auto model = new ChunkModel(data, data_attr);
        if (it != models.end()) {
            auto second = it->second;
            it->second = model;
            delete second;
        } else {
            models.insert({data->position, model});
        }

    }

    ChunkShader::ChunkShader(const float _fov, const GLuint _block_texture,
                             const GLuint _sky_texture, const float _near_distance, const string &vert_str,
                             const string &frag_str) :
        ShaderProgram("chunk", vert_str, frag_str),
        data_attr(attributeId("data")),
        matrix(uniformId("matrix")),
        translation(uniformId("translation")),
        sampler(uniformId("sampler")),
        sky_sampler(uniformId("sky_sampler")),
        fog_distance(uniformId("fog_distance")),
        light_color(uniformId("light_color")),
        ambient_light(uniformId("ambient_light")),
        timer(uniformId("timer")),
        camera(uniformId("camera")),
        fov(_fov),
        block_texture(_block_texture),
        sky_texture(_sky_texture),
        near_distance(_near_distance) {}

    int ChunkShader::render(const Player &player, const int width, const int height,
                            const float current_daylight, const float current_timer,
                            World &world, Client &client, const int radius) {
        int faces = 0;
        int visible = 0;
        bind([&](Context c) {
                c.enable(GL_DEPTH_TEST);
                c.enable(GL_CULL_FACE);
                float aspect_ratio = (float)width / (float)height;
                float max_distance = (radius - 1) * CHUNK_SIZE;
                const Matrix4f m = matrix::projection_perspective(fov, aspect_ratio, near_distance, max_distance) * player.view();
                c.set(matrix, m);
                c.set(sampler, (int)block_texture);
                c.set(sky_sampler, (int)sky_texture);
                c.set(fog_distance, max_distance);
                float value = min(1.0f, current_daylight);
                float v = value * 0.3 + 0.2;
                c.set(light_color, Vector3f(v, v, v));
                Vector3f ambient((float)sin(M_PI*current_daylight)/2 + v, (float)sin(M_PI*current_daylight)/4 + v, v);
                c.set(ambient_light, ambient);
                c.set(timer, current_timer);
                c.set(camera, player.camera());
                float planes[6][4];
                matrix::ext_frustum_planes(planes, radius, m);
                Vector3i player_chunk = chunked_vec(player.camera());
                Vector3i best_chunk;
                int best_chunk_score = NO_CHUNK_FOUND;
                for(int p = player_chunk[0] - (radius - 1); p < player_chunk[0] + radius; p++) {
                    for(int q = player_chunk[1] - (radius - 1); q < player_chunk[1] + radius; q++) {
                        for(int k = player_chunk[2] - (radius - 1); k < player_chunk[2] + radius; k++) {
                            Vector3i pos(p, q, k);
                            int distance = (pos - player_chunk).norm();
                            int score;
                            auto it = models.find(pos);
                            bool is_updated = world.chunk_updated_since_requested(pos);
                            if(chunk_visible(planes, pos)) {
                                if(it != models.end()) {
                                    /* Ok, found the model, let's render it!
                                     */
                                    const auto &m = it->second;
                                    visible++;
                                    c.set(translation, m->translation);
                                    c.draw(m);
                                    faces += m->faces;
                                    if(is_updated) {
                                        /* We already have a model of the chunk,
                                         * but it's outdated, so it needs to be refreshed.
                                         */
                                        score = distance / 2;
                                    } else {
                                        /* We already have a model of the chunk,
                                         * it didn't change since last requested,
                                         * so we really don't need it.
                                         */
                                        score = NO_CHUNK_FOUND;
                                    }
                                } else {
                                    /* We wanted to render the model,
                                     * but we didn't have it, so we really want this chunk!
                                     */
                                    score = distance / 2;
                                }
                            } else {
                                /* The player can not see the chunk right now,
                                 * but if she turns around she might need this chunk
                                 * so let's fetch it, but not super urgent
                                 */
                                if(it != models.end() && !is_updated) {
                                    /* We already have it and it's not updated */
                                    score = NO_CHUNK_FOUND;
                                } else {
                                    /* We don't have it or it's update*/
                                    score = distance;
                                }
                            }
                            if(score < best_chunk_score && world.chunk_not_requested(pos)) {
                                best_chunk_score = score;
                                best_chunk = pos;
                            }
                        }
                    }
                }
                if(best_chunk_score != NO_CHUNK_FOUND) {
                    world.request_chunk(best_chunk, client);
                }
                c.disable(GL_CULL_FACE);
                c.disable(GL_DEPTH_TEST);
            });
        return faces;
    }

    void ChunkShader::delete_unused_models(const Vector3f position, const int radi) {
        Vector3i player_chunk = chunked_vec(position);
        for ( auto it = models.begin(); it != models.end();) {
            if ((it->second->position - player_chunk).norm() > radi) {
                it = models.erase(it);
            } else {
                ++it;
            }
        }
    }

    bool chunk_visible(const float planes[6][4], const Vector3i &position) {
        float x = position[0] * CHUNK_SIZE - 1;
        float z = position[1] * CHUNK_SIZE - 1;
        float y = position[2] * CHUNK_SIZE - 1;
        float d = CHUNK_SIZE + 1;
        float points[8][3] = {
            {x + 0, y + 0, z + 0},
            {x + d, y + 0, z + 0},
            {x + 0, y + 0, z + d},
            {x + d, y + 0, z + d},
            {x + 0, y + d, z + 0},
            {x + d, y + d, z + 0},
            {x + 0, y + d, z + d},
            {x + d, y + d, z + d}
        };
        for (int i = 0; i < 6; i++) {
            int in = 0;
            int out = 0;
            for (int j = 0; j < 8; j++) {
                float d =
                    planes[i][0] * points[j][0] +
                    planes[i][1] * points[j][1] +
                    planes[i][2] * points[j][2] +
                    planes[i][3];
                if (d < 0) {
                    out++;
                }
                else {
                    in++;
                }
                if (in && out) {
                    break;
                }
            }
            if (in == 0) {
                return false;
            }
        }
        return true;
    }

};

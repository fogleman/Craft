#include <thread>
#include "chunk_factory.h"
#include "noise/noise.h"
#include "util.h"
#include "cube.h"

#define WORKERS 4

namespace konstructs {
    ChunkModelResult::ChunkModelResult(const Vector3i _position, const int components,
                                       const int faces):
        position(_position), size(6 * components * faces) {
        mData = new GLfloat[size];
    }


    ChunkModelResult::~ChunkModelResult() {
        delete[] mData;
    }

    GLfloat *ChunkModelResult::data() {
        return mData;
    }

    ChunkModelFactory::ChunkModelFactory(const BlockData &_block_data) :
        block_data(_block_data),
        BELOW(0, 0, -1),
        ABOVE(0, 0, 1),
        LEFT(-1, 0, 0),
        RIGHT(1, 0, 0),
        FRONT(0, -1, 0),
        BACK(0, 1, 0),
        ABOVE_LEFT(-1, 0, 1),
        ABOVE_RIGHT(1, 0, 1),
        ABOVE_FRONT(0, -1, 1),
        ABOVE_BACK(0, 1, 1),
        ABOVE_LEFT_FRONT(-1, -1, 1),
        ABOVE_RIGHT_FRONT(1, -1, 1),
        ABOVE_LEFT_BACK(-1, 1, 1),
        ABOVE_RIGHT_BACK(1, 1, 1),
        LEFT_FRONT(-1, -1, 0),
        RIGHT_FRONT(1, -1, 0),
        LEFT_BACK(-1, 1, 0),
        RIGHT_BACK(1, 1, 0),
        SOLID_CHUNK(std::make_shared<ChunkData>()) {
        for(int i = 0; i < WORKERS; i++) {
            new std::thread(&ChunkModelFactory::worker, this);
        }
    }

    const std::shared_ptr<ChunkData>
    ChunkModelFactory::get_chunk(const Vector3i &position,
                                 const std::unordered_map<Vector3i,
                                 shared_ptr<ChunkData>,
                                 matrix_hash<Vector3i>> &chunks) {
        try {
            return chunks.at(position);
        } catch(std::out_of_range e) {
            return SOLID_CHUNK;
        }
    }

    void ChunkModelFactory::create_model(const Vector3i &position,
                                         const std::unordered_map<Vector3i, shared_ptr<ChunkData>, matrix_hash<Vector3i>> &chunk_data) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            chunks.push(create_model_data(position, chunk_data));
            if(chunk_data.find(position + BELOW) != chunk_data.end())
                chunks.push(create_model_data(position + BELOW, chunk_data));
            if(chunk_data.find(position + ABOVE) != chunk_data.end())
                chunks.push(create_model_data(position + ABOVE, chunk_data));
            if(chunk_data.find(position + LEFT) != chunk_data.end())
                chunks.push(create_model_data(position + LEFT, chunk_data));
            if(chunk_data.find(position + RIGHT) != chunk_data.end())
                chunks.push(create_model_data(position + RIGHT, chunk_data));
            if(chunk_data.find(position + FRONT) != chunk_data.end())
                chunks.push(create_model_data(position + FRONT, chunk_data));
            if(chunk_data.find(position + BACK) != chunk_data.end())
                chunks.push(create_model_data(position + BACK, chunk_data));
        }
        chunks_condition.notify_all();
    }

    const ChunkModelData ChunkModelFactory::create_model_data(const Vector3i &position,
                                                              const std::unordered_map<Vector3i, shared_ptr<ChunkData>, matrix_hash<Vector3i>> &chunk_data) {
        const ChunkModelData data = {
            position,
            get_chunk(position + BELOW, chunk_data),
            get_chunk(position + ABOVE, chunk_data),
            get_chunk(position + LEFT, chunk_data),
            get_chunk(position + RIGHT, chunk_data),
            get_chunk(position + FRONT, chunk_data),
            get_chunk(position + BACK, chunk_data),
            get_chunk(position + ABOVE_LEFT, chunk_data),
            get_chunk(position + ABOVE_RIGHT, chunk_data),
            get_chunk(position + ABOVE_FRONT, chunk_data),
            get_chunk(position + ABOVE_BACK, chunk_data),
            get_chunk(position + ABOVE_LEFT_FRONT, chunk_data),
            get_chunk(position + ABOVE_RIGHT_FRONT, chunk_data),
            get_chunk(position + ABOVE_LEFT_BACK, chunk_data),
            get_chunk(position + ABOVE_RIGHT_BACK, chunk_data),
            get_chunk(position + LEFT_FRONT, chunk_data),
            get_chunk(position + LEFT_BACK, chunk_data),
            get_chunk(position + RIGHT_FRONT, chunk_data),
            get_chunk(position + RIGHT_BACK, chunk_data),
            get_chunk(position, chunk_data)
        };
        return data;
    }

    std::vector<std::shared_ptr<ChunkModelResult>> ChunkModelFactory::fetch_models() {
        std::vector<std::shared_ptr<ChunkModelResult>> return_models;
        {
            std::lock_guard<std::mutex> lock(mutex);
            return_models = models;
            models.clear();
        }
        return return_models;
    }

    void ChunkModelFactory::worker() {
        while(1) {
            std::unique_lock<std::mutex> lk(mutex);
            chunks_condition.wait(lk, [&]{return !chunks.empty();});
            auto data = chunks.front();
            chunks.pop();
            lk.unlock();
            auto result = compute_chunk(data, block_data);
            {
                std::lock_guard<std::mutex> lock(mutex);
                models.push_back(result);
            }
        }
    }

#define XZ_SIZE (CHUNK_SIZE * 3 + 2)
#define XZ_LO (CHUNK_SIZE)
#define XZ_HI (CHUNK_SIZE * 2 + 1)
#define XYZ(x, y, z) ((y) * XZ_SIZE * XZ_SIZE + (x) * XZ_SIZE + (z))
#define XZ(x, z) ((x) * XZ_SIZE + (z))

    void occlusion(
                   char neighbors[27], float shades[27],
                   float ao[6][4], float light[6][4])
    {
        static const int lookup3[6][4][3] = {
            {{0, 1, 3}, {2, 1, 5}, {6, 3, 7}, {8, 5, 7}},
            {{18, 19, 21}, {20, 19, 23}, {24, 21, 25}, {26, 23, 25}},
            {{6, 7, 15}, {8, 7, 17}, {24, 15, 25}, {26, 17, 25}},
            {{0, 1, 9}, {2, 1, 11}, {18, 9, 19}, {20, 11, 19}},
            {{0, 3, 9}, {6, 3, 15}, {18, 9, 21}, {24, 15, 21}},
            {{2, 5, 11}, {8, 5, 17}, {20, 11, 23}, {26, 17, 23}}
        };
        static const int lookup4[6][4][4] = {
            {{0, 1, 3, 4}, {1, 2, 4, 5}, {3, 4, 6, 7}, {4, 5, 7, 8}},
            {{18, 19, 21, 22}, {19, 20, 22, 23}, {21, 22, 24, 25}, {22, 23, 25, 26}},
            {{6, 7, 15, 16}, {7, 8, 16, 17}, {15, 16, 24, 25}, {16, 17, 25, 26}},
            {{0, 1, 9, 10}, {1, 2, 10, 11}, {9, 10, 18, 19}, {10, 11, 19, 20}},
            {{0, 3, 9, 12}, {3, 6, 12, 15}, {9, 12, 18, 21}, {12, 15, 21, 24}},
            {{2, 5, 11, 14}, {5, 8, 14, 17}, {11, 14, 20, 23}, {14, 17, 23, 26}}
        };
        static const float curve[4] = {0.0, 0.25, 0.5, 0.75};
        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < 4; j++) {
                int corner = neighbors[lookup3[i][j][0]];
                int side1 = neighbors[lookup3[i][j][1]];
                int side2 = neighbors[lookup3[i][j][2]];
                int value = side1 && side2 ? 3 : corner + side1 + side2;
                float shade_sum = 0;
                float light_sum = 0;
                for (int k = 0; k < 4; k++) {
                    shade_sum += shades[lookup4[i][j][k]];
                }
                float total = curve[value] + shade_sum / 4.0;
                ao[i][j] = std::min(total, 1.0f);
                light[i][j] = light_sum / 15.0 / 4.0;
            }
        }
    }


    GLfloat *malloc_faces(int components, int faces) {
        return (GLfloat*)malloc(sizeof(GLfloat) * 6 * components * faces);
    }

    shared_ptr<ChunkModelResult> compute_chunk(const ChunkModelData &data,
                                               const BlockData &block_data) {
        char *opaque = (char *)calloc(XZ_SIZE * XZ_SIZE * XZ_SIZE, sizeof(char));
        char *highest = (char *)calloc(XZ_SIZE * XZ_SIZE, sizeof(char));
        const char *is_transparent = block_data.is_transparent;
        const char *is_plant = block_data.is_plant;

        int ox = - CHUNK_SIZE - 1;
        int oy = - CHUNK_SIZE - 1;
        int oz = - CHUNK_SIZE - 1;


        /* Populate the opaque array with the chunk itself */
        const char *blocks = data.self->blocks();

        CHUNK_FOR_EACH(blocks, ex, ey, ez, ew) {
            int x = ex - ox;
            int y = ey - oy;
            int z = ez - oz;
            int w = ew;
            opaque[XYZ(x, y, z)] = !is_transparent[w];
            if (opaque[XYZ(x, y, z)]) {
                highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH;

        /* With the six sides of the chunk */

        /* Populate the opaque array with the chunk below */
        CHUNK_FOR_EACH_XZ(data.below->blocks(), CHUNK_SIZE - 1, ex, ey, ez, ew) {
            int x = ex - ox;
            int y = ey - CHUNK_SIZE - oy;
            int z = ez - oz;
            int w = ew;
            opaque[XYZ(x, y, z)] = !is_transparent[w];
            if (opaque[XYZ(x, y, z)]) {
                highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH_2D;


        /* Populate the opaque array with the chunk above
         * The shading requires additional 8 blocks
         */
        for(int i = 0; i < 8; i++) {
            CHUNK_FOR_EACH_XZ(data.above->blocks(), i, ex, ey, ez, ew) {
                int x = ex - ox;
                int y = ey + CHUNK_SIZE - oy;
                int z = ez - oz;
                int w = ew;
                opaque[XYZ(x, y, z)] = !is_transparent[w];
                if (opaque[XYZ(x, y, z)]) {
                    highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
                }
            } END_CHUNK_FOR_EACH_2D;
        }

        /* Populate the opaque array with the chunk left */
        CHUNK_FOR_EACH_YZ(data.left->blocks(), CHUNK_SIZE - 1, ex, ey, ez, ew) {
            int x = ex - CHUNK_SIZE - ox;
            int y = ey - oy;
            int z = ez - oz;
            int w = ew;
            opaque[XYZ(x, y, z)] = !is_transparent[w];
            if (opaque[XYZ(x, y, z)]) {
                highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH_2D;

        /* Populate the opaque array with the chunk right */
        CHUNK_FOR_EACH_YZ(data.right->blocks(), 0, ex, ey, ez, ew) {
            int x = ex + CHUNK_SIZE - ox;
            int y = ey - oy;
            int z = ez - oz;
            int w = ew;
            opaque[XYZ(x, y, z)] = !is_transparent[w];
            if (opaque[XYZ(x, y, z)]) {
                highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH_2D;


        /* Populate the opaque array with the chunk front */
        CHUNK_FOR_EACH_XY(data.front->blocks(), CHUNK_SIZE - 1, ex, ey, ez, ew) {
            int x = ex - ox;
            int y = ey - oy;
            int z = ez - CHUNK_SIZE - oz;
            int w = ew;
            opaque[XYZ(x, y, z)] = !is_transparent[w];
            if (opaque[XYZ(x, y, z)]) {
                highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH_2D;


        /* Populate the opaque array with the chunk back */
        CHUNK_FOR_EACH_XY(data.back->blocks(), 0, ex, ey, ez, ew) {
            int x = ex - ox;
            int y = ey - oy;
            int z = ez + CHUNK_SIZE - oz;
            int w = ew;
            opaque[XYZ(x, y, z)] = !is_transparent[w];
            if (opaque[XYZ(x, y, z)]) {
                highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH_2D;

        /* Populate the corner cases above
         * Shading yet again requires 8 additional blocks
         */

        for(int i = 0; i < 8; i++) {
            /* Populate the opaque array with the chunk above-left */
            CHUNK_FOR_EACH_Z(data.above_left->blocks(), CHUNK_SIZE - 1, i, ex, ey, ez, ew) {
                int x = ex - CHUNK_SIZE - ox;
                int y = ey + CHUNK_SIZE - oy;
                int z = ez - oz;
                int w = ew;
                opaque[XYZ(x, y, z)] = !is_transparent[w];
                if (opaque[XYZ(x, y, z)]) {
                    highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
                }
            } END_CHUNK_FOR_EACH_1D;

            /* Populate the opaque array with the chunk above-right */
            CHUNK_FOR_EACH_Z(data.above_right->blocks(), 0, i, ex, ey, ez, ew) {
                int x = ex + CHUNK_SIZE - ox;
                int y = ey + CHUNK_SIZE - oy;
                int z = ez - oz;
                int w = ew;
                opaque[XYZ(x, y, z)] = !is_transparent[w];
                if (opaque[XYZ(x, y, z)]) {
                    highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
                }
            } END_CHUNK_FOR_EACH_1D;

            /* Populate the opaque array with the chunk above-front */
            CHUNK_FOR_EACH_X(data.above_front->blocks(), CHUNK_SIZE - 1, i, ex, ey, ez, ew) {
                int x = ex - ox;
                int y = ey + CHUNK_SIZE - oy;
                int z = ez - CHUNK_SIZE - oz;
                int w = ew;
                opaque[XYZ(x, y, z)] = !is_transparent[w];
                if (opaque[XYZ(x, y, z)]) {
                    highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
                }
            } END_CHUNK_FOR_EACH_1D;
            /* Populate the opaque array with the chunk above-back */
            CHUNK_FOR_EACH_X(data.above_back->blocks(), 0, i, ex, ey, ez, ew) {
                int x = ex - ox;
                int y = ey + CHUNK_SIZE - oy;
                int z = ez + CHUNK_SIZE - oz;
                int w = ew;
                opaque[XYZ(x, y, z)] = !is_transparent[w];
                if (opaque[XYZ(x, y, z)]) {
                    highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
                }
            } END_CHUNK_FOR_EACH_1D;

            /* Populate the opaque array with the block above-left-front */
            {
                int ex = CHUNK_SIZE - 1;
                int ey = i;
                int ez = CHUNK_SIZE - 1;
                int x = ex - CHUNK_SIZE - ox;
                int y = ey + CHUNK_SIZE - oy;
                int z = ez - CHUNK_SIZE - oz;
                int w = data.above_left_front->blocks()[ex+ey*CHUNK_SIZE+ez*CHUNK_SIZE*CHUNK_SIZE];
                opaque[XYZ(x, y, z)] = !is_transparent[w];
                if (opaque[XYZ(x, y, z)]) {
                    highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
                }
            }

            /* Populate the opaque array with the block above-right-front */
            {
                int ex = 0;
                int ey = i;
                int ez = CHUNK_SIZE - 1;
                int x = ex + CHUNK_SIZE - ox;
                int y = ey + CHUNK_SIZE - oy;
                int z = ez - CHUNK_SIZE - oz;
                int w = data.above_right_front->blocks()[ex+ey*CHUNK_SIZE+ez*CHUNK_SIZE*CHUNK_SIZE];
                opaque[XYZ(x, y, z)] = !is_transparent[w];
                if (opaque[XYZ(x, y, z)]) {
                    highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
                }
            }

            /* Populate the opaque array with the block above-left-back */
            {
                int ex = CHUNK_SIZE - 1;
                int ey = i;
                int ez = 0;
                int x = ex - CHUNK_SIZE - ox;
                int y = ey + CHUNK_SIZE - oy;
                int z = ez + CHUNK_SIZE - oz;
                int w = data.above_left_back->blocks()[ex+ey*CHUNK_SIZE+ez*CHUNK_SIZE*CHUNK_SIZE];
                opaque[XYZ(x, y, z)] = !is_transparent[w];
                if (opaque[XYZ(x, y, z)]) {
                    highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
                }
            }

            /* Populate the opaque array with the block above-right-back */
            {
                int ex = 0;
                int ey = i;
                int ez = 0;
                int x = ex + CHUNK_SIZE - ox;
                int y = ey + CHUNK_SIZE - oy;
                int z = ez + CHUNK_SIZE - oz;
                int w = data.above_right_back->blocks()[ex+ey*CHUNK_SIZE+ez*CHUNK_SIZE*CHUNK_SIZE];
                opaque[XYZ(x, y, z)] = !is_transparent[w];
                if (opaque[XYZ(x, y, z)]) {
                    highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
                }
            }

        }

        /* Populate the corner cases on the same level */

        /* Populate the opaque array with the chunk left-front */
        CHUNK_FOR_EACH_Y(data.left_front->blocks(), CHUNK_SIZE - 1, CHUNK_SIZE - 1, ex, ey, ez, ew) {
            int x = ex - CHUNK_SIZE - ox;
            int y = ey - oy;
            int z = ez - CHUNK_SIZE - oz;
            int w = ew;
            opaque[XYZ(x, y, z)] = !is_transparent[w];
            if (opaque[XYZ(x, y, z)]) {
                highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH_1D;

        /* Populate the opaque array with the chunk left-back */
        CHUNK_FOR_EACH_Y(data.left_back->blocks(), CHUNK_SIZE - 1, 0, ex, ey, ez, ew) {
            int x = ex - CHUNK_SIZE - ox;
            int y = ey - oy;
            int z = ez + CHUNK_SIZE - oz;
            int w = ew;
            opaque[XYZ(x, y, z)] = !is_transparent[w];
            if (opaque[XYZ(x, y, z)]) {
                highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH_1D;

        /* Populate the opaque array with the chunk right-front */
        CHUNK_FOR_EACH_Y(data.right_front->blocks(), 0, CHUNK_SIZE - 1, ex, ey, ez, ew) {
            int x = ex + CHUNK_SIZE - ox;
            int y = ey - oy;
            int z = ez - CHUNK_SIZE - oz;
            int w = ew;
            opaque[XYZ(x, y, z)] = !is_transparent[w];
            if (opaque[XYZ(x, y, z)]) {
                highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH_1D;

        /* Populate the opaque array with the chunk right-back */
        CHUNK_FOR_EACH_Y(data.right_back->blocks(), 0, 0, ex, ey, ez, ew) {
            int x = ex + CHUNK_SIZE - ox;
            int y = ey - oy;
            int z = ez + CHUNK_SIZE - oz;
            int w = ew;
            opaque[XYZ(x, y, z)] = !is_transparent[w];
            if (opaque[XYZ(x, y, z)]) {
                highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH_1D;


        // count exposed faces
        int faces = 0;
        CHUNK_FOR_EACH(blocks, ex, ey, ez, ew) {
            if (ew <= 0) {
                continue;
            }
            int x = ex - ox;
            int y = ey - oy;
            int z = ez - oz;
            int f1 = !opaque[XYZ(x - 1, y, z)];
            int f2 = !opaque[XYZ(x + 1, y, z)];
            int f3 = !opaque[XYZ(x, y + 1, z)];
            int f4 = !opaque[XYZ(x, y - 1, z)];
            int f5 = !opaque[XYZ(x, y, z - 1)];
            int f6 = !opaque[XYZ(x, y, z + 1)];
            int total = f1 + f2 + f3 + f4 + f5 + f6;
            if (total == 0) {
                continue;
            }
            if (is_plant[ew]) {
                total = 4;
            }
            faces += total;
        } END_CHUNK_FOR_EACH;

        // generate geometry
        auto result = std::make_shared<ChunkModelResult>(data.position, 10, faces);
        GLfloat * vertices = result->data();
        int offset = 0;

        CHUNK_FOR_EACH(blocks, ex, ey, ez, ew) {
            if (ew <= 0) {
                continue;
            }
            int x = ex - ox;
            int y = ey - oy;
            int z = ez - oz;
            int f1 = !opaque[XYZ(x - 1, y, z)];
            int f2 = !opaque[XYZ(x + 1, y, z)];
            int f3 = !opaque[XYZ(x, y + 1, z)];
            int f4 = !opaque[XYZ(x, y - 1, z)];
            int f5 = !opaque[XYZ(x, y, z - 1)];
            int f6 = !opaque[XYZ(x, y, z + 1)];
            int total = f1 + f2 + f3 + f4 + f5 + f6;
            if (total == 0) {
                continue;
            }
            char neighbors[27] = {0};
            float shades[27] = {0};
            int index = 0;
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dz = -1; dz <= 1; dz++) {
                        neighbors[index] = opaque[XYZ(x + dx, y + dy, z + dz)];
                        shades[index] = 0;
                        if (y + dy <= highest[XZ(x + dx, z + dz)]) {
                            for (int oy = 0; oy < 8; oy++) {
                                if (opaque[XYZ(x + dx, y + dy + oy, z + dz)]) {
                                    shades[index] = 1.0 - oy * 0.125;
                                    break;
                                }
                            }
                        }
                        index++;
                    }
                }
            }
            float ao[6][4];
            float light[6][4];
            occlusion(neighbors, shades, ao, light);
            if (is_plant[ew]) {
                total = 4;
                float min_ao = 1;
                float max_light = 0;
                for (int a = 0; a < 6; a++) {
                    for (int b = 0; b < 4; b++) {
                        min_ao = std::min(min_ao, ao[a][b]);
                    }
                }
                float rotation = simplex2(ex, ez, 4, 0.5, 2) * 360;
                make_plant(vertices + offset, min_ao, 0,
                           ex, ey, ez, 0.5, ew, rotation, block_data.blocks);
            }
            else {
                make_cube(vertices + offset, ao, light,
                          f1, f2, f3, f4, f5, f6,
                          ex, ey, ez, 0.5, ew, block_data.blocks);
            }
            offset += total * 60;
        } END_CHUNK_FOR_EACH;

        free(opaque);
        free(highest);
        return result;
    }
};

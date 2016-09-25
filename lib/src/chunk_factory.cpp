#include <thread>
#include "chunk_factory.h"
#include "util.h"
#include "cube.h"

#define WORKERS 2

namespace konstructs {
    static Vector3i BELOW(0, 0, -1);
    static Vector3i ABOVE(0, 0, 1);
    static Vector3i LEFT(-1, 0, 0);
    static Vector3i RIGHT(1, 0, 0);
    static Vector3i FRONT(0, -1, 0);
    static Vector3i BACK(0, 1, 0);
    static Vector3i ABOVE_LEFT(-1, 0, 1);
    static Vector3i ABOVE_RIGHT(1, 0, 1);
    static Vector3i ABOVE_FRONT(0, -1, 1);
    static Vector3i ABOVE_BACK(0, 1, 1);
    static Vector3i ABOVE_LEFT_FRONT(-1, -1, 1);
    static Vector3i ABOVE_RIGHT_FRONT(1, -1, 1);
    static Vector3i ABOVE_LEFT_BACK(-1, 1, 1);
    static Vector3i ABOVE_RIGHT_BACK(1, 1, 1);
    static Vector3i LEFT_FRONT(-1, -1, 0);
    static Vector3i RIGHT_FRONT(1, -1, 0);
    static Vector3i LEFT_BACK(-1, 1, 0);
    static Vector3i RIGHT_BACK(1, 1, 0);

    ChunkModelResult::ChunkModelResult(const Vector3i _position, const int components,
                                       const int _faces):
        position(_position), size(6 * components * _faces), faces(_faces) {
        mData = new GLuint[size];
    }

    ChunkModelResult::~ChunkModelResult() {
        delete[] mData;
    }

    GLuint *ChunkModelResult::data() {
        return mData;
    }

    ChunkModelFactory::ChunkModelFactory(const BlockTypeInfo &block_data) :
        block_data(block_data),
        processed(0),
        empty(0),
        created(0) {
        for(int i = 0; i < WORKERS; i++) {
            new std::thread(&ChunkModelFactory::worker, this);
        }
    }

    int ChunkModelFactory::waiting() {
        std::lock_guard<std::mutex> lock(mutex);
        return chunks.size();
    }

    int ChunkModelFactory::total() {
        std::lock_guard<std::mutex> lock(mutex);
        return processed;
    }

    int ChunkModelFactory::total_empty() {
        std::lock_guard<std::mutex> lock(mutex);
        return empty;
    }

    int ChunkModelFactory::total_created() {
        std::lock_guard<std::mutex> lock(mutex);
        return created;
    }

    void ChunkModelFactory::create_models(const std::vector<Vector3i> &positions,
                                          const World &world) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            for(auto position: positions) {
                for(auto m : adjacent(position, world)) {
                    chunks.insert(m.position);
                    model_data.erase(m.position);
                    model_data.insert({m.position, m});
                }
            }
        }
        chunks_condition.notify_all();
    }

    std::vector<ChunkModelData> adjacent(const Vector3i position, const World &world) {
        std::vector<ChunkModelData> adj;
        adj.reserve(7);
        adj.push_back(create_model_data(position, world));
        if(world.find(position + BELOW) != world.end())
            adj.push_back(create_model_data(position + BELOW, world));
        if(world.find(position + ABOVE) != world.end())
            adj.push_back(create_model_data(position + ABOVE, world));
        if(world.find(position + LEFT) != world.end())
            adj.push_back(create_model_data(position + LEFT, world));
        if(world.find(position + RIGHT) != world.end())
            adj.push_back(create_model_data(position + RIGHT, world));
        if(world.find(position + FRONT) != world.end())
            adj.push_back(create_model_data(position + FRONT, world));
        if(world.find(position + BACK) != world.end())
            adj.push_back(create_model_data(position + BACK, world));
        return adj;
    }

    const std::shared_ptr<ChunkData>  get_chunk(const Vector3i &position,
                                                const World &world) {
        auto chunk = world.chunk_opt(position);

        if(chunk) {
            return chunk.value();
        } else {
            return SOLID_CHUNK;
        }

    }

    const ChunkModelData create_model_data(const Vector3i &position,
                                           const World &world) {
        const ChunkModelData data = {
            position,
            get_chunk(position + BELOW, world),
            get_chunk(position + ABOVE, world),
            get_chunk(position + LEFT, world),
            get_chunk(position + RIGHT, world),
            get_chunk(position + FRONT, world),
            get_chunk(position + BACK, world),
            get_chunk(position + ABOVE_LEFT, world),
            get_chunk(position + ABOVE_RIGHT, world),
            get_chunk(position + ABOVE_FRONT, world),
            get_chunk(position + ABOVE_BACK, world),
            get_chunk(position + ABOVE_LEFT_FRONT, world),
            get_chunk(position + ABOVE_RIGHT_FRONT, world),
            get_chunk(position + ABOVE_LEFT_BACK, world),
            get_chunk(position + ABOVE_RIGHT_BACK, world),
            get_chunk(position + LEFT_FRONT, world),
            get_chunk(position + LEFT_BACK, world),
            get_chunk(position + RIGHT_FRONT, world),
            get_chunk(position + RIGHT_BACK, world),
            get_chunk(position, world)
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
            std::unique_lock<std::mutex> ulock(mutex);
            chunks_condition.wait(ulock, [&]{return !chunks.empty();});
            auto it = chunks.begin();
            auto position = *it;
            chunks.erase(it);
            auto itr = model_data.find(position);
            if(itr == model_data.end())
                continue;
            auto data = itr->second;
            model_data.erase(itr);
            ulock.unlock();
            auto result = compute_chunk(data, block_data);
            if(result->size > 0) {
                std::lock_guard<std::mutex> ulock(mutex);
                models.push_back(result);
                created++;
                processed++;
            } else {
                std::lock_guard<std::mutex> ulock(mutex);
                empty++;
                processed++;
            }
        }
    }

#define XZ_SIZE (CHUNK_SIZE * 3 + 2)
#define XZ_LO (CHUNK_SIZE)
#define XZ_HI (CHUNK_SIZE * 2 + 1)
#define XYZ(x, y, z) ((y) * XZ_SIZE * XZ_SIZE + (x) * XZ_SIZE + (z))
#define XZ(x, z) ((x) * XZ_SIZE + (z))

    void occlusion(
                   char neighbors[27], char shades[27],
                   char ao[6][4])
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
        static const char curve[4] = {0, 8, 16, 24};
        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < 4; j++) {
                int corner = neighbors[lookup3[i][j][0]];
                int side1 = neighbors[lookup3[i][j][1]];
                int side2 = neighbors[lookup3[i][j][2]];
                int value = side1 && side2 ? 3 : corner + side1 + side2;
                char shade_sum = 0;
                for (int k = 0; k < 4; k++) {
                    shade_sum += shades[lookup4[i][j][k]];
                }
                char total = curve[value] + shade_sum;
                ao[i][j] = std::min(total, (char)31);
            }
        }
    }

    bool face_visible(int self, int neighbour, const char *is_transparent, const char *state) {
        return is_transparent[neighbour] || (self != neighbour && state[neighbour] == STATE_LIQUID);
    }

    shared_ptr<ChunkModelResult> compute_chunk(const ChunkModelData &data,
                                               const BlockTypeInfo &block_data) {
        std::vector<BlockData> blocks(XZ_SIZE * XZ_SIZE * XZ_SIZE);
        std::vector<char> highest(XZ_SIZE * XZ_SIZE);

        BlockData *above = data.above->blocks;
        BlockData *below = data.below->blocks;
        BlockData *left = data.left->blocks;
        BlockData *right = data.right->blocks;
        BlockData *front = data.front->blocks;
        BlockData *back = data.back->blocks;
        BlockData *above_left = data.above_left->blocks;
        BlockData *above_right = data.above_right->blocks;
        BlockData *above_front = data.above_front->blocks;
        BlockData *above_back = data.above_back->blocks;
        BlockData *above_left_front = data.above_left_front->blocks;
        BlockData *above_right_front = data.above_right_front->blocks;
        BlockData *above_left_back = data.above_left_back->blocks;
        BlockData *above_right_back = data.above_right_back->blocks;
        BlockData *left_front = data.left_front->blocks;
        BlockData *right_front = data.right_front->blocks;
        BlockData *left_back = data.left_back->blocks;
        BlockData *right_back = data.right_back->blocks;

        const char *is_transparent = block_data.is_transparent;
        const char *is_plant = block_data.is_plant;
        const char *state = block_data.state;

        int ox = - CHUNK_SIZE - 1;
        int oy = - CHUNK_SIZE - 1;
        int oz = - CHUNK_SIZE - 1;

        /* Populate the blocks array with the chunk itself */
        const BlockData *self = data.self->blocks;

        CHUNK_FOR_EACH(self, ex, ey, ez, eb) {
            int x = ex - ox;
            int y = ey - oy;
            int z = ez - oz;
            blocks[XYZ(x, y, z)] = eb;
            if (!is_transparent[eb.type]) {
                highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH;

        /* With the six sides of the chunk */

        /* Populate the blocks array with the chunk below */
        CHUNK_FOR_EACH_XZ(below, CHUNK_SIZE - 1, ex, ey, ez, eb) {
            int x = ex - ox;
            int y = ey - CHUNK_SIZE - oy;
            int z = ez - oz;
            blocks[XYZ(x, y, z)] = eb;
            if (!is_transparent[eb.type]) {
                highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH_2D;


        /* Populate the blocks array with the chunk above
         * The shading requires additional 8 blocks
         */
        for(int i = 0; i < 8; i++) {
            CHUNK_FOR_EACH_XZ(above, i, ex, ey, ez, eb) {
                int x = ex - ox;
                int y = ey + CHUNK_SIZE - oy;
                int z = ez - oz;
                blocks[XYZ(x, y, z)] = eb;
                if (!is_transparent[eb.type]) {
                    highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
                }
            } END_CHUNK_FOR_EACH_2D;
        }

        /* Populate the blocks array with the chunk left */
        CHUNK_FOR_EACH_YZ(left, CHUNK_SIZE - 1, ex, ey, ez, eb) {
            int x = ex - CHUNK_SIZE - ox;
            int y = ey - oy;
            int z = ez - oz;
            blocks[XYZ(x, y, z)] = eb;
            if (!is_transparent[eb.type]) {
                highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH_2D;

        /* Populate the blocks array with the chunk right */
        CHUNK_FOR_EACH_YZ(right, 0, ex, ey, ez, eb) {
            int x = ex + CHUNK_SIZE - ox;
            int y = ey - oy;
            int z = ez - oz;
            blocks[XYZ(x, y, z)] = eb;
            if (!is_transparent[eb.type]) {
                highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH_2D;


        /* Populate the blocks array with the chunk front */
        CHUNK_FOR_EACH_XY(front, CHUNK_SIZE - 1, ex, ey, ez, eb) {
            int x = ex - ox;
            int y = ey - oy;
            int z = ez - CHUNK_SIZE - oz;
            blocks[XYZ(x, y, z)] = eb;
            if (!is_transparent[eb.type]) {
                highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH_2D;


        /* Populate the blocks array with the chunk back */
        CHUNK_FOR_EACH_XY(back, 0, ex, ey, ez, eb) {
            int x = ex - ox;
            int y = ey - oy;
            int z = ez + CHUNK_SIZE - oz;
            blocks[XYZ(x, y, z)] = eb;
            if (!is_transparent[eb.type]) {
                highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH_2D;

        /* Populate the corner cases above
         * Shading yet again requires 8 additional blocks
         */

        for(int i = 0; i < 8; i++) {
            /* Populate the blocks array with the chunk above-left */
            CHUNK_FOR_EACH_Z(above_left, CHUNK_SIZE - 1, i, ex, ey, ez, eb) {
                int x = ex - CHUNK_SIZE - ox;
                int y = ey + CHUNK_SIZE - oy;
                int z = ez - oz;
                blocks[XYZ(x, y, z)] = eb;
                if (!is_transparent[eb.type]) {
                    highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
                }
            } END_CHUNK_FOR_EACH_1D;

            /* Populate the blocks array with the chunk above-right */
            CHUNK_FOR_EACH_Z(above_right, 0, i, ex, ey, ez, eb) {
                int x = ex + CHUNK_SIZE - ox;
                int y = ey + CHUNK_SIZE - oy;
                int z = ez - oz;
                blocks[XYZ(x, y, z)] = eb;
                if (!is_transparent[eb.type]) {
                    highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
                }
            } END_CHUNK_FOR_EACH_1D;

            /* Populate the blocks array with the chunk above-front */
            CHUNK_FOR_EACH_X(above_front, CHUNK_SIZE - 1, i, ex, ey, ez, eb) {
                int x = ex - ox;
                int y = ey + CHUNK_SIZE - oy;
                int z = ez - CHUNK_SIZE - oz;
                blocks[XYZ(x, y, z)] = eb;
                if (!is_transparent[eb.type]) {
                    highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
                }
            } END_CHUNK_FOR_EACH_1D;
            /* Populate the blocks array with the chunk above-back */
            CHUNK_FOR_EACH_X(above_back, 0, i, ex, ey, ez, eb) {
                int x = ex - ox;
                int y = ey + CHUNK_SIZE - oy;
                int z = ez + CHUNK_SIZE - oz;
                blocks[XYZ(x, y, z)] = eb;
                if (!is_transparent[eb.type]) {
                    highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
                }
            } END_CHUNK_FOR_EACH_1D;

            /* Populate the blocks array with the block above-left-front */
            {
                int ex = CHUNK_SIZE - 1;
                int ey = i;
                int ez = CHUNK_SIZE - 1;
                int x = ex - CHUNK_SIZE - ox;
                int y = ey + CHUNK_SIZE - oy;
                int z = ez - CHUNK_SIZE - oz;
                BlockData eb = above_left_front[ex+ey*CHUNK_SIZE+ez*CHUNK_SIZE*CHUNK_SIZE];
                blocks[XYZ(x, y, z)] = eb;
                if (!is_transparent[eb.type]) {
                    highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
                }
            }

            /* Populate the blocks array with the block above-right-front */
            {
                int ex = 0;
                int ey = i;
                int ez = CHUNK_SIZE - 1;
                int x = ex + CHUNK_SIZE - ox;
                int y = ey + CHUNK_SIZE - oy;
                int z = ez - CHUNK_SIZE - oz;
                BlockData eb = above_right_front[ex+ey*CHUNK_SIZE+ez*CHUNK_SIZE*CHUNK_SIZE];
                blocks[XYZ(x, y, z)] = eb;
                if (!is_transparent[eb.type]) {
                    highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
                }
            }

            /* Populate the blocks array with the block above-left-back */
            {
                int ex = CHUNK_SIZE - 1;
                int ey = i;
                int ez = 0;
                int x = ex - CHUNK_SIZE - ox;
                int y = ey + CHUNK_SIZE - oy;
                int z = ez + CHUNK_SIZE - oz;
                BlockData eb = above_left_back[ex+ey*CHUNK_SIZE+ez*CHUNK_SIZE*CHUNK_SIZE];
                blocks[XYZ(x, y, z)] = eb;
                if (!is_transparent[eb.type]) {
                    highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
                }
            }

            /* Populate the blocks array with the block above-right-back */
            {
                int ex = 0;
                int ey = i;
                int ez = 0;
                int x = ex + CHUNK_SIZE - ox;
                int y = ey + CHUNK_SIZE - oy;
                int z = ez + CHUNK_SIZE - oz;
                BlockData eb = above_right_back[ex+ey*CHUNK_SIZE+ez*CHUNK_SIZE*CHUNK_SIZE];
                blocks[XYZ(x, y, z)] = eb;
                if (!is_transparent[eb.type]) {
                    highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
                }
            }

        }

        /* Populate the corner cases on the same level */

        /* Populate the blocks array with the chunk left-front */
        CHUNK_FOR_EACH_Y(left_front, CHUNK_SIZE - 1, CHUNK_SIZE - 1, ex, ey, ez, eb) {
            int x = ex - CHUNK_SIZE - ox;
            int y = ey - oy;
            int z = ez - CHUNK_SIZE - oz;
            blocks[XYZ(x, y, z)] = eb;
            if (!is_transparent[eb.type]) {
                highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH_1D;

        /* Populate the blocks array with the chunk left-back */
        CHUNK_FOR_EACH_Y(left_back, CHUNK_SIZE - 1, 0, ex, ey, ez, eb) {
            int x = ex - CHUNK_SIZE - ox;
            int y = ey - oy;
            int z = ez + CHUNK_SIZE - oz;
            blocks[XYZ(x, y, z)] = eb;
            if (!is_transparent[eb.type]) {
                highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH_1D;

        /* Populate the blocks array with the chunk right-front */
        CHUNK_FOR_EACH_Y(right_front, 0, CHUNK_SIZE - 1, ex, ey, ez, eb) {
            int x = ex + CHUNK_SIZE - ox;
            int y = ey - oy;
            int z = ez - CHUNK_SIZE - oz;
            blocks[XYZ(x, y, z)] = eb;
            if (!is_transparent[eb.type]) {
                highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH_1D;

        /* Populate the blocks array with the chunk right-back */
        CHUNK_FOR_EACH_Y(right_back, 0, 0, ex, ey, ez, eb) {
            int x = ex + CHUNK_SIZE - ox;
            int y = ey - oy;
            int z = ez + CHUNK_SIZE - oz;
            blocks[XYZ(x, y, z)] = eb;
            if (!is_transparent[eb.type]) {
                highest[XZ(x, z)] = std::max((int)highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH_1D;


        // count exposed faces
        int faces = 0;
        CHUNK_FOR_EACH(self, ex, ey, ez, eb) {
            if (eb.type <= 0) {
                continue;
            }
            int x = ex - ox;
            int y = ey - oy;
            int z = ez - oz;
            int f1 = face_visible(eb.type, blocks[XYZ(x - 1, y, z)].type, is_transparent, state);
            int f2 = face_visible(eb.type, blocks[XYZ(x + 1, y, z)].type, is_transparent, state);
            int f3 = face_visible(eb.type, blocks[XYZ(x, y + 1, z)].type, is_transparent, state);
            int f4 = face_visible(eb.type, blocks[XYZ(x, y - 1, z)].type, is_transparent, state);
            int f5 = face_visible(eb.type, blocks[XYZ(x, y, z - 1)].type, is_transparent, state);
            int f6 = face_visible(eb.type, blocks[XYZ(x, y, z + 1)].type, is_transparent, state);
            int total = f1 + f2 + f3 + f4 + f5 + f6;

            if (total == 0) {
                continue;
            }

            if (is_plant[eb.type]) {
                total = 4;
            }

            faces += total;
        } END_CHUNK_FOR_EACH;

        // generate geometry
        auto result = std::make_shared<ChunkModelResult>(data.position, 2, faces);
        GLuint * vertices = result->data();
        int offset = 0;

        CHUNK_FOR_EACH(self, ex, ey, ez, eb) {
            if (eb.type <= 0) {
                continue;
            }
            int x = ex - ox;
            int y = ey - oy;
            int z = ez - oz;
            int f1 = face_visible(eb.type, blocks[XYZ(x - 1, y, z)].type, is_transparent, state);
            int f2 = face_visible(eb.type, blocks[XYZ(x + 1, y, z)].type, is_transparent, state);
            int f3 = face_visible(eb.type, blocks[XYZ(x, y + 1, z)].type, is_transparent, state);
            int f4 = face_visible(eb.type, blocks[XYZ(x, y - 1, z)].type, is_transparent, state);
            int f5 = face_visible(eb.type, blocks[XYZ(x, y, z - 1)].type, is_transparent, state);
            int f6 = face_visible(eb.type, blocks[XYZ(x, y, z + 1)].type, is_transparent, state);
            int total = f1 + f2 + f3 + f4 + f5 + f6;
            if (total == 0) {
                continue;
            }
            char neighbors[27] = {0};
            char shades[27] = {0};
            int index = 0;
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dz = -1; dz <= 1; dz++) {
                        neighbors[index] = !is_transparent[blocks[XYZ(x + dx, y + dy, z + dz)].type];
                        shades[index] = 0;
                        if (y + dy <= highest[XZ(x + dx, z + dz)]) {
                            for (int oy = 0; oy < 8; oy++) {
                                if (!is_transparent[blocks[XYZ(x + dx, y + dy + oy, z + dz)].type]) {
                                    shades[index] = 8 - oy;
                                    break;
                                }
                            }
                        }
                        index++;
                    }
                }
            }
            char ao[6][4];
            occlusion(neighbors, shades, ao);
            if (is_plant[eb.type]) {
                total = 4;
                char min_ao = 1;
                for (int a = 0; a < 6; a++) {
                    for (int b = 0; b < 4; b++) {
                        min_ao = std::min(min_ao, ao[a][b]);
                    }
                }
                make_plant(vertices + offset, min_ao,
                           ex, ey, ez, eb, block_data.blocks);
            }
            else {
                int damage = (int)(8.0f - ((float)eb.health / (float)(MAX_HEALTH + 1)) * 8.0f);
                make_cube2(vertices + offset, ao,
                           f1, f2, f3, f4, f5, f6,
                           ex, ey, ez, eb, damage, block_data.blocks);
            }
            offset += total * 12;
        } END_CHUNK_FOR_EACH;

        return result;
    }
};

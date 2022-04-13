#ifndef _game_h_
#define _game_h_

#define MAX_CHUNKS 8192
#define MAX_PLAYERS 128
#define WORKERS 4
#define MAX_TEXT_LENGTH 256
#define MAX_NAME_LENGTH 32
#define MAX_PATH_LENGTH 256
#define MAX_ADDR_LENGTH 256

#define ALIGN_LEFT 0
#define ALIGN_CENTER 1
#define ALIGN_RIGHT 2

#define MODE_OFFLINE 0
#define MODE_ONLINE 1

#define WORKER_IDLE 0
#define WORKER_BUSY 1
#define WORKER_DONE 2

// World chunk data (big area of blocks)
// - map: block hash map
// - lights: light hash map
// - signs: signs in this chunk
// - p: chunk x coordinate
// - q: chunk z coordinate
// - faces: number of faces
// - sign_faces: number of sign face
// - dirty: dirty flag
// - miny: minimum Y value held by any block
// - maxy: maximum Y value held by any block
// - buffer:
// - sign_buffer:
typedef struct {
    Map map;
    Map lights;
    SignList signs;
    int p;
    int q;
    int faces;
    int sign_faces;
    int dirty;
    int miny;
    int maxy;
    GLuint buffer;
    GLuint sign_buffer;
} Chunk;

// A single item that a Worker can work on
// - p: chunk x coordinate
// - q: chunk y coordinate
// - load
// - block_maps
// - light_maps
// - miny
// - maxy
// - faces
// - data
typedef struct {
    int p;
    int q;
    int load;
    Map *block_maps[3][3];
    Map *light_maps[3][3];
    int miny;
    int maxy;
    int faces;
    GLfloat *data;
} WorkerItem;

// Worker data (for multi-threading)
// - index:
// - state:
// - thrd: thread
// - mtx: mutex
// - cnd: condition variable
// - item:
typedef struct {
    int index;
    int state;
    thrd_t thrd;
    mtx_t mtx;
    cnd_t cnd;
    WorkerItem item;
} Worker;

// Block data
// - x: x position
// - y: y position
// - z: z position
// - w: block id / value
typedef struct {
    int x;
    int y;
    int z;
    int w;
} Block;

// State for a player
// - x: x position (player feet level)
// - y: y position (player feet level)
// - z: z position (player feet level)
// - rx: rotation x
// - ry: rotation y
// - t: keep track of time, for interpolation
typedef struct {
    float x; 
    float y;
    float z;
    float rx;
    float ry;
    float t; 
    int flying;
} State;

// Player
// - id
// - name: name string buffer
// - state: current player position state
// - state1: another state, for interpolation
// - state2: another state, for interpolation
// - buffer: some GL buffer id (?)
typedef struct {
    int id;
    char name[MAX_NAME_LENGTH];
    State state;
    State state1;
    State state2;
    GLuint buffer;
} Player;

// OpenGL attribute data
// - program:
// - position:
// - normal:
// - uv:
// - matrix:
// - sampler:
// - camera:
// - timer:
// - extra1:
// - extra2:
// - extra3:
// - extra4:
typedef struct {
    GLuint program;
    GLuint position;
    GLuint normal;
    GLuint uv;
    GLuint matrix;
    GLuint sampler;
    GLuint camera;
    GLuint timer;
    GLuint extra1;
    GLuint extra2;
    GLuint extra3;
    GLuint extra4;
} Attrib;

// Program state model
// - window:
// - workers:
// - chunks:
// - chunk_count:
// - create_radius:
// - render_radius:
// - delete_radius:
// - sign_radius:
// - players:
// - player_count:
// - typing:
// - typing_buffer:
// - message_index:
// - messages:
// - width:
// - height:
// - observe1:
// - observe2:
// - flying:
// - item_index: current selected block to place next
// - scale:
// - ortho:
// - fov:
// - suppress_char:
// - mode:
// - mode_changed:
// - db_path:
// - server_addr:
// - server_port:
// - day_length:
// - time_changed:
// - block0:
// - block1:
// - copy0:
// - copy1:
typedef struct {
    GLFWwindow *window;
    Worker workers[WORKERS];
    Chunk chunks[MAX_CHUNKS];
    int chunk_count;
    int create_radius;
    int render_radius;
    int delete_radius;
    int sign_radius;
    Player players[MAX_PLAYERS];
    int player_count;
    int typing;
    char typing_buffer[MAX_TEXT_LENGTH];
    int message_index;
    char messages[MAX_MESSAGES][MAX_TEXT_LENGTH];
    int width;
    int height;
    int observe1;
    int observe2;
    int item_index;
    int scale;
    int ortho;
    float fov;
    int suppress_char;
    int mode;
    int mode_changed;
    char db_path[MAX_PATH_LENGTH];
    char server_addr[MAX_ADDR_LENGTH];
    int server_port;
    int day_length;
    int time_changed;
    Block block0;
    Block block1;
    Block copy0;
    Block copy1;
} Model;

// Game model pointer
extern Model *g;

int chunked(float x);
float time_of_day();
float get_daylight();
int get_scale_factor();
void get_sight_vector(float rx, float ry, float *vx, float *vy, float *vz);
void get_motion_vector(
        int flying, int sz, int sx, float rx, float ry, float *vx, float *vy,
        float *vz);
GLuint gen_crosshair_buffer();
GLuint gen_wireframe_buffer(float x, float y, float z, float n);
GLuint gen_sky_buffer();
GLuint gen_cube_buffer(float x, float y, float z, float n, int w);
GLuint gen_plant_buffer(float x, float y, float z, float n, int w);
GLuint gen_player_buffer(float x, float y, float z, float rx, float ry);
GLuint gen_text_buffer(float x, float y, float n, char *text);
void draw_triangles_3d_ao(Attrib *attrib, GLuint buffer, int count);
void draw_triangles_3d_text(Attrib *attrib, GLuint buffer, int count);
void draw_triangles_3d(Attrib *attrib, GLuint buffer, int count);
void draw_triangles_2d(Attrib *attrib, GLuint buffer, int count);
void draw_lines(Attrib *attrib, GLuint buffer, int components, int count);
void draw_chunk(Attrib *attrib, Chunk *chunk);
void draw_item(Attrib *attrib, GLuint buffer, int count);
void draw_text(Attrib *attrib, GLuint buffer, int length);
void draw_signs(Attrib *attrib, Chunk *chunk);
void draw_sign(Attrib *attrib, GLuint buffer, int length);
void draw_cube(Attrib *attrib, GLuint buffer);
void draw_plant(Attrib *attrib, GLuint buffer);
void draw_player(Attrib *attrib, Player *player);
Player *find_player(int id);
void update_player(
        Player *player, float x, float y, float z, float rx, float ry,
        int interpolate); void interpolate_player(Player *player);
void delete_player(int id);
void delete_all_players();
float player_player_distance(Player *p1, Player *p2);
float player_crosshair_distance(Player *p1, Player *p2);
Player *player_crosshair(Player *player);
Chunk *find_chunk(int p, int q);
int chunk_distance(Chunk *chunk, int p, int q);
int chunk_visible(float planes[6][4], int p, int q, int miny, int maxy);
int highest_block(float x, float z);
int _hit_test(
        Map *map, float max_distance, int previous, float x, float y, float z,
        float vx, float vy, float vz, int *hx, int *hy, int *hz);
int hit_test(
        int previous, float x, float y, float z, float rx, float ry,
        int *bx, int *by, int *bz);
int hit_test_face(Player *player, int *x, int *y, int *z, int *face);
int collide(int height, float *x, float *y, float *z);
int player_intersects_block(
        int height, float x, float y, float z, int hx, int hy, int hz);
int _gen_sign_buffer(
        GLfloat *data, float x, float y, float z, int face, const char *text);
void gen_sign_buffer(Chunk *chunk);
int has_lights(Chunk *chunk);
void dirty_chunk(Chunk *chunk);
void occlusion(
        char neighbors[27], char lights[27], float shades[27], float ao[6][4],
        float light[6][4]);
void light_fill(
        char *opaque, char *light, int x, int y, int z, int w, int force);
void compute_chunk(WorkerItem *item);
void generate_chunk(Chunk *chunk, WorkerItem *item);
void gen_chunk_buffer(Chunk *chunk);
void map_set_func(int x, int y, int z, int w, void *arg);
void load_chunk(WorkerItem *item);
void request_chunk(int p, int q);
void init_chunk(Chunk *chunk, int p, int q);
void create_chunk(Chunk *chunk, int p, int q);
void delete_chunks();
void delete_all_chunks();
void check_workers();
void force_chunks(Player *player);
void ensure_chunks_worker(Player *player, Worker *worker);
void ensure_chunks(Player *player);
int worker_run(void *arg);
void unset_sign(int x, int y, int z);
void unset_sign_face(int x, int y, int z, int face);
void _set_sign(
        int p, int q, int x, int y, int z, int face, const char *text,
        int dirty);
void set_sign(int x, int y, int z, int face, const char *text);
void toggle_light(int x, int y, int z);
void set_light(int p, int q, int x, int y, int z, int w);
void _set_block(int p, int q, int x, int y, int z, int w, int dirty);
void set_block(int x, int y, int z, int w);
void record_block(int x, int y, int z, int w);
int get_block(int x, int y, int z);
void builder_block(int x, int y, int z, int w);
int render_chunks(Attrib *attrib, Player *player);
void render_signs(Attrib *attrib, Player *player);
void render_sign(Attrib *attrib, Player *player);
void render_players(Attrib *attrib, Player *player);
void render_sky(Attrib *attrib, Player *player, GLuint buffer);
void render_wireframe(Attrib *attrib, Player *player);
void render_crosshairs(Attrib *attrib);
void render_item(Attrib *attrib);
void render_text(
        Attrib *attrib, int justify, float x, float y, float n, char *text);
void add_message(const char *text);
void login();
void copy();
void paste();
void array(Block *b1, Block *b2, int xc, int yc, int zc);
void cube(Block *b1, Block *b2, int fill);
void sphere(Block *center, int radius, int fill, int fx, int fy, int fz);
void cylinder(Block *b1, Block *b2, int radius, int fill);
void tree(Block *block);
void parse_command(const char *buffer, int forward);
void on_light();
void on_left_click();
void on_right_click();
void on_middle_click();
void on_key(GLFWwindow *window, int key, int scancode, int action, int mods);
void on_char(GLFWwindow *window, unsigned int u);
void on_scroll(GLFWwindow *window, double xdelta, double ydelta);
void on_mouse_button(GLFWwindow *window, int button, int action, int mods);
void create_window();
void handle_mouse_input();
void handle_movement(double dt);
void parse_buffer(char *buffer);
void reset_model();

#endif /*_game_h_*/


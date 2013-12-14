#ifndef _config_h_
#define _config_h_

// app parameters
#define VSYNC 1
#define SHOW_FPS 0
#define SCROLL_THRESHOLD 0.1
#define MAX_MESSAGES 4
#define DB_PATH "craft.db"

// key bindings
#define CRAFT_KEY_QUIT 'Q'
#define CRAFT_KEY_JUMP GLFW_KEY_SPACE
#define CRAFT_KEY_FLY GLFW_KEY_TAB
#define CRAFT_KEY_TELEPORT 'P'
#define CRAFT_KEY_BLOCK_TYPE 'E'
#define CRAFT_KEY_ZOOM GLFW_KEY_LEFT_SHIFT
#define CRAFT_KEY_ORTHO 'F'
#define CRAFT_KEY_CHAT 't'
#define CRAFT_KEY_COMMAND '/'

// keys for moving along axes
#define CRAFT_KEY_XM 'Z'
#define CRAFT_KEY_XP 'X'
#define CRAFT_KEY_YM 'C'
#define CRAFT_KEY_YP 'V'
#define CRAFT_KEY_ZM 'B'
#define CRAFT_KEY_ZP 'N'

// advanced parameters
#define CHUNK_SIZE 32

typedef struct {
    int fullscreen;
    int width;
    int height;
    unsigned char forward;
    unsigned char backward;
    unsigned char left;
    unsigned char right;
} configuration;

int configure(configuration *config);

#endif

#ifndef _config_h_
#define _config_h_

// app parameters
#define DEBUG 0
#define FULLSCREEN 0
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define VSYNC 1
#define SCROLL_THRESHOLD 0.1
#define MAX_MESSAGES 4
#define DB_PATH "craft.db"
#define USE_CACHE 1
#define DAY_LENGTH 300
#define INVERT_MOUSE 0

// rendering options
#define SHOW_PLANTS 1
#define SHOW_CLOUDS 1
#define SHOW_TREES 1
#define SHOW_ITEM 1
#define SHOW_CROSSHAIRS 1
#define SHOW_WIREFRAME 1
#define SHOW_INFO_TEXT 1
#define SHOW_CHAT_TEXT 1
#define SHOW_PLAYER_NAMES 1

// key bindings
#define CRAFT_KEY_QUIT 'Q'
#define CRAFT_KEY_FORWARD 'W'
#define CRAFT_KEY_BACKWARD 'S'
#define CRAFT_KEY_LEFT 'A'
#define CRAFT_KEY_RIGHT 'D'
#define CRAFT_KEY_JUMP GLFW_KEY_SPACE
#define CRAFT_KEY_FLY GLFW_KEY_TAB
#define CRAFT_KEY_OBSERVE 'O'
#define CRAFT_KEY_OBSERVE_INSET 'P'
#define CRAFT_KEY_ITEM_NEXT 'E'
#define CRAFT_KEY_ITEM_PREV 'R'
#define CRAFT_KEY_ZOOM GLFW_KEY_LEFT_SHIFT
#define CRAFT_KEY_ORTHO 'F'
#define CRAFT_KEY_CHAT 't'
#define CRAFT_KEY_COMMAND '/'
#define CRAFT_KEY_SIGN '`'

// keys for moving along axes
#define CRAFT_KEY_XM 'Z'
#define CRAFT_KEY_XP 'X'
#define CRAFT_KEY_YM 'C'
#define CRAFT_KEY_YP 'V'
#define CRAFT_KEY_ZM 'B'
#define CRAFT_KEY_ZP 'N'

// advanced parameters
#define CREATE_CHUNK_RADIUS 6
#define RENDER_CHUNK_RADIUS 6
#define RENDER_SIGN_RADIUS 3
#define DELETE_CHUNK_RADIUS 12
#define CHUNK_SIZE 32
#define COMMIT_INTERVAL 5

#endif

#ifndef _keys_h_
#define _keys_h_

// display parameters:
#define WINDOW_WIDTH    1024
#define WINDOW_HEIGHT   768
#define FULLSCREEN      0
#define VSYNC           1
#define SHOW_FPS        0

// key bindings:
#define CRAFT_KEY_QUIT  'Q'
#define CRAFT_KEY_FWD   'W'
#define CRAFT_KEY_BACK  'S'
#define CRAFT_KEY_LEFT  'A'
#define CRAFT_KEY_RIGHT 'D'
#define CRAFT_KEY_JUMP  GLFW_KEY_SPACE
#define CRAFT_KEY_FLY   GLFW_KEY_TAB
// teleport:
#define CRAFT_KEY_TELE  'P'
// cycle block type:
#define CRAFT_KEY_BTYPE 'E'
#define CRAFT_KEY_ZOOM  GLFW_KEY_LEFT_SHIFT
// orthogonal view:
#define CRAFT_KEY_ORTHO 'F'

// keys for moving along axes:
#define CRAFT_KEY_XM    'Z'
#define CRAFT_KEY_XP    'X'
#define CRAFT_KEY_YM    'C'
#define CRAFT_KEY_YP    'V'
#define CRAFT_KEY_ZM    'B'
#define CRAFT_KEY_ZP    'N'

// chat:
#define CRAFT_KEY_CHAT  116 // 't'

#endif

#ifndef _config_h_
#define _config_h_

// app parameters
#define SCROLL_THRESHOLD 0.1
#define MAX_MESSAGES 4
#define DB_PATH "craft.db"

// advanced parameters
#define CHUNK_SIZE 32

typedef struct {
    // Video.
    int fullscreen;
    int width;
    int height;
    int vsync;
    int show_fps;
    // Controls.
    int forward;
    int backward;
    int strafe_left;
    int strafe_right;
    int jump;
    int fly;
    int teleport;
    int cycle_block;
    int ortho_view;
    int zoom;
    int quit;
    int x_inc;
    int x_dec;
    int y_inc;
    int y_dec;
    int z_inc;
    int z_dec;
    int chat;
    int command;
} configuration;

int configure(configuration *config);

#endif

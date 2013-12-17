#ifndef _config_h_
#define _config_h_

// app parameters
#define SCROLL_THRESHOLD 0.1
#define MAX_MESSAGES 4
#define DB_PATH "craft.db"
#define USE_CACHE 1

// advanced parameters
#define CHUNK_SIZE 32
#define COMMIT_INTERVAL 5

typedef struct {
    // Video.
    int fullscreen;
    int width;
    int height;
    int vsync;
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

/* Reads settings into *config. First default-initializes all keys in *config,
 * then searches for the configuration file "profile.ini". If the file exists
 * it is parsed and corresponding keys in *config are replaced. It is not an
 * error that the file does not exist but it is an error that the file is
 * malformed.
 * Returns 0 on success, 1 otherwise.
 */
int configure(configuration *config);

#endif

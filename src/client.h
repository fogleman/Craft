#ifndef _client_h_
#define _client_h_

#define DEFAULT_PORT 4080

#define SHOWERROR(ErrMsg) { char aBuf[256]; snprintf(aBuf, 256, "At '%s:%d' in function '%s' occurred error '%s'",__FILE__,__LINE__,__FUNCTION__,ErrMsg); perror(aBuf); };

typedef struct {
    int size;
    char *payload;
} Packet;

void client_enable();
void client_disable();
int get_client_enabled();
int check_server(char *server);
void client_connect(char *hostname, int port);
void client_start();
void client_stop();
void client_send(char *data);
Packet client_recv();
void client_version(int version, char *nick, char *hash);
void client_login(const char *username, const char *identity_token);
void client_position(float x, float y, float z, float rx, float ry);
void client_chunk(int amount);
void client_block(int x, int y, int z, int w);
void client_konstruct();
void client_click_inventory(int item);
void client_close_inventory();
void client_light(int x, int y, int z, int w);
void client_sign(int x, int y, int z, int face, const char *text);
void client_talk(const char *text);
void client_inventory_select(int pos);
void click_at(int hit, int button, int x, int y, int z);

#endif

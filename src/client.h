#ifndef _client_h_
#define _client_h_

#define DEFAULT_PORT 4080

typedef struct {
    int size;
    char *payload;
} Packet;

void client_enable();
void client_disable();
int get_client_enabled();
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
void client_light(int x, int y, int z, int w);
void client_sign(int x, int y, int z, int face, const char *text);
void client_talk(const char *text);
void client_inventory();
void client_inventory_select(int pos);
void click_at(int button, int x, int y, int z);

#endif

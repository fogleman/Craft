#ifndef _client_h_
#define _client_h_

#define DEFAULT_PORT 4080

void client_enable();
void client_disable();
int get_client_enabled();
void client_connect(char *hostname, int port);
void client_start();
void client_stop();
void client_send(char *data);
char *client_recv();
void client_version(int version);
void client_login(const char *username, const char *identity_token);
void client_position(float x, float y, float z, float rx, float ry);
void client_chunk(int p, int q, int key);
void client_block(int x, int y, int z, int w);
void client_light(int x, int y, int z, int w);
void client_sign(int x, int y, int z, int face, const char *text);
void client_talk(const char *text);

#endif

#define DEFAULT_PORT 4080

void client_enable();
void client_disable();
int get_client_enabled();
void client_send(char *data);
void client_chunk(int p, int q);
void client_block(int p, int q, int x, int y, int z, int w);
int client_recv(char *data);
void client_connect(char *hostname, int port);
void client_start();
void client_stop();

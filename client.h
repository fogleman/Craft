void client_enable();
void client_disable();
int get_client_enabled();
void client_connect(char *hostname, int port);
void client_send(char *data);
int client_recv(char *data);
void client_start();
void client_stop();

#ifdef _WIN32
    #include <winsock2.h>
    #include <windows.h>
    #define close closesocket
    #define sleep Sleep
#else
    #include <netdb.h>
    #include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include "tinycthread.h"

#define RECV_SIZE 256*256*256
#define HEADER_SIZE 4

static int client_enabled = 0;
static int running = 0;
static int sd = 0;
static int bytes_sent = 0;
static int bytes_received = 0;
static char *packet_buffer = 0;
static int packet_buffer_size = 0;
static thrd_t recv_thread;
static mtx_t mutex;

void client_enable() {
    client_enabled = 1;
}

void client_disable() {
    client_enabled = 0;
}

int get_client_enabled() {
    return client_enabled;
}

int client_sendall(int sd, char *data, int length) {
    if (!client_enabled) {
        return 0;
    }
    int count = 0;
    while (count < length) {
        int n = send(sd, data + count, length, 0);
        if (n == -1) {
            return -1;
        }
        count += n;
        length -= n;
        bytes_sent += n;
    }
    return 0;
}

void client_send(char *data) {
    if (!client_enabled) {
        return;
    }
    int length = strlen(data);
    int header_size = htonl(length);
    if (client_sendall(sd, (char*)&header_size, sizeof(header_size)) == -1) {
        perror("client_sendall");
        exit(1);
    }
    if (client_sendall(sd, data, length) == -1) {
        perror("client_sendall");
        exit(1);
    }
}

void client_version(int version, char *nick, char *hash) {
    if (!client_enabled) {
        return;
    }
    char buffer[1024];
    snprintf(buffer, 1024, "V,%d,%s,%s", version, nick, hash);
    client_send(buffer);
}

void client_login(const char *username, const char *identity_token) {
    if (!client_enabled) {
        return;
    }
    char buffer[1024];
    snprintf(buffer, 1024, "A,%s,%s", username, identity_token);
    client_send(buffer);
}

void client_position(float x, float y, float z, float rx, float ry) {
    if (!client_enabled) {
        return;
    }
    static float px, py, pz, prx, pry = 0;
    float distance =
        (px - x) * (px - x) +
        (py - y) * (py - y) +
        (pz - z) * (pz - z) +
        (prx - rx) * (prx - rx) +
        (pry - ry) * (pry - ry);
    if (distance < 0.0001) {
        return;
    }
    px = x; py = y; pz = z; prx = rx; pry = ry;
    char buffer[1024];
    snprintf(buffer, 1024, "P,%.2f,%.2f,%.2f,%.2f,%.2f", x, y, z, rx, ry);
    client_send(buffer);
}

void client_inventory() {
    if (!client_enabled) {
        return;
    }
    client_send("I");
}

void client_inventory_select(int pos) {
    if (!client_enabled) {
        return;
    }
    char buffer[16];
    snprintf(buffer, 16, "A,%d", pos);
    client_send(buffer);
}

void click_at(int x, int y, int z, int button) {
    if (!client_enabled) {
        return;
    }
    char buffer[64];
    snprintf(buffer, 64, "M,%d,%d,%d,%d", x, y, z, button);
    client_send(buffer);
}

void client_chunk(int amount) {
    if (!client_enabled) {
        return;
    }
    char buffer[32];
    snprintf(buffer, 32, "C,%d", amount);
    client_send(buffer);
}

void client_block(int x, int y, int z, int w) {
    if (!client_enabled) {
        return;
    }
    char buffer[1024];
    snprintf(buffer, 1024, "B,%d,%d,%d,%d", x, y, z, w);
    client_send(buffer);
}

void client_konstruct() {
    if (!client_enabled) {
        return;
    }
    char buffer[16];
    snprintf(buffer, 16, "K");
    client_send(buffer);
}

void client_light(int x, int y, int z, int w) {
    if (!client_enabled) {
        return;
    }
    char buffer[1024];
    snprintf(buffer, 1024, "L,%d,%d,%d,%d", x, y, z, w);
    client_send(buffer);
}

void client_sign(int x, int y, int z, int face, const char *text) {
    if (!client_enabled) {
        return;
    }
    char buffer[1024];
    snprintf(buffer, 1024, "S,%d,%d,%d,%d,%s", x, y, z, face, text);
    client_send(buffer);
}

void client_talk(const char *text) {
    if (!client_enabled) {
        return;
    }
    if (strlen(text) == 0) {
        return;
    }
    char buffer[1024];
    snprintf(buffer, 1024, "T,%s", text);
    client_send(buffer);
}

Packet client_recv() {
    Packet packet = { 0, 0 };
    if (!client_enabled) {
        return packet;
    }
    mtx_lock(&mutex);
    if (packet_buffer_size > 0) {
        packet.payload = malloc(sizeof(char) * packet_buffer_size);
        memcpy(packet.payload, packet_buffer, sizeof(char) * packet_buffer_size);
        packet.size = packet_buffer_size;
        packet_buffer_size = 0;
    }
    mtx_unlock(&mutex);

    return packet;
}

// recv worker thread
int recv_worker(void *arg) {
    char *data = malloc(sizeof(char) * RECV_SIZE);
    int size;
    while (1) {
        int t = 0;
        int length = 0;
        // get package length
        while(t < HEADER_SIZE) {
            if ((length = recv(sd, ((char *)&size) + t, HEADER_SIZE - t, 0)) <= 0) {
                if (running) {
                    perror("recv");
                    exit(1);
                } else {
                    break;
                }
            }
            t += length;
        }
        size = ntohl(size);

        if (size > RECV_SIZE) {
            printf("package to large, received %d bytes\n", size);
            exit(1);
        }

        // read 'size' bytes from the network
        t=0;
        length = 0;
        while(t < size) {
            if ((length = recv(sd, data+t, size-t, 0)) <= 0) {
                if (running) {
                    perror("recv");
                    exit(1);
                } else {
                    break;
                }
            }
            t += length;
        }

        // move data over to packet_buffer
        while (1) {
            int done = 0;
            mtx_lock(&mutex);
            if (packet_buffer_size + size + sizeof(size) < RECV_SIZE) {
                memcpy(packet_buffer + packet_buffer_size, &size, sizeof(size));
                memcpy(packet_buffer + packet_buffer_size + sizeof(size), data, sizeof(char) * size);
                packet_buffer_size += size + sizeof(size);
                done = 1;
            }
            mtx_unlock(&mutex);
            if (done) {
                break;
            }
            sleep(0);
        }
    }
    free(data);
    return 0;
}

int check_server(char *server) {
    struct hostent *host;
    return ((host = gethostbyname(server)) == 0 ? 0 : 1);
}

void client_connect(char *hostname, int port) {
    if (!client_enabled) {
        return;
    }
    struct hostent *host;
    struct sockaddr_in address;
    if ((host = gethostbyname(hostname)) == 0) {
#ifdef _WIN32
        printf("WSAGetLastError: %d",  WSAGetLastError());
#endif
        perror("gethostbyname");
        exit(1);
    }
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = ((struct in_addr *)(host->h_addr_list[0]))->s_addr;
    address.sin_port = htons(port);
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    if (connect(sd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("connect");
        exit(1);
    }
}

void client_start() {
    if (!client_enabled) {
        return;
    }
    running = 1;
    packet_buffer = (char *)calloc(RECV_SIZE, sizeof(char));
    packet_buffer_size = 0;
    mtx_init(&mutex, mtx_plain);
    if (thrd_create(&recv_thread, recv_worker, NULL) != thrd_success) {
        perror("thrd_create");
        exit(1);
    }
}

void client_stop() {
    if (!client_enabled) {
        return;
    }
    running = 0;
    close(sd);
    // if (thrd_join(recv_thread, NULL) != thrd_success) {
    //     perror("thrd_join");
    //     exit(1);
    // }
    // mtx_destroy(&mutex);
    packet_buffer_size = 0;
    free(packet_buffer);
    // printf("Bytes Sent: %d, Bytes Received: %d\n",
    //     bytes_sent, bytes_received);
}

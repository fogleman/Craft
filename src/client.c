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
#include <errno.h>
#include "client.h"
#include "konstructs.h"
#include "tinycthread.h"

#define MAX_RECV_SIZE 4096*1024
#define PACKETS MAX_PENDING_CHUNKS * 2
#define HEADER_SIZE 4

static int client_enabled = 0;
static int running = 0;
static int sd = 0;
static int bytes_sent = 0;
static int bytes_received = 0;
static thrd_t recv_thread;
static mtx_t mutex;
static Packet packets[PACKETS];

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
        SHOWERROR("client_sendall");
        exit(1);
    }
    if (client_sendall(sd, data, length) == -1) {
        SHOWERROR("client_sendall");
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

void client_inventory_select(int pos) {
    if (!client_enabled) {
        return;
    }
    char buffer[16];
    snprintf(buffer, 16, "A,%d", pos);
    client_send(buffer);
}

void click_at(int hit, int x, int y, int z, int button) {
    if (!client_enabled) {
        return;
    }
    char buffer[64];
    snprintf(buffer, 64, "M,%d,%d,%d,%d,%d", hit, x, y, z, button);
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

void client_click_inventory(int item) {
    if (!client_enabled) return;
    char buffer[64];
    snprintf(buffer, 64, "R,%d", item);
    client_send(buffer);
}

void client_close_inventory() {
    if (!client_enabled) {
        return;
    }
    char buffer[16];
    snprintf(buffer, 16, "I");
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

int client_recv(Packet *r_packets, int r_size) {
    if (!client_enabled) {
        return 0;
    }
    mtx_lock(&mutex);
    int j;
    for(j = 0; j < r_size; j++) {
        int found = 0;
        for(int i = 0; i < PACKETS; i++) {
            if (packets[i].size > 0) {
                r_packets[j] = packets[i];
                packets[i].size = 0;
                found = 1;
                break;
            }
        }
        if(!found) {
            break;
        }
    }
    mtx_unlock(&mutex);

    return j;
}

size_t recv_all(char* out_buf, size_t size) {
    int t = 0;
    int length = 0;
    while(t < size) {
        if ((length = recv(sd, out_buf + t, size - t, 0)) <= 0) {
            if (running) {
                #ifdef _WIN32
                if(WSAGetLastError() == WSAEINTR) {
                #else
                if(errno == EINTR) {
                #endif
                    continue;
                }
                SHOWERROR("recv");
                exit(1);
            } else {
                break;
            }
        }
        t += length;
    }
    return t;
}

// recv worker thread
int recv_worker(void *arg) {
    int size;

    while (1) {
        // Read header from network
        recv_all((char*)&size, HEADER_SIZE);
        size = ntohl(size);

        if (size > MAX_RECV_SIZE) {
            printf("package to large, received %d bytes\n", size);
            exit(1);
        }

        char type;
        // read 'size' bytes from the network
        recv_all(&type, sizeof(char));

        // Remove one byte type header'
        size = size - sizeof(char);

        char *data = malloc(sizeof(char) * (size + sizeof(size)));
        // read 'size' bytes from the network
        recv_all(data, size);
        // move data over to packet_buffer
        while (1) {
            int done = 0;
            mtx_lock(&mutex);
            for(int i = 0; i < PACKETS; i++) {
                Packet packet = packets[i];
                if (packet.size == 0) {
                    packet.payload = data;
                    packet.size = size;
                    packet.type = type;
                    packets[i] = packet;
                    done = 1;
                    break;
                }
            }
            mtx_unlock(&mutex);
            if (done) {
                break;
            }
            thrd_yield();
        }
    }
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
        SHOWERROR("gethostbyname");
        exit(1);
    }
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = ((struct in_addr *)(host->h_addr_list[0]))->s_addr;
    address.sin_port = htons(port);
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        SHOWERROR("socket");
        exit(1);
    }
    if (connect(sd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        SHOWERROR("connect");
        exit(1);
    }
}

void client_start() {
    if (!client_enabled) {
        return;
    }
    running = 1;
    memset(packets, 0, sizeof(Packet)*PACKETS);
    mtx_init(&mutex, mtx_plain);
    if (thrd_create(&recv_thread, recv_worker, NULL) != thrd_success) {
        SHOWERROR("thrd_create");
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
    //     SHOWERROR("thrd_join");
    //     exit(1);
    // }
    // mtx_destroy(&mutex);
    memset(packets, 0, sizeof(Packet)*PACKETS);
    // printf("Bytes Sent: %d, Bytes Received: %d\n",
    //     bytes_sent, bytes_received);
}

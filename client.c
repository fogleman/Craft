#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <zlib.h>
#include "client.h"
#include "tinycthread.h"

#define RECV_SIZE 4096
#define CHUNK_SIZE 65536
#define QUEUE_SIZE 1048576

static int client_enabled = 0;
static int sd = 0;
static char queue[QUEUE_SIZE] = {0};
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
    }
    return 0;
}

void client_send(char *data) {
    if (!client_enabled) {
        return;
    }
    if (client_sendall(sd, data, strlen(data)) == -1) {
        perror("client_sendall");
        exit(1);
    }
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
    if (distance < 0.1) {
        return;
    }
    px = x; py = y; pz = z; prx = rx; pry = ry;
    char buffer[1024];
    snprintf(buffer, 1024, "P,%.2f,%.2f,%.2f,%.2f,%.2f\n", x, y, z, rx, ry);
    client_send(buffer);
}

void client_chunk(int p, int q) {
    if (!client_enabled) {
        return;
    }
    char buffer[1024];
    snprintf(buffer, 1024, "C,%d,%d\n", p, q);
    client_send(buffer);
}

void client_block(int x, int y, int z, int w) {
    if (!client_enabled) {
        return;
    }
    char buffer[1024];
    snprintf(buffer, 1024, "B,%d,%d,%d,%d\n", x, y, z, w);
    client_send(buffer);
}

void client_talk(char *text) {
    if (!client_enabled) {
        return;
    }
    if (strlen(text) == 0) {
        return;
    }
    char buffer[1024];
    snprintf(buffer, 1024, "T,%s\n", text);
    client_send(buffer);
}

int client_recv(char *data, int length) {
    if (!client_enabled) {
        return 0;
    }
    int result = 0;
    mtx_lock(&mutex);
    char *p = strstr(queue, "\n");
    if (p) {
        *p = '\0';
        strncpy(data, queue, length);
        data[length - 1] = '\0';
        memmove(queue, p + 1, strlen(p + 1) + 1);
        result = 1;
    }
    mtx_unlock(&mutex);
    return result;
}

int recv_worker(void *arg) {
    int result;
    unsigned int have;
    unsigned char in[CHUNK_SIZE];
    unsigned char out[CHUNK_SIZE];
    z_stream stream;
    stream.avail_in = 0;
    stream.next_in = Z_NULL;
    while (client_enabled) {
        stream.zalloc = Z_NULL;
        stream.zfree = Z_NULL;
        stream.opaque = Z_NULL;
        result = inflateInit(&stream);
        if (result != Z_OK) {
            printf("END: inflateInit\n");
            perror("inflateInit");
            exit(1);
        }
        do {
            if (stream.avail_in == 0) {
                stream.avail_in = recv(sd, in, RECV_SIZE, 0);
                if (stream.avail_in <= 0) {
                    inflateEnd(&stream);
                    printf("END: recv\n");
                    perror("recv");
                    exit(1);
                }
                stream.next_in = in;
            }
            do {
                stream.avail_out = CHUNK_SIZE - 1;
                stream.next_out = out;
                result = inflate(&stream, Z_NO_FLUSH);
                switch (result) {
                    case Z_NEED_DICT:
                        result = Z_DATA_ERROR;
                    case Z_DATA_ERROR:
                    case Z_MEM_ERROR:
                        inflateEnd(&stream);
                        printf("END: inflate\n");
                        perror("inflate");
                        exit(1);
                }
                have = CHUNK_SIZE - 1 - stream.avail_out;
                out[have] = '\0';
                while (client_enabled) {
                    int done = 0;
                    mtx_lock(&mutex);
                    if (strlen(queue) + strlen(out) < QUEUE_SIZE) {
                        strcat(queue, out);
                        done = 1;
                    }
                    mtx_unlock(&mutex);
                    if (done) {
                        break;
                    }
                    sleep(0);
                }
            } while (stream.avail_out == 0);
        } while (result != Z_STREAM_END);
        inflateEnd(&stream);
        if (result != Z_STREAM_END) {
            printf("END: recv_worker\n");
            perror("recv_worker");
            exit(1);
        }
    }
    return 0;
}

void client_connect(char *hostname, int port) {
    if (!client_enabled) {
        return;
    }
    struct hostent *host;
    struct sockaddr_in address;
    if ((host = gethostbyname(hostname)) == 0) {
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
    client_enabled = 0;
    close(sd);
    if (thrd_join(recv_thread, NULL) != thrd_success) {
        perror("thrd_join");
        exit(1);
    }
    mtx_destroy(&mutex);
}

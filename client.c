#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#define BUFFER_SIZE 8192

static int client_enabled = 0;
static int sd;
static char send_buffer[BUFFER_SIZE] = {0};
static char recv_buffer[BUFFER_SIZE] = {0};
// static pthread_t send_thread;
static pthread_t recv_thread;
static pthread_mutex_t mutex;

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
    address.sin_addr.s_addr = ((struct in_addr *)(host->h_addr))->s_addr;
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

void client_send(char *data) {
    if (!client_enabled) {
        return;
    }
    if (client_sendall(sd, data, strlen(data)) == -1) {
        perror("client_sendall");
        exit(1);
    }
    // pthread_mutex_lock(&mutex);
    // strcat(send_buffer, data);
    // pthread_mutex_unlock(&mutex);
}

int client_recv(char *data) {
    if (!client_enabled) {
        return 0;
    }
    int result = 0;
    pthread_mutex_lock(&mutex);
    char *p = strstr(recv_buffer, "\n");
    if (p) {
        *p = '\0';
        strcpy(data, recv_buffer);
        memmove(recv_buffer, p + 1, strlen(p + 1) + 1);
        result = 1;
    }
    pthread_mutex_unlock(&mutex);
    return result;
}

// void *send_worker(void *arg) {
//     while (1) {
//         pthread_mutex_lock(&mutex);
//         int length = strlen(send_buffer);
//         if (length) {
//             if (client_sendall(sd, send_buffer, length) == -1) {
//                 perror("client_sendall");
//                 exit(1);
//             }
//             send_buffer[0] = '\0';
//         }
//         pthread_mutex_unlock(&mutex);
//     }
//     return NULL;
// }

void *recv_worker(void *arg) {
    while (1) {
        char data[BUFFER_SIZE] = {0};
        if (recv(sd, data, BUFFER_SIZE, 0) == -1) {
            perror("recv");
            exit(1);
        }
        pthread_mutex_lock(&mutex);
        strcat(recv_buffer, data);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void client_start() {
    if (!client_enabled) {
        return;
    }
    pthread_mutex_init(&mutex, NULL);
    // if (pthread_create(&send_thread, NULL, send_worker, NULL)) {
    //     perror("pthread_create");
    //     exit(1);
    // }
    if (pthread_create(&recv_thread, NULL, recv_worker, NULL)) {
        perror("pthread_create");
        exit(1);
    }
}

void client_stop() {
    if (!client_enabled) {
        return;
    }
    close(sd);
    // if (pthread_join(send_thread, NULL)) {
    //     perror("pthread_join");
    //     exit(1);
    // }
    if (pthread_join(recv_thread, NULL)) {
        perror("pthread_join");
        exit(1);
    }
    pthread_mutex_destroy(&mutex);
}

// int main(int argc, char **argv) {
//     client_connect(HOST, PORT);
//     client_start();
//     client_send("B,0,0,0,0,0,1\n");
//     client_send("C,0,0\n");
//     char data[BUFFER_SIZE];
//     while (1) {
//         if (client_recv(data)) {
//             printf("%s\n", data);
//         }
//         sleep(1);
//     }
//     client_stop();
// }

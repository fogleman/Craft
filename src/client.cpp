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
#include <sstream>
#include "client.h"
#include "konstructs.h"

#define MAX_RECV_SIZE 4096*1024
#define PACKETS (MAX_PENDING_CHUNKS * 2)
#define HEADER_SIZE 4

namespace konstructs {

    Client::Client(const string &nick, const string &hash,
                   const string &hostname, const int port) {
        struct hostent *host;
        struct sockaddr_in address;
        if ((host = gethostbyname(hostname.c_str())) == 0) {
#ifdef _WIN32
            std::cerr << "WSAGetLastError: " << WSAGetLastError() << std::endl;
#endif
            SHOWERROR("gethostbyname");
            exit(1);
        }
        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = ((struct in_addr *)(host->h_addr_list[0]))->s_addr;
        address.sin_port = htons(port);
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            SHOWERROR("socket");
            throw std::runtime_error("Failed to create socket");
        }
        if (connect(sock, (struct sockaddr *)&address, sizeof(address)) == -1) {
            SHOWERROR("connect");
            throw std::runtime_error("Failed to connect");
        }
        worker_thread = new std::thread(&Client::recv_worker, this);
        version(PROTOCOL_VERSION, nick, hash);
    }

    size_t Client::recv_all(char* out_buf, const size_t size) {
        int t = 0;
        int length = 0;
        while(t < size) {
            if ((length = recv(sock, out_buf + t, size - t, 0)) <= 0) {
#ifdef _WIN32
                if(WSAGetLastError() == WSAEINTR) {
                    continue;
                }
#else
                if(errno == EINTR) {
                    continue;
                }
#endif
                SHOWERROR("recv");
                throw std::runtime_error("Failed to receive");
            }
            t += length;
        }
        return t;
    }

    void Client::recv_worker() {
        int size;
        while (1) {
            // Read header from network
            recv_all((char*)&size, HEADER_SIZE);

            size = ntohl(size);

            if (size > MAX_RECV_SIZE) {
                std::cerr << "package too large, received " << size << " bytes" << std::endl;
                throw std::runtime_error("Packet too large");
            }

            char type;
            // read 'size' bytes from the network
            recv_all(&type, sizeof(char));

            // Remove one byte type header'
            size = size - 1;
            auto packet = make_shared<Packet>(type,size);
            // read 'size' bytes from the network
            int r = recv_all(packet->buffer(), packet->size);
            // move data over to packet_buffer
            packets_mutex.lock();
            packets.push(packet);
            packets_mutex.unlock();
        }
    }

    vector<shared_ptr<Packet>> Client::receive(const int max) {
        vector<shared_ptr<Packet>> head;
        packets_mutex.lock();
        for(int i=0; i < max; i++) {
            if(packets.empty()) break;
            head.push_back(packets.front());
            packets.pop();
        }
        packets_mutex.unlock();
        return head;
    }

    int Client::send_all(const char *data, int length) {
        int count = 0;
        while (count < length) {
            int n = send(sock, data + count, length, 0);
            if (n == -1) {
                return -1;
            }
            count += n;
            length -= n;
            bytes_sent += n;
        }
        return 0;
    }

    void Client::send_string(const string &str) {
        int header_size = htonl(str.size());
        if (send_all((char*)&header_size, sizeof(header_size)) == -1) {
            SHOWERROR("client_sendall");
            throw std::runtime_error("Failed to send");
        }
        if (send_all(str.c_str(), str.size()) == -1) {
            SHOWERROR("client_sendall");
            throw std::runtime_error("Failed to send");
        }
    }

    void Client::version(const int version, const string &nick, const string &hash) {
        std::stringstream ss;
        ss << "V," << version << "," << nick << "," << hash;
        send_string(ss.str());
    }

    void Client::position(const Vector3f position,
                          const float rx, const float ry) {
        std::stringstream ss;
        ss << "P," << position[0] << "," << position[1] << "," << position[2] << "," << rx << "," << ry;
        send_string(ss.str());
    }

    void Client::inventory_select(const int pos) {
        std::stringstream ss;
        ss << "A," << pos;
        send_string(ss.str());
    }

    void Client::click_at(const int hit, const int x, const int y, const int z,
                          const int button) {
        std::stringstream ss;
        ss << "M," << hit << "," << x << "," << y << "," << z << "," << button;
        send_string(ss.str());
    }

    void Client::chunk(const int amount) {
        std::stringstream ss;
        ss << "C," << amount;
        send_string(ss.str());
    }

    void Client::konstruct() {
        send_string("K");
    }

    void Client::click_inventory(const int item) {
        std::stringstream ss;
        ss << "R," << item;
        send_string(ss.str());
    }

    void Client::close_inventory() {
        send_string("I");
    }

    void Client::talk(const string &text) {
        std::stringstream ss;
        ss << "T," << text;
        send_string(ss.str());
    }
};

/*
 * Deprecated
 */

static int client_enabled = 0;
static int running = 0;
static int sd = 0;
static int bytes_sent = 0;
static int bytes_received = 0;
static std::mutex mutex;
static Packet packets[PACKETS];
static int last_packet;

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
    char buffer[128];
    snprintf(buffer, 128, "V,%d,%s,%s", version, nick, hash);
    client_send(buffer);
}

void client_login(const char *username, const char *identity_token) {
    if (!client_enabled) {
        return;
    }
    char buffer[128];
    snprintf(buffer, 128, "A,%s,%s", username, identity_token);
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
    char buffer[128];
    snprintf(buffer, 128, "P,%.2f,%.2f,%.2f,%.2f,%.2f", x, y, z, rx, ry);
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
    char buffer[128];
    snprintf(buffer, 128, "B,%d,%d,%d,%d", x, y, z, w);
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
    char buffer[128];
    snprintf(buffer, 128, "L,%d,%d,%d,%d", x, y, z, w);
    client_send(buffer);
}

void client_sign(int x, int y, int z, int face, const char *text) {
    if (!client_enabled) {
        return;
    }
    char buffer[128];
    snprintf(buffer, 128, "S,%d,%d,%d,%d,%s", x, y, z, face, text);
    client_send(buffer);
}

void client_talk(const char *text) {
    if (!client_enabled) {
        return;
    }
    if (strlen(text) == 0) {
        return;
    }
    char buffer[512];
    snprintf(buffer, 512, "T,%s", text);
    client_send(buffer);
}

int client_recv(Packet *r_packets, int r_size) {
    if (!client_enabled) {
        return 0;
    }
    mutex.lock();
    int r_found = 0;
    int index;
    for(int i = 0; i < PACKETS; i++) {
        index = (last_packet + i) % PACKETS;
        if (packets[index].size > 0) {
            r_packets[r_found] = packets[index];
            packets[index].size = 0;
            r_found++;
            if(r_found == r_size)
                 break;
        }
    }

    last_packet = index;
    mutex.unlock();

    return r_found;
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
void recv_worker() {
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

        char *data = (char*)malloc(sizeof(char) * (size + sizeof(size)));
        // read 'size' bytes from the network
        recv_all(data, size);
        // move data over to packet_buffer
        while (1) {
            int done = 0;
            mutex.lock();
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
            mutex.unlock();
            if (done) {
                break;
            }
            std::this_thread::yield();
        }
    }
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
    last_packet = 0;
    new std::thread(recv_worker);
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

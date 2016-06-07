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
#include <algorithm>
#include "client.h"

#define PROTOCOL_VERSION 8
#define MAX_RECV_SIZE 4096*1024
#define PACKETS (MAX_PENDING_CHUNKS * 2)
#define HEADER_SIZE 4

namespace konstructs {
    using nonstd::nullopt;

    Client::Client() : connected(false) {
        worker_thread = new std::thread(&Client::recv_worker, this);
    }

    void Client::open_connection(const string &nick, const string &hash,
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

    void Client::process_chunk(Packet *packet) {
        int p, q, k;
        char *pos = packet->buffer();

        p = ntohl(*((int*)pos));
        pos += sizeof(int);

        q = ntohl(*((int*)pos));
        pos += sizeof(int);

        k = ntohl(*((int*)pos));
        pos += sizeof(int);

        Vector3i position(p, q, k);
        const int blocks_size = packet->size - 3 * sizeof(int);
        auto chunk = make_shared<ChunkData>(position, pos, blocks_size);
        std::unique_lock<std::mutex> ulock_packets(packets_mutex);
        chunks.push_back(chunk);
    }

    void Client::recv_worker() {
        while(1) {
            try {
                // Wait for an open connection
                std::unique_lock<std::mutex> ulock_connected(mutex_connected);
                cv_connected.wait(ulock_connected, [&]{ return connected; });
                ulock_connected.unlock();

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
                    if(packet->type == 'C')
                        process_chunk(packet.get());
                    else {
                        std::unique_lock<std::mutex> ulock_packets(packets_mutex);
                        packets.push(packet);
                    }
                }
            } catch(const std::exception& ex) {
                std::cout << "Caught exception: " << ex.what() << std::endl;
                std::cout << "Will assume connection is down: " << ex.what() << std::endl;
                force_close();
            }
        }
    }

    vector<shared_ptr<Packet>> Client::receive(const int max) {
        vector<shared_ptr<Packet>> head;
        std::unique_lock<std::mutex> ulock_packets(packets_mutex);
        for(int i=0; i < max; i++) {
            if(packets.empty()) break;
            head.push_back(packets.front());
            packets.pop();
        }
        return head;
    }

    optional<shared_ptr<ChunkData>> Client::receive_prio_chunk(const Vector3i pos) {
        std::unique_lock<std::mutex> ulock_packets(packets_mutex);
        for(auto it = chunks.begin(); it != chunks.end(); ++it) {
            auto chunk = *it;
            Vector3i chunk_position = chunk->position;
            int dp = chunk_position[0] - pos[0];
            int dq = chunk_position[1] - pos[1];
            int dk = chunk_position[2] - pos[2];
            if(dp >= -1 && dp <= 1 && dq >= -1 && dq <= 1 && dk >= -1 && dk <= 1) {
                chunks.erase(it);
                return optional<shared_ptr<ChunkData>>(chunk);
            }
        }
        return nullopt;
    }

    vector<shared_ptr<ChunkData>> Client::receive_chunks(const int max) {
        std::unique_lock<std::mutex> ulock_packets(packets_mutex);
        int maxOrSize = std::min(max, (int)chunks.size());
        auto last = chunks.begin() + maxOrSize;
        vector<shared_ptr<ChunkData>> head(chunks.begin(), last);
        chunks.erase(chunks.begin(), last);
        return head;
    }

    int Client::send_all(const char *data, int length) {
        int count = 0;
        while (count < length) {
            int n = send(sock, data + count, length, 0);
            if (n == -1) {
                force_close();
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
        }
        if (send_all(str.c_str(), str.size()) == -1) {
            SHOWERROR("client_sendall");
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

    void Client::click_at(const int hit, const Vector3i pos,
                          const int button) {
        std::stringstream ss;
        ss << "M," << hit << "," << pos[0] << "," << pos[1] << "," << pos[2] << "," << button;
        send_string(ss.str());
    }

    void Client::chunk(const Vector3i position) {
        std::stringstream ss;
        ss << "C," << position[0] << "," << position[1] << "," << position[2];
        send_string(ss.str());
    }

    void Client::konstruct() {
        send_string("K");
    }

    void Client::click_inventory(const int item, const int button) {
        std::stringstream ss;
        ss << "R," << item << "," << button;
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

    bool Client::is_connected() {
        std::unique_lock<std::mutex> ulck_connected(mutex_connected);
        return connected;
    }

    void Client::set_connected(bool state) {
        std::unique_lock<std::mutex> ulck_connected(mutex_connected);
        connected = state;
        cv_connected.notify_all();
    }

    void Client::force_close() {
        close(sock);
        set_connected(false);
    }
};

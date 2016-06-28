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
#include <chrono>
#include "client.h"

#define PROTOCOL_VERSION 8
#define MAX_RECV_SIZE 4096*1024
#define PACKETS (MAX_PENDING_CHUNKS * 2)
#define HEADER_SIZE 4

namespace konstructs {
    using nonstd::nullopt;

    const int NO_CHUNK_FOUND = 0x0FFFFFFF;

    Client::Client() :
        connected(false),
        player_chunk(0,0,0), radius(0) {
        recv_thread = new std::thread(&Client::recv_worker, this);
        send_thread = new std::thread(&Client::send_worker, this);
        chunk_thread = new std::thread(&Client::chunk_worker, this);
        inflation_buffer = new char[BLOCK_BUFFER_SIZE];
    }

    string Client::get_error_message() {
        return error_message;
    }

    void Client::open_connection(const string &nick, const string &hash,
                                 const string &hostname, const int port) {
        error_message = "";
        struct hostent *host;
        struct sockaddr_in address;
        if ((host = gethostbyname(hostname.c_str())) == 0) {
#ifdef _WIN32
            std::cerr << "WSAGetLastError: " << WSAGetLastError() << std::endl;
#endif
            SHOWERROR("gethostbyname");
            error_message = "Could not find server: " + hostname;
            throw std::runtime_error(error_message);
        }
        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = ((struct in_addr *)(host->h_addr_list[0]))->s_addr;
        address.sin_port = htons(port);
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            SHOWERROR("socket");
            error_message = "Failed to create socket";
            throw std::runtime_error(error_message);
        }
        if (connect(sock, (struct sockaddr *)&address, sizeof(address)) == -1) {
            SHOWERROR("connect");
            error_message = "Could not connect to server";
            throw std::runtime_error(error_message);
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

    void Client::process_chunk_updated(Packet *packet) {
        std::string str = packet->to_string();
        int p,q,k;
        if(sscanf(str.c_str(), ",%d,%d,%d", &p, &q, &k) != 3) {
            throw std::runtime_error(str);
        }
        Vector3i pos(p, q, k);
        chunk_updated(pos);
    }


    void Client::process_error(Packet *packet) {
        error_message = packet->to_string().substr(1);
        force_close();
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
        received_chunk(position);
        const int blocks_size = packet->size - 3 * sizeof(int);
        auto chunk = make_shared<ChunkData>(position, pos, blocks_size, (uint8_t*)inflation_buffer);
        std::lock_guard<std::mutex> lock_packets(packets_mutex);
        chunks.push_back(chunk);
    }

    void Client::recv_worker() {
        std::cout<<"[Recv worker]: started"<<std::endl;
        while(1) {
            try {
                std::cout<<"[Recv worker]: waiting for connection"<<std::endl;
                // Wait for an open connection
                std::unique_lock<std::mutex> ulock_connected(mutex_connected);
                cv_connected.wait(ulock_connected, [&]{ return connected; });
                ulock_connected.unlock();
                std::cout<<"[Recv worker]: connection established, entering main loop"<<std::endl;
                int size;
                while (connected) {
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
                    else if(packet->type == 'E')
                        process_error(packet.get());
                    else if(packet->type == 'c')
                        process_chunk_updated(packet.get());
                    else {
                        std::lock_guard<std::mutex> lock_packets(packets_mutex);
                        packets.push(packet);
                    }
                }
            } catch(const std::exception& ex) {
                std::cout << "[Recv worker]: Caught exception: " << ex.what() << std::endl;
                std::cout << "[Recv worker]: Will assume connection is down: " << ex.what() << std::endl;
                error_message = "Disconnected from server.";
                force_close();
            }
        }
    }

    vector<shared_ptr<Packet>> Client::receive(const int max) {
        vector<shared_ptr<Packet>> head;
        std::lock_guard<std::mutex> lock_packets(packets_mutex);
        for(int i=0; i < max; i++) {
            if(packets.empty()) break;
            head.push_back(packets.front());
            packets.pop();
        }
        return head;
    }

    optional<shared_ptr<ChunkData>> Client::receive_prio_chunk(const Vector3i pos) {
        std::lock_guard<std::mutex> lock_packets(packets_mutex);
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
        std::lock_guard<std::mutex> lock_packets(packets_mutex);
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
                return -1;
            }
            count += n;
            length -= n;
            bytes_sent += n;
        }
        return 0;
    }

    void Client::send_string(const string &str) {
        {
            std::lock_guard<std::mutex> lock(mutex_send);
            send_queue.push(str);
        }
        cv_send.notify_all();
    }

    void Client::send_worker() {
        std::cout<<"[Send worker]: started"<<std::endl;
        while(1) {
            try {
                std::cout<<"[Send worker]: waiting for connection"<<std::endl;
                // Wait for an open connection
                std::unique_lock<std::mutex> ulock_connected(mutex_connected);
                cv_connected.wait(ulock_connected, [&]{ return connected; });
                ulock_connected.unlock();
                std::cout<<"[Send worker]: connection established, entering main loop"<<std::endl;
                int size;
                while (connected) {
                    // Wait for an open connection
                    std::unique_lock<std::mutex> ulock_send(mutex_send);
                    cv_send.wait(ulock_send, [&]{ return send_queue.size() > 0; });
                    auto str = send_queue.front();
                    send_queue.pop();
                    ulock_send.unlock();
                    int header_size = htonl(str.size());
                    if (send_all((char*)&header_size, sizeof(header_size)) == -1) {
                        SHOWERROR("client_sendall");
                    }
                    if (send_all(str.c_str(), str.size()) == -1) {
                        SHOWERROR("client_sendall");
                    }
                }
            } catch(const std::exception& ex) {
                std::cout << "[Send worker]: Caught exception: " << ex.what() << std::endl;
                std::cout << "[Send worker]: Will assume connection is down: " << ex.what() << std::endl;
                error_message = "Disconnected from server.";
                force_close();
            }
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

    void Client::click_at(const int hit, const Vector3i pos,
                          const int button, const int active) {
        std::stringstream ss;
        ss << "M," << hit << "," << pos[0] << "," << pos[1] << "," << pos[2] << "," << button << "," << active;
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

    void Client::set_connected(bool state) {
        std::lock_guard<std::mutex> lck(mutex_connected);
        connected = state;
        cv_connected.notify_all();
    }

    bool Client::is_connected() {
        std::lock_guard<std::mutex> lck(mutex_connected);
        return connected;
    }

    void Client::set_logged_in(bool state) {
        std::lock_guard<std::mutex> lck(mutex_connected);
        logged_in = state;
        cv_connected.notify_all();
    }

    bool Client::is_logged_in() {
        std::lock_guard<std::mutex> lck(mutex_connected);
        return logged_in;
    }

    void Client::force_close() {
        close(sock);
        set_logged_in(false);
        set_connected(false);
    }

    void Client::chunk_updated(const Vector3i &pos) {
        std::lock_guard<std::mutex> ulck_chunk(mutex_chunk);
        updated_queue.push_back(pos);
    }


    void Client::set_player_chunk(const Vector3i &chunk) {
        std::lock_guard<std::mutex> ulck_chunk(mutex_chunk);
        player_chunk = chunk;
    }

    void Client::set_radius(int r) {
        std::lock_guard<std::mutex> ulck_chunk(mutex_chunk);
        radius = r;
    }

    void Client::received_chunk(const Vector3i &pos) {
        std::lock_guard<std::mutex> ulck_chunk(mutex_chunk);
        received_queue.push_back(pos);
    }

    void Client::chunk_worker() {
        std::cout<<"[Chunk worker]: started"<<std::endl;
        while(1) {
            std::cout<<"[Chunk worker]: waiting for connection and user log in"<<std::endl;
            // Wait for an open connection
            std::unique_lock<std::mutex> ulock_connected(mutex_connected);
            cv_connected.wait(ulock_connected, [&]{ return connected && logged_in; });
            ulock_connected.unlock();
            std::cout<<"[Chunk worker]: connection established and user logged in, entering main loop"<<std::endl;
            while(connected && logged_in) {
                int sleep_time;
                Vector3i p_chunk;
                int r;
                int request_chunks;
                {
                    std::lock_guard<std::mutex> lck_chunk(mutex_chunk);
                    for(auto chunk: updated_queue) {
                        updated.insert(chunk);
                    }

                    for(auto chunk: received_queue) {
                        requested.erase(chunk);
                        updated.erase(chunk);
                        received.insert(chunk);
                    }
                    received_queue.clear();
                    p_chunk = player_chunk;
                    r = radius;
                    request_chunks = 1;
                }
                for(auto it = received.begin(); it != received.end();) {
                    if((*it - p_chunk).norm() > r) {
                        it = received.erase(it);
                    } else {
                        ++it;
                    }
                }

                vector<Vector3i> best_chunks;

                for(int i = 0; i < request_chunks; i++) {
                    Vector3i best_chunk;
                    int best_chunk_distance = NO_CHUNK_FOUND;
                    for(int p = p_chunk[0] - (r - 1); p < p_chunk[0] + r; p++) {
                        for(int q = p_chunk[1] - (r - 1); q < p_chunk[1] + r; q++) {
                            for(int k = p_chunk[2] - (r - 1); k < p_chunk[2] + r; k++) {

                                Vector3i pos(p, q, k);
                                int distance = (pos - p_chunk).norm();
                                if(received.find(pos) == received.end()) {
                                    if(requested.find(pos) == requested.end()) {
                                        /* We don't have it and didn't request it */
                                        if(distance < best_chunk_distance) {
                                            best_chunk_distance = distance;
                                            best_chunk = pos;
                                        }
                                    } else {
                                        /* Already requested */
                                    }
                                } else if(requested.find(pos) == requested.end() && updated.find(pos) != updated.end()) {
                                    /* we got it, but it was updated and not yet again requested */
                                    if(distance < best_chunk_distance) {
                                        best_chunk_distance = distance;
                                        best_chunk = pos;
                                    }
                                }

                            }
                        }
                    }

                    if(best_chunk_distance != NO_CHUNK_FOUND) {
                        best_chunks.push_back(best_chunk);
                        requested.insert(best_chunk);
                        updated.erase(best_chunk);
                    }
                }
                for(auto best_chunk: best_chunks)
                    chunk(best_chunk);
                if(best_chunks.empty()) {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(15 * request_chunks));
                }
            }
        }
    }
};

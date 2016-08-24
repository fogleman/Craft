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
#include <functional>
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
        player_chunk(0,0,0), radius(0), loaded_radius(0) {
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
                          const int button, const int active,
                          const uint8_t direction, const uint8_t rotation) {
        std::stringstream ss;
        ss << "M," << hit << "," << pos[0] << "," << pos[1] << "," << pos[2] <<
            "," << button << "," << active << "," << (int)direction << "," << (int)rotation;
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

    void Client::update_radius(const int radius) {
        std::stringstream ss;
        ss << "D," << radius;
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
        {
            std::lock_guard<std::mutex> ulck_chunk(mutex_chunk);
            radius = r;
        }
        update_radius(r + KEEP_EXTRA_CHUNKS);
    }

    void Client::set_loaded_radius(int r) {
        std::lock_guard<std::mutex> ulck_chunk(mutex_chunk);

        if (r > radius || loaded_radius > radius) {
            // Never set radius outside radius

            loaded_radius = radius;
        } else if (r > loaded_radius) {
            // The loaded radius has increased

            loaded_radius = r;
        } else {
            // We recevied a chunk closer then the loaded radius
            // and we never want to reduce the loaded radius.
        }
    }

    int Client::get_loaded_radius() {
        std::lock_guard<std::mutex> ulck_chunk(mutex_chunk);
        return loaded_radius;
    }

    void Client::received_chunk(const Vector3i &pos) {
        std::lock_guard<std::mutex> ulck_chunk(mutex_chunk);
        received_queue.push_back(pos);
    }

    /* The chunk is not received, and never requested */
    bool Client::is_empty_chunk(Vector3i pos) {
        return received.find(pos) == received.end() && requested.find(pos) == requested.end();
    }

    /* The chunk is requested */
    bool Client::is_requested_chunk(Vector3i pos) {
        return requested.find(pos) != requested.end();
    }

    /* The chunk is not requested, and has updates */
    bool Client::is_updated_chunk(Vector3i pos) {
        return requested.find(pos) == requested.end() && updated.find(pos) != updated.end();
    }

    /* Ask the server for a chunk, and wait a little while */
    void Client::request_chunk_and_sleep(Vector3i pos, int msec) {
        requested.insert(pos);
        chunk(pos);
        std::this_thread::sleep_for(std::chrono::milliseconds(msec));
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

            int r = 0; // Stores the current radius
            int old_r = 0; // Stores the previous radius
            Vector3i p_chunk; // Stores the player chunk
            bool chunk_changed = false; // Stores if the chunk the player is in changed
            // Stores the chunks that needs to be fetched in priority order
            priority_queue<ChunkToFetch, vector<ChunkToFetch>, LessThanByScore> chunks_to_fetch;

            while(connected && logged_in) {

                {
                    // Locks the chunk mutex and makes local copies of all
                    // shared variables and empties the updated and received queues
                    std::lock_guard<std::mutex> lck_chunk(mutex_chunk);

                    // Copy all chunks from the updated queue
                    for(auto chunk: updated_queue) {
                        updated.insert(chunk);
                    }

                    // Clear the updated queue
                    updated_queue.clear();

                    // Copy all chunks from the receive queue
                    for(auto chunk: received_queue) {
                        // Remove received chunks from requested set
                        requested.erase(chunk);
                        // Remove received chunks from updated set
                        updated.erase(chunk);
                        // Insert into received set
                        received.insert(chunk);
                    }

                    // Clear received queue
                    received_queue.clear();

                    // Check if player chunk changed
                    if(p_chunk != player_chunk) {
                        // Update local chunk variable
                        p_chunk = player_chunk;
                        // Set chunk_changed true
                        chunk_changed = true;
                    }

                    // Updated local radius
                    r = radius;
                }

                // Chunk changed
                if (chunk_changed) {

                    // Request the surrounding chunks quickly
                    for (int p = -1; p < 2; p++) {
                        for (int q = -1; q < 2; q++) {
                            for (int s = -1; s < 2; s++) {

                                Vector3i lpos = p_chunk + Vector3i(p, q, s);
                                if (is_empty_chunk(lpos)) {
                                    // Insert into requested set
                                    requested.insert(lpos);

                                    // Request the chunk
                                    chunk(lpos);
                                }
                            }
                        }
                    }

                    // Empty the priority queue
                    chunks_to_fetch = priority_queue<ChunkToFetch, vector<ChunkToFetch>, LessThanByScore>();

                    // Rebuild the priority queue
                    for(int p = -r - 1; p < r; p++) {
                        for(int q = -r - 1; q < r; q++) {
                            for(int k = -r - 1; k < r; k++) {
                                Vector3i pos = p_chunk + Vector3i(p, q, k);

                                if(is_empty_chunk(pos)) {
                                    int distance = (pos - p_chunk).norm();
                                    // This checks removes edges so that we request a sphere not a cube
                                    if(distance <= r) {
                                        // Add chunk to queue
                                        chunks_to_fetch.push({distance, pos});
                                    }
                                }
                            }
                        }
                    }
                    // Set chunk change to false,
                    // no need to rebuild queue until the chunk changes again
                    chunk_changed = false;
                }

                // Check if the radius increased
                if(r - old_r > 0) {
                    // Extend the priority queue with new chunks due to increased radius
                    for(int p = -r - 1; p < r; p++) {
                        for(int q = -r - 1; q < r; q++) {
                            for(int k = -r - 1; k < r; k++) {
                                Vector3i pos = p_chunk + Vector3i(p, q, k);

                                if(is_empty_chunk(pos)) {
                                    int distance = (pos - p_chunk).norm();

                                    // This checks removes edges so that we request a sphere not a cube
                                    // It also rejects chunks that was already previously added to the queue
                                    // that is chunks within the old radius
                                    if(distance <= r && distance >= old_r) {
                                        chunks_to_fetch.push({distance, pos});
                                    }
                                }
                            }
                        }
                    }
                }

                // Update the old radius with the new one
                old_r = r;

                // Remove old chunks in received set that are outside render distance
                for(auto it = received.begin(); it != received.end();) {
                    int distance = (*it - p_chunk).norm();
                    if(distance >= (r + KEEP_EXTRA_CHUNKS)) {
                        // Erase increases iterator to the next element
                        it = received.erase(it);
                    } else {
                        // If we didn't erase we need to increase iterator ourselves
                        ++it;
                    }
                }

                // Look at the update queue and add to request queue
                for(auto it = updated.begin(); it != updated.end();) {
                    Vector3i pos = *it;
                    // Erase increases iterator
                    it = updated.erase(it);
                    int distance = (pos - p_chunk).norm();

                    // Add to chunk queue
                    chunks_to_fetch.push({distance, pos});

                    // Remove from requested set, since we need a newer version
                    requested.erase(pos);
                }

                if(!chunks_to_fetch.empty()) {
                    ChunkToFetch c;
                    c.score = NO_CHUNK_FOUND;

                    // Fetch next chunk in the queue that is within radius
                    do {
                        c = chunks_to_fetch.top();
                        chunks_to_fetch.pop();
                        // The following check validates before exiting the loop that:
                        // 1. The chunk is within the radius
                        // 2. The chunk queue is empty (i.e. no chunk was found)
                        // 3. The chunk was not already requested
                    } while((c.chunk - p_chunk).norm() > r &&
                            !chunks_to_fetch.empty() &&
                            !is_requested_chunk(c.chunk));

                    // Check that at least one suitable chunk was found
                    if(c.score != NO_CHUNK_FOUND) {
                        // Insert into requested
                        requested.insert(c.chunk);
                        // Remove from updated
                        updated.erase(c.chunk);
                        // Request chunk
                        chunk(c.chunk);
                        // Update loaded radius
                        set_loaded_radius(c.score);
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(15));
            }
        }
    }
};

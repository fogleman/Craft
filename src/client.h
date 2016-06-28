#ifndef _client_h_
#define _client_h_
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <string>
#include <memory>
#include <queue>
#include <unordered_set>
#include <thread>
#include <Eigen/Geometry>
#include "matrix.h"
#include "optional.hpp"
#include "chunk.h"

#define DEFAULT_PORT 4080

namespace konstructs {
    using namespace std;
    using namespace Eigen;
    using nonstd::optional;

    class Packet {
    public:
        Packet(const char _type, const size_t _size):
            type(_type), size(_size) {
            mBuffer = new char[size];
        }
        ~Packet() {
            delete[] mBuffer;
        }
        const char type;
        const size_t size;
        char* buffer() { return mBuffer; }
        string to_string() {
            string str(mBuffer, size);
            return str;
        }
    private:
        char *mBuffer;
    };

    class Client {
    public:
        Client();
        void open_connection(const string &nick, const string &hash,
               const string &hostname, const int port = DEFAULT_PORT);
        void version(const int version, const string &nick, const string &hash);
        void position(const Vector3f position,
                      const float rx, const float ry);
        void chunk(const Vector3i position);
        void konstruct();
        void click_inventory(const int item, const int button);
        void close_inventory();
        void talk(const string &text);
        void click_at(const int hit, const Vector3i pos, const int button, const int active);
        string get_error_message();
        void set_connected(bool state);
        bool is_connected();
        void set_logged_in(bool state);
        bool is_logged_in();
        vector<shared_ptr<Packet>> receive(const int max);
        optional<shared_ptr<ChunkData>> receive_prio_chunk(const Vector3i pos);
        vector<shared_ptr<ChunkData>> receive_chunks(const int max);
        void set_player_chunk(const Vector3i &chunk);
        void set_radius(int r);
    private:
        int send_all(const char *data, const int length);
        void send_string(const string &str);
        size_t recv_all(char* out_buf, const size_t size);
        void process_error(Packet *packet);
        void process_chunk(Packet *packet);
        void process_chunk_updated(Packet *packet);
        void recv_worker();
        void send_worker();
        void chunk_worker();
        void force_close();
        void received_chunk(const Vector3i &pos);
        void chunk_updated(const Vector3i &pos);
        int bytes_sent;
        int sock;
        std::mutex mutex_send;
        std::condition_variable cv_send;
        std::queue<std::string> send_queue;
        std::mutex packets_mutex;
        std::mutex mutex_connected;
        std::condition_variable cv_connected;
        std::thread *recv_thread;
        std::thread *send_thread;
        std::thread *chunk_thread;
        std::queue<shared_ptr<Packet>> packets;
        std::deque<shared_ptr<ChunkData>> chunks;
        bool connected;
        bool logged_in;
        std::string error_message;
        char *inflation_buffer;
        /* Chunk worker */
        Vector3i player_chunk;
        int radius;
        std::unordered_set<Vector3i, matrix_hash<Vector3i>> updated;
        std::unordered_set<Vector3i, matrix_hash<Vector3i>> requested;
        std::unordered_set<Vector3i, matrix_hash<Vector3i>> received;
        std::vector<Vector3i> received_queue;
        std::vector<Vector3i> updated_queue;
        std::mutex mutex_chunk;
    };
};


#define SHOWERROR(ErrMsg) { char aBuf[256]; snprintf(aBuf, 256, "At '%s:%d' in function '%s' occurred error '%s'",__FILE__,__LINE__,__FUNCTION__,ErrMsg); perror(aBuf); };

#endif

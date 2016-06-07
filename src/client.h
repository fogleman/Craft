#ifndef _client_h_
#define _client_h_
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <string>
#include <memory>
#include <queue>
#include <thread>
#include <Eigen/Geometry>
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
        bool is_connected();
        string get_error_message();
        void set_connected(bool state);
        vector<shared_ptr<Packet>> receive(const int max);
        optional<shared_ptr<ChunkData>> receive_prio_chunk(const Vector3i pos);
        vector<shared_ptr<ChunkData>> receive_chunks(const int max);
    private:
        int send_all(const char *data, const int length);
        void send_string(const string &str);
        size_t recv_all(char* out_buf, const size_t size);
        void process_error(Packet *packet);
        void process_chunk(Packet *packet);
        void recv_worker();
        void force_close();
        int bytes_sent;
        int sock;
        std::mutex packets_mutex;
        std::mutex mutex_connected;
        std::condition_variable cv_connected;
        std::thread *worker_thread;
        std::queue<shared_ptr<Packet>> packets;
        std::deque<shared_ptr<ChunkData>> chunks;
        bool connected;
        std::string error_message;
    };
};


#define SHOWERROR(ErrMsg) { char aBuf[256]; snprintf(aBuf, 256, "At '%s:%d' in function '%s' occurred error '%s'",__FILE__,__LINE__,__FUNCTION__,ErrMsg); perror(aBuf); };

#endif

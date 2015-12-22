#ifndef _client_h_
#define _client_h_
#include <iostream>
#include <mutex>
#include <string>
#include <memory>
#include <queue>
#include <thread>
#include <Eigen/Geometry>

#define DEFAULT_PORT 4080

namespace konstructs {
    using namespace std;
    using namespace Eigen;

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
            std::cout << str << std::endl;
            return str;
        }
    private:
        char *mBuffer;
    };

    class Client {
    public:
        Client(const string &nick, const string &hash,
               const string &hostname, const int port = DEFAULT_PORT);
        void version(const int version, const string &nick, const string &hash);
        void position(const Vector3f position,
                      const float rx, const float ry);
        void chunk(const int amount);
        void konstruct();
        void click_inventory(const int item);
        void close_inventory();
        void talk(const string &text);
        void inventory_select(const int pos);
        void click_at(const int hit, const Vector3i pos, const int button);
        vector<shared_ptr<Packet>> receive(const int max);
        vector<shared_ptr<Packet>> receive_chunks(const int max);
    private:
        int send_all(const char *data, const int length);
        void send_string(const string &str);
        size_t recv_all(char* out_buf, const size_t size);
        void recv_worker();
        int bytes_sent;
        int sock;
        std::mutex packets_mutex;
        std::thread *worker_thread;
        std::queue<shared_ptr<Packet>> packets;
        std::queue<shared_ptr<Packet>> chunk_packets;
    };
};


#define SHOWERROR(ErrMsg) { char aBuf[256]; snprintf(aBuf, 256, "At '%s:%d' in function '%s' occurred error '%s'",__FILE__,__LINE__,__FUNCTION__,ErrMsg); perror(aBuf); };

#endif

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
        void click_at(const int hit, const int button, const int x, const int y, const int z);
        vector<shared_ptr<Packet>> receive(const int max);
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
    };
};


#define SHOWERROR(ErrMsg) { char aBuf[256]; snprintf(aBuf, 256, "At '%s:%d' in function '%s' occurred error '%s'",__FILE__,__LINE__,__FUNCTION__,ErrMsg); perror(aBuf); };


typedef struct {
    int size;
    int type;
    char *payload;
} Packet;

void client_enable();
void client_disable();
int get_client_enabled();
int check_server(char *server);
void client_connect(char *hostname, int port);
void client_start();
void client_stop();
void client_send(char *data);
int client_recv(Packet *r_packets, int r_size);
void client_version(int version, char *nick, char *hash);
void client_login(const char *username, const char *identity_token);
void client_position(float x, float y, float z, float rx, float ry);
void client_chunk(int amount);
void client_block(int x, int y, int z, int w);
void client_konstruct();
void client_click_inventory(int item);
void client_close_inventory();
void client_light(int x, int y, int z, int w);
void client_sign(int x, int y, int z, int face, const char *text);
void client_talk(const char *text);
void client_inventory_select(int pos);
void click_at(int hit, int button, int x, int y, int z);

#endif

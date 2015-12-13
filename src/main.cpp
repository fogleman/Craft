
#include <nanogui/nanogui.h>
#if defined(WIN32)
#include <windows.h>
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <nanogui/glutil.h>
#include <iostream>
#include <vector>
#include <memory>
#include "optional.hpp"
#include "matrix.h"
#include "shader.h"
#include "crosshair.h"
#include "block.h"
#include "chunk.h"
#include "world.h"
#include "chunk_shader.h"
#include "sky_shader.h"
#include "selection_shader.h"
#include "client.h"
#include "util.h"

#define KONSTRUCTS_APP_TITLE "Konstructs"
#define KONSTRUCTS_APP_WIDTH 854
#define KONSTRUCTS_APP_HEIGHT 480
#define MAX_PENDING_CHUNKS 256
#define KONSTRUCTS_KEY_FORWARD 'W'
#define KONSTRUCTS_KEY_BACKWARD 'S'
#define KONSTRUCTS_KEY_LEFT 'A'
#define KONSTRUCTS_KEY_RIGHT 'D'
#define KONSTRUCTS_KEY_JUMP GLFW_KEY_SPACE
#define KONSTRUCTS_KEY_FLY GLFW_KEY_TAB

using std::cout;
using std::cerr;
using std::endl;
using namespace konstructs;
using nonstd::optional;
using nonstd::nullopt;

class Konstructs: public nanogui::Screen {
public:
    Konstructs() :
        nanogui::Screen(Eigen::Vector2i(KONSTRUCTS_APP_WIDTH, KONSTRUCTS_APP_HEIGHT), KONSTRUCTS_APP_TITLE),
        crosshair(mSize.y(), mSize.x()),
        player(0, Vector3f(0.0f, 0.0f, 0.0f), 0.0f, 0.0f),
        px(0), py(0),
        model_factory(blocks),
        client("tetestte", "123456789", "localhost"),
        radius(10),
        fov(60.0f),
        near_distance(0.125f),
        sky(radius, fov, 2, near_distance),
        chunk(radius, fov, 0, 2, near_distance),
        selection(radius, fov, near_distance, 0.52),
        day_length(600),
        last_frame(glfwGetTime()),
        looking_at(nullopt),
        looking_through(nullopt){
        client.chunk(MAX_PENDING_CHUNKS);
        using namespace nanogui;
        performLayout(mNVGContext);
        glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        blocks.is_plant[SOLID_BLOCK] = 0;
        blocks.is_obstacle[SOLID_BLOCK] = 1;
        blocks.is_transparent[SOLID_BLOCK] = 0;
        memset(&fps, 0, sizeof(fps));
    }

    ~Konstructs() {
    }

    virtual bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) {
        if(looking_at) {
            if(button == GLFW_MOUSE_BUTTON_1 && down) {
                client.click_at(1, looking_at->position, 1);
            } else if(button == GLFW_MOUSE_BUTTON_2 && down) {
                client.click_at(1, looking_through->position, 2);
            } else if(button == GLFW_MOUSE_BUTTON_2 && down) {
                client.click_at(1, looking_at->position, 3);
            }
        }
        return Screen::mouseButtonEvent(p, button, down, modifiers);
    }

    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) {
        if (Screen::keyboardEvent(key, scancode, action, modifiers))
            return true;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            setVisible(false);
            return true;
        } else if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
            init_menu();
            performLayout(mNVGContext);
        } else if (key == KONSTRUCTS_KEY_FLY && action == GLFW_PRESS) {
            player.fly();
        }
        return false;
    }

    virtual void draw(NVGcontext *ctx) {
        Screen::draw(ctx);
    }

    virtual void drawContents() {
        using namespace nanogui;
        update_fps(&fps);
        handle_network();
        handle_keys();
        handle_mouse();
        looking_at = player.looking_at(world, false, blocks);
        looking_through = player.looking_at(world, true, blocks);
        glClear(GL_DEPTH_BUFFER_BIT);
        for(auto model : model_factory.fetch_models()) {
            chunk.add(model);
        }
        sky.render(player, mSize.x(), mSize.y(), time_of_day());
        glClear(GL_DEPTH_BUFFER_BIT);
        int faces = chunk.render(player, mSize.x(), mSize.y(), daylight(), time_of_day());
        if(looking_at) {
            selection.render(player, mSize.x(), mSize.y(), looking_at->position);
        }
        //cout << "Faces: " << faces << " FPS: " << fps.fps << endl;
        crosshair.render();
    }

private:

    void handle_mouse() {
        int exclusive =
             glfwGetInputMode(mGLFWWindow, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
        if (exclusive && (px || py)) {
            double mx, my;
            glfwGetCursorPos(mGLFWWindow, &mx, &my);
            float m = 0.0025;
            float drx = (mx - px) * m;
            float dry = (my - py) * m;

            player.rotate_x(dry);
            player.rotate_y(drx);
            px = mx;
            py = my;
        }
        else {
            glfwGetCursorPos(mGLFWWindow, &px, &py);
        }
    }

    void handle_keys() {
        int sx = 0;
        int sz = 0;
        bool jump = false;
        double now = glfwGetTime();
        double dt = now - last_frame;
        dt = MIN(dt, 0.2);
        dt = MAX(dt, 0.0);
        last_frame = now;

        if(glfwGetKey(mGLFWWindow, GLFW_KEY_W)) {
            sz--;
        }
        if(glfwGetKey(mGLFWWindow, GLFW_KEY_S)) {
            sz++;
        }
        if(glfwGetKey(mGLFWWindow, GLFW_KEY_A)) {
            sx--;
        }
        if(glfwGetKey(mGLFWWindow, GLFW_KEY_D)) {
            sx++;
        }
        if(glfwGetKey(mGLFWWindow, KONSTRUCTS_KEY_JUMP)) {
            jump = true;
        }
        client.position(player.update_position(sz, sx, (float)dt, world,
                                               blocks, near_distance, jump),
                        player.rx(), player.ry());
    }

    void handle_network() {
        for(auto packet : client.receive(10)) {
            handle_packet(packet);
        }
    }

    void handle_packet(shared_ptr<konstructs::Packet> packet) {
        switch(packet->type) {
        case 'U':
            handle_player_packet(packet->to_string());
            break;
        case 'W':
            handle_block_type(packet->to_string());
            break;
        case 'C':
            handle_chunk(packet);
            break;
        case 'M':
            handle_texture(packet);
            break;
        default:
            cout << "UNKNOWN: " << packet->type << endl;
            break;
        }
    }

    void handle_player_packet(const string &str) {
        int pid;
        float x, y, z, rx, ry;

        if(sscanf(str.c_str(), ",%d,%f,%f,%f,%f,%f",
                  &pid, &x, &y, &z, &rx, &ry) != 6)
            throw std::runtime_error(str);
        player = Player(pid, Vector3f(x, 50.0, z), rx, ry);
    }

    void handle_block_type(const string &str) {
        int w, obstacle, transparent, left, right, top, bottom, front, back;
        char shape[16];
        if(sscanf(str.c_str(), ",%d,%15[^,],%d,%d,%d,%d,%d,%d,%d,%d",
                  &w, shape, &obstacle, &transparent, &left, &right,
                  &top, &bottom, &front, &back) != 10)
            throw std::runtime_error(str);
        blocks.is_plant[w] = strncmp(shape, "plant", 16) == 0;
        blocks.is_obstacle[w] = obstacle;
        blocks.is_transparent[w] = transparent;
        blocks.blocks[w][0] = left;
        blocks.blocks[w][1] = right;
        blocks.blocks[w][2] = top;
        blocks.blocks[w][3] = bottom;
        blocks.blocks[w][4] = front;
        blocks.blocks[w][5] = back;
    }


    void handle_chunk(shared_ptr<konstructs::Packet> packet) {
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
        world.insert(position, chunk);
        model_factory.create_model(position, world);
        client.chunk(1);
    }

    void handle_texture(shared_ptr<konstructs::Packet> packet) {
        GLuint texture;
        glGenTextures(1, &texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        load_png_texture_from_buffer(packet->buffer(), packet->size);
    }

    float time_of_day() {
        if (day_length <= 0) {
            return 0.5;
        }
        float t;
        t = glfwGetTime();
        t = t / day_length;
        t = t - (int)t;
        return t;
    }

    float daylight() {
        float timer = time_of_day();
        if (timer < 0.5) {
            float t = (timer - 0.25) * 100;
            return 1 / (1 + powf(2, -t));
        }
        else {
            float t = (timer - 0.85) * 100;
            return 1 - 1 / (1 + powf(2, -t));
        }
    }


    void init_menu() {
        using namespace nanogui;

        Window *window = new Window(this, "Button demo");
        window->setPosition(Vector2i(15, 15));
        window->setLayout(new GroupLayout());
    }

    BlockData blocks;
    Crosshair crosshair;
    int radius;
    int fov;
    float near_distance;
    int day_length;
    World world;
    SkyShader sky;
    ChunkShader chunk;
    SelectionShader selection;
    ChunkModelFactory model_factory;
    Client client;
    Player player;
    optional<konstructs::Block> looking_at;
    optional<konstructs::Block> looking_through;
    double px;
    double py;
    FPS fps;
    double last_frame;
};

int main(int /* argc */, char ** /* argv */) {
    try {
        nanogui::init();

        {
            nanogui::ref<Konstructs> app = new Konstructs();
            app->drawAll();
            app->setVisible(true);
            nanogui::mainloop();
        }

        nanogui::shutdown();
    } catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        #if defined(WIN32)
            MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
        #else
            std::cerr << error_msg << endl;
        #endif
        return -1;
    }

    return 0;
}

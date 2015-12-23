
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
#include <utility>
#include "optional.hpp"
#include "matrix.h"
#include "shader.h"
#include "crosshair_shader.h"
#include "block.h"
#include "chunk.h"
#include "world.h"
#include "chunk_shader.h"
#include "sky_shader.h"
#include "selection_shader.h"
#include "hud.h"
#include "hud_shader.h"
#include "textures.h"
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
#define KONSTRUCTS_KEY_INVENTORY 'E'
using std::cout;
using std::cerr;
using std::endl;
using namespace konstructs;
using nonstd::optional;
using nonstd::nullopt;
using std::pair;

class Konstructs: public nanogui::Screen {
public:
    Konstructs() :
        nanogui::Screen(Eigen::Vector2i(KONSTRUCTS_APP_WIDTH, KONSTRUCTS_APP_HEIGHT), KONSTRUCTS_APP_TITLE),
        player(0, Vector3f(0.0f, 0.0f, 0.0f), 0.0f, 0.0f),
        px(0), py(0),
        model_factory(blocks),
        client("tetestte", "123456789", "localhost"),
        radius(10),
        fov(60.0f),
        near_distance(0.125f),
        sky_shader(radius, fov, SKY_TEXTURE, near_distance),
        chunk_shader(radius, fov, BLOCK_TEXTURES, SKY_TEXTURE, near_distance),
        hud_shader(17, 14, INVENTORY_TEXTURE, BLOCK_TEXTURES),
        selection_shader(radius, fov, near_distance, 0.52),
        day_length(600),
        last_frame(glfwGetTime()),
        looking_at(nullopt),
        hud(17, 14),
        hud_interaction(false) {
        load_textures();
        client.chunk(MAX_PENDING_CHUNKS);
        using namespace nanogui;
        performLayout(mNVGContext);
        glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        blocks.is_plant[SOLID_BLOCK] = 0;
        blocks.is_obstacle[SOLID_BLOCK] = 1;
        blocks.is_transparent[SOLID_BLOCK] = 0;
        memset(&fps, 0, sizeof(fps));
        hud.set_background(Vector2i(4, 0), 3);
        hud.set_background(Vector2i(5, 0), 3);
        hud.set_background(Vector2i(6, 0), 3);
        hud.set_background(Vector2i(7, 0), 3);
        hud.set_background(Vector2i(8, 0), 3);
        hud.set_background(Vector2i(9, 0), 3);
        hud.set_background(Vector2i(10, 0), 3);
        hud.set_background(Vector2i(11, 0), 3);
        hud.set_background(Vector2i(12, 0), 3);
    }

    ~Konstructs() {
    }

    virtual bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) {
        if(hud_interaction) {
            if(down) {
                double x, y;
                glfwGetCursorPos(mGLFWWindow, &x, &y);

                auto clicked_at = hud_shader.clicked_at(x, y, mSize.x(), mSize.y());

                if(clicked_at) {
                    Vector2i pos = *clicked_at;
                    if(hud.active(pos)) {
                        int index = pos[0] + pos[1] * 17;
                        client.click_inventory(index);
                    }
                }
            }
        } else {
            glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            if(looking_at) {
                auto &l = *looking_at;
                if(button == GLFW_MOUSE_BUTTON_1 && down) {
                    client.click_at(1, l.second.position, 1);
                } else if(button == GLFW_MOUSE_BUTTON_2 && down) {
                    client.click_at(1, l.first.position, 2);
                } else if(button == GLFW_MOUSE_BUTTON_2 && down) {
                    client.click_at(1, l.second.position, 3);
                }
            }
        }
        return Screen::mouseButtonEvent(p, button, down, modifiers);
    }

    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) {
        if (Screen::keyboardEvent(key, scancode, action, modifiers))
            return true;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            if(hud_interaction) {
                close_hud();
            } else {
                glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        } else if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
            init_menu();
            performLayout(mNVGContext);
        } else if (key == KONSTRUCTS_KEY_FLY && action == GLFW_PRESS) {
            player.fly();
        } else if(key == KONSTRUCTS_KEY_INVENTORY && action == GLFW_PRESS) {
            if(hud_interaction) {
                close_hud();
            } else {
                client.click_at(0, Vector3i::Zero(), 3);
            }
        } else if(key > 48 && key < 58 && action == GLFW_PRESS) {
            client.inventory_select(key - 49);
        } else {
            return false;
        }
        return true;
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
        looking_at = player.looking_at(world, blocks);
        glClear(GL_DEPTH_BUFFER_BIT);
        for(auto model : model_factory.fetch_models()) {
            chunk_shader.add(model);
        }
        sky_shader.render(player, mSize.x(), mSize.y(), time_of_day());
        glClear(GL_DEPTH_BUFFER_BIT);
        int faces = chunk_shader.render(player, mSize.x(), mSize.y(),
                                        daylight(), time_of_day());
        if(looking_at) {
            selection_shader.render(player, mSize.x(), mSize.y(),
                                    looking_at->second.position);
        }
        //cout << "Faces: " << faces << " FPS: " << fps.fps << endl;
        if(!hud_interaction)
            crosshair_shader.render(mSize.x(), mSize.y());
        hud_shader.render(mSize.x(), mSize.y(), hud, blocks.blocks);
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
        } else {
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

    void close_hud() {
        hud_interaction = false;
        glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        client.close_inventory();
        for(int i = 0; i < 17; i++) {
            for(int j = 1; j < 14; j++) {
                Vector2i pos(i, j);
                hud.reset_background(pos);
                hud.reset_stack(pos);
            }
        }
    }

    void handle_network() {
        for(auto packet : client.receive(100)) {
            handle_packet(packet.get());
        }
        for(auto chunk : client.receive_chunks(10)) {
            handle_chunk(chunk);
        }
    }

    void handle_packet(konstructs::Packet *packet) {
        switch(packet->type) {
        case 'U':
            handle_player_packet(packet->to_string());
            break;
        case 'W':
            handle_block_type(packet->to_string());
            break;
        case 'M':
            handle_texture(packet);
            break;
        case 'G':
            handle_belt(packet->to_string());
            break;
        case 'I':
            handle_inventory(packet->to_string());
            hud_interaction = true;
            glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            break;
        case 'A':
            handle_select_active(packet->to_string());
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


    void handle_chunk(const std::shared_ptr<ChunkData> chunk) {
        world.insert(chunk->position, chunk);
        model_factory.create_model(chunk->position, world);
        client.chunk(1);
    }

    void handle_texture(konstructs::Packet *packet) {
        GLuint texture;
        glGenTextures(1, &texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        load_png_texture_from_buffer(packet->buffer(), packet->size);
    }

    void handle_belt(const string &str) {
        int column, size, type;
        if(sscanf(str.c_str(), ",%d,%d,%d",
                  &column, &size, &type) != 3)
            throw std::runtime_error(str);
        Vector2i pos(column + 4, 0);

        if(size < 1) {
            hud.reset_stack(pos);
        } else {
            hud.set_stack(pos, {size, type});
        }
    }

    void handle_inventory(const string &str) {
        int index, size, type;
        if(sscanf(str.c_str(), ",%d,%d,%d",
                  &index, &size, &type) != 3)
            throw std::runtime_error(str);
        int row = index / 17;
        int column = index % 17;
        Vector2i pos(column, row);

        if(type == -1) {
            hud.reset_background(pos);
            hud.reset_stack(pos);
        } else {
            hud.set_background(pos, 2);
            hud.set_stack(pos, {size, type});
        }
    }

    void handle_select_active(const string &str) {
        int column;
        if(sscanf(str.c_str(), ",%d",
                  &column) != 1)
            throw std::runtime_error(str);
        for(int i = 0; i < 9; i++) {
            Vector2i pos(i + 4, 0);
            if(i == column) {
                hud.set_background(pos, 3);
            } else {
                hud.set_background(pos, 2);
            }
        }
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
    CrosshairShader crosshair_shader;
    int radius;
    int fov;
    float near_distance;
    int day_length;
    World world;
    SkyShader sky_shader;
    ChunkShader chunk_shader;
    SelectionShader selection_shader;
    HudShader hud_shader;
    ChunkModelFactory model_factory;
    Client client;
    Player player;
    optional<pair<konstructs::Block, konstructs::Block>> looking_at;
    Hud hud;
    bool hud_interaction;
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

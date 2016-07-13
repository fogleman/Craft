
#include <nanogui/nanogui.h>
#if defined(WIN32)
#define _WINSOCKAPI_ 
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
#include <stdexcept>
#include "tiny_obj_loader.h"
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
#include "player_shader.h"
#include "textures.h"
#include "client.h"
#include "util.h"
#include "cube.h"

#define KONSTRUCTS_APP_TITLE "Konstructs"
#define KONSTRUCTS_APP_WIDTH 854
#define KONSTRUCTS_APP_HEIGHT 480
#define MAX_PENDING_CHUNKS 64
#define KONSTRUCTS_KEY_FORWARD 'W'
#define KONSTRUCTS_KEY_BACKWARD 'S'
#define KONSTRUCTS_KEY_LEFT 'A'
#define KONSTRUCTS_KEY_RIGHT 'D'
#define KONSTRUCTS_KEY_JUMP GLFW_KEY_SPACE
#define KONSTRUCTS_KEY_FLY GLFW_KEY_TAB
#define KONSTRUCTS_KEY_SNEAK GLFW_KEY_LEFT_SHIFT
#define KONSTRUCTS_KEY_INVENTORY 'E'
#define MOUSE_CLICK_DELAY_IN_FRAMES 15

using std::cout;
using std::cerr;
using std::endl;
using namespace konstructs;
using nonstd::optional;
using nonstd::nullopt;
using std::pair;

void print_usage();

class Konstructs: public nanogui::Screen {
public:
    Konstructs(const string &hostname,
               const string &username,
               const string &password,
               bool debug_mode) :
        nanogui::Screen(Eigen::Vector2i(KONSTRUCTS_APP_WIDTH,
                                        KONSTRUCTS_APP_HEIGHT),
                        KONSTRUCTS_APP_TITLE),
        hostname(hostname),
        username(username),
        password(password),
        player(0, Vector3f(0.0f, 0.0f, 0.0f), 0.0f, 0.0f),
        px(0), py(0),
        model_factory(blocks),
        radius(5),
        max_radius(20),
        client(),
        view_distance((float)radius*CHUNK_SIZE),
        fov(70.0f),
        near_distance(0.125f),
        sky_shader(fov, SKY_TEXTURE, near_distance),
        chunk_shader(fov, BLOCK_TEXTURES, DAMAGE_TEXTURE, SKY_TEXTURE, near_distance,
                     load_chunk_vertex_shader(), load_chunk_fragment_shader()),
        hud_shader(17, 14, INVENTORY_TEXTURE, BLOCK_TEXTURES, FONT_TEXTURE, HEALTH_BAR_TEXTURE),
        selection_shader(fov, near_distance, 0.52),
        day_length(600),
        last_frame(glfwGetTime()),
        looking_at(nullopt),
        hud(17, 14, 9),
        menu_state(false),
        debug_mode(debug_mode),
        frame(0),
        click_delay(0) {

        using namespace nanogui;
        performLayout(mNVGContext);
        glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (username.size() > 0 && password.size() > 0 && hostname.size() > 0) {
            setup_connection();
        } else {
            show_menu(string("Connect to a server"));
        }
        blocks.is_plant[SOLID_TYPE] = 0;
        blocks.is_obstacle[SOLID_TYPE] = 1;
        blocks.is_transparent[SOLID_TYPE] = 0;
        blocks.state[SOLID_TYPE] = STATE_SOLID;
        memset(&fps, 0, sizeof(fps));

        tinyobj::shape_t shape = load_player();
        player_shader = new PlayerShader(fov, PLAYER_TEXTURE, SKY_TEXTURE,
                                         near_distance, shape);
    }

    ~Konstructs() {
        delete player_shader;
    }

    virtual bool scrollEvent(const Vector2i &p, const Vector2f &rel) {
        hud.scroll(rel[1]);
        return true;
    }

    virtual bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) {
        if(hud.get_interactive()) {
            if(down) {
                double x, y;
                glfwGetCursorPos(mGLFWWindow, &x, &y);

                auto clicked_at = hud_shader.clicked_at(x, y, mSize.x(), mSize.y());

                if(clicked_at) {
                    Vector2i pos = *clicked_at;
                    if(hud.active(pos)) {
                        int index = pos[0] + pos[1] * 17;
                        client.click_inventory(index, translate_button(button));
                    }
                }
            }
        } else if(!menu_state) {
            // Clicking at the window captures the mouse pointer
            glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        return Screen::mouseButtonEvent(p, button, down, modifiers);
    }

    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) {
        if (Screen::keyboardEvent(key, scancode, action, modifiers))
            return true;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            if(hud.get_interactive()) {
                close_hud();
            } else {
                glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        } else if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
            // TODO: implement this again when time has come ;)
            /*if (!menu_state) {
                show_menu("","","");
            } else {
                hide_menu();
            }*/
        } else if (key == KONSTRUCTS_KEY_FLY
                && action == GLFW_PRESS
                && debug_mode) {
            player.fly();
        } else if(key == KONSTRUCTS_KEY_INVENTORY && action == GLFW_PRESS) {
            if(hud.get_interactive()) {
                close_hud();
            } else if (client.is_connected()){
                if(looking_at) {
                    auto &l = *looking_at;
                    uint8_t direction = direction_from_vector(l.first.position, l.second.position);
                    client.click_at(1, l.second.position, 3, hud.get_selection(), direction);
                } else {
                    client.click_at(0, Vector3i::Zero(), 3, hud.get_selection(), 0);
                }
            }
        } else if(key > 48 && key < 58 && action == GLFW_PRESS) {
            hud.set_selected(key - 49);
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
        frame++;
        if (client.is_connected()) {
            handle_network();
            handle_keys();
            handle_mouse();
            looking_at = player.looking_at(world, blocks);
            glClear(GL_DEPTH_BUFFER_BIT);
            for(auto model : model_factory.fetch_models()) {
                chunk_shader.add(model);
            }
            sky_shader.render(player, mSize.x(), mSize.y(), time_of_day(), view_distance);
            glClear(GL_DEPTH_BUFFER_BIT);
            faces = chunk_shader.render(player, mSize.x(), mSize.y(),
                                        daylight(), time_of_day(), radius,
                                        view_distance, player_chunk);
            if(faces > max_faces)
                max_faces = faces;
            player_shader->render(player, mSize.x(), mSize.y(),
                                  daylight(), time_of_day(), view_distance);
            if(looking_at && !hud.get_interactive() && !menu_state) {
                selection_shader.render(player, mSize.x(), mSize.y(),
                                        looking_at->second.position, view_distance);
            }
            glClear(GL_DEPTH_BUFFER_BIT);
            if(!hud.get_interactive() && !menu_state)
                crosshair_shader.render(mSize.x(), mSize.y());
            double mx, my;
            glfwGetCursorPos(mGLFWWindow, &mx, &my);
            hud_shader.render(mSize.x(), mSize.y(), mx, my, hud, blocks);
            update_radius();
        } else if(!menu_state){
            show_menu(client.get_error_message());
        }
    }

private:

    int translate_button(int button) {
        switch(button) {
        case GLFW_MOUSE_BUTTON_1:
            return 1;
        case GLFW_MOUSE_BUTTON_2:
            return 2;
        case GLFW_MOUSE_BUTTON_3:
            return 3;
        }
    }

    bool update_view_distance() {
        double frame_fps = 1.15 / frame_time;

        if(frame_fps > 0.0 && frame_fps < 60.0 && radius > 1) {
            view_distance = view_distance - (float)CHUNK_SIZE * 0.2f * ((60.0f - (float)frame_fps) / 60.0f);
            return true;
        } else if(frame_fps >= 60.0
                && radius < max_radius
                && model_factory.waiting() == 0
                && radius <= client.get_loaded_radius()) {
            view_distance = view_distance + 0.05;
            return true;
        } else {
            return false;
        }
    }

    void update_radius() {
        if (update_view_distance()) {
            int new_radius = (int)(view_distance / (float)CHUNK_SIZE) + 1;
            radius = new_radius;
            client.set_radius(radius);
        }

        if(frame % 6 == 0) {
            double frame_fps = 1.15 / frame_time;
            cout << "View distance: " << view_distance << " (" << radius << "/" << client.get_loaded_radius() << ") faces: " << faces << "(" << max_faces << ") FPS: " << fps.fps << "(" << frame_fps << ")" << endl;
            cout << "Chunks: " << world.size() << " models: " << chunk_shader.size() << endl;
            cout << "Model factory, waiting: " << model_factory.waiting() << " created: " << model_factory.total_created() << " empty: " << model_factory.total_empty() << " total: " <<  model_factory.total() << endl;
        }
    }

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

            if(click_delay == 0) {
                if(looking_at) {
                    auto &l = *looking_at;
                    cout<<"FROM: " << l.first.position << " TO: " << l.second.position << endl;
                    cout<<"DIFF: " << (l.first.position - l.second.position) << endl;
                    uint8_t direction = direction_from_vector(l.first.position, l.second.position);
                    cout<<"DIR: " << (int)direction << endl;
                    if(glfwGetMouseButton(mGLFWWindow, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
                        click_delay = MOUSE_CLICK_DELAY_IN_FRAMES;
                        client.click_at(1, l.second.position, translate_button(GLFW_MOUSE_BUTTON_1), hud.get_selection(),
                                        direction);
                    } else if(glfwGetMouseButton(mGLFWWindow, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS &&
                              player.can_place(l.first.position, world, blocks)) {
                        optional<ItemStack> selected = hud.selected();
                        if(selected) {
                            std::shared_ptr<ChunkData> updated_chunk =
                                world.chunk_at(l.first.position)->set(l.first.position,
                                                                      {selected->type, selected->health, direction});
                            world.insert(updated_chunk);
                            model_factory.create_models({updated_chunk->position}, world);
                        }
                        click_delay = MOUSE_CLICK_DELAY_IN_FRAMES;
                        client.click_at(1, l.first.position, translate_button(GLFW_MOUSE_BUTTON_2), hud.get_selection(),
                                        direction);
                    } else if(glfwGetMouseButton(mGLFWWindow, GLFW_MOUSE_BUTTON_3) == GLFW_PRESS) {
                        click_delay = MOUSE_CLICK_DELAY_IN_FRAMES;
                        client.click_at(1, l.second.position, translate_button(GLFW_MOUSE_BUTTON_3), hud.get_selection(),
                                        direction);
                    }
                } else if(glfwGetMouseButton(mGLFWWindow, GLFW_MOUSE_BUTTON_3) == GLFW_PRESS) {
                        click_delay = MOUSE_CLICK_DELAY_IN_FRAMES;
                        client.click_at(0, Vector3i::Zero(), translate_button(GLFW_MOUSE_BUTTON_3), hud.get_selection(),
                                        0);
                }
            } else {
                click_delay--;
            }
        } else {
            glfwGetCursorPos(mGLFWWindow, &px, &py);
        }
    }

    void handle_keys() {
        int sx = 0;
        int sz = 0;
        bool jump = false;
        bool sneak = false;
        double now = glfwGetTime();
        double dt = now - last_frame;
        frame_time = now - last_frame;
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
        if(glfwGetKey(mGLFWWindow, KONSTRUCTS_KEY_SNEAK)) {
            sneak = true;
        }
        client.position(player.update_position(sz, sx, (float)dt, world,
                                              blocks, near_distance, jump, sneak),
                       player.rx(), player.ry());
        Vector3i new_chunk(chunked_vec(player.camera()));
        if(new_chunk != player_chunk) {
            player_chunk = new_chunk;
            client.set_player_chunk(player_chunk);
        }
    }

    void close_hud() {
        hud.set_interactive(false);
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
        Vector3f pos = player.position;

        auto prio = client.receive_prio_chunk(Vector3i(chunked(pos[0]), chunked(pos[2]), chunked(pos[1])));
        /* Insert prio chunk into world */
        if(prio) {
            world.insert(*prio);
            model_factory.create_models({(*prio)->position}, world);
        }
        auto new_chunks = client.receive_chunks(1);
        if(!new_chunks.empty()) {
            std::vector<Vector3i> positions;
            positions.reserve(new_chunks.size());
            for(auto chunk : new_chunks) {
                world.insert(chunk);
                positions.push_back(chunk->position);
            }
            model_factory.create_models(positions, world);
        }
        if(frame % 7883 == 0) {
            /* Book keeping */
            world.delete_unused_chunks(player_chunk, radius + KEEP_EXTRA_CHUNKS);
        }

    }

    void handle_packet(konstructs::Packet *packet) {
        switch(packet->type) {
        case 'P':
            handle_other_player_packet(packet->to_string());
            break;
        case 'D':
            handle_delete_other_player_packet(packet->to_string());
            break;
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
            hud.set_interactive(true);
            glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            break;
        case 'i':
            handle_held_stack(packet->to_string());
            break;
        case 'T':
            handle_time(packet->to_string());
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
        player = Player(pid, Vector3f(x, y, z), rx, ry);
        player_chunk = chunked_vec(player.camera());
        client.set_player_chunk(player_chunk);
        client.set_radius(radius);
        client.set_logged_in(true);
    }

    void handle_other_player_packet(const string &str) {
        int pid;
        float x, y, z, rx, ry;
        if(sscanf(str.c_str(), ",%d,%f,%f,%f,%f,%f",
                  &pid, &x, &y, &z, &rx, &ry) != 6)
            throw std::runtime_error(str);
        player_shader->add(Player(pid, Vector3f(x, y, z), rx, ry));
    }

    void handle_delete_other_player_packet(const string &str) {
        int pid;

        if(sscanf(str.c_str(), ",%d",
                  &pid) != 1)
            throw std::runtime_error(str);
        player_shader->remove(pid);
    }

    void handle_block_type(const string &str) {
        int w, obstacle, transparent, left, right, top, bottom, front, back;
        char shape[16];
        char state[16];
        if(sscanf(str.c_str(), ",%d,%15[^,],%15[^,],%d,%d,%d,%d,%d,%d,%d,%d",
                  &w, shape, state, &obstacle, &transparent, &left, &right,
                  &top, &bottom, &front, &back) != 11)
            throw std::runtime_error(str);
        blocks.is_plant[w] = strncmp(shape, "plant", 16) == 0;
        if(strncmp(state, "solid", 16) == 0) {
            blocks.state[w] = STATE_SOLID;
        } else if(strncmp(state, "liquid", 16) == 0) {
            blocks.state[w] = STATE_LIQUID;
        } else if(strncmp(state, "gas", 16) == 0) {
            blocks.state[w] = STATE_GAS;
        } else if(strncmp(state, "plasma", 16) == 0) {
            blocks.state[w] = STATE_PLASMA;
        } else {
            throw std::invalid_argument("Invalid block type state received!");
        }
        blocks.is_obstacle[w] = obstacle;
        blocks.is_transparent[w] = transparent;
        blocks.blocks[w][0] = left;
        blocks.blocks[w][1] = right;
        blocks.blocks[w][2] = top;
        blocks.blocks[w][3] = bottom;
        blocks.blocks[w][4] = front;
        blocks.blocks[w][5] = back;
    }

    void handle_texture(konstructs::Packet *packet) {
        GLuint texture;
        glGenTextures(1, &texture);
        glActiveTexture(GL_TEXTURE0 + BLOCK_TEXTURES);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        load_png_texture_from_buffer(packet->buffer(), packet->size);
    }

    void handle_belt(const string &str) {
        uint32_t column, size, type, health;
        if(sscanf(str.c_str(), ",%u,%u,%u,%u",
                  &column, &size, &type, &health) != 4)
            throw std::runtime_error(str);

        if(size < 1) {
            hud.reset_belt(column);
        } else {
            hud.set_belt(column, {size, (uint16_t)type, (uint16_t)health});
        }
    }

    void handle_inventory(const string &str) {
        uint32_t index, size, type, health;
        if(sscanf(str.c_str(), ",%u,%u,%u,%u",
                  &index, &size, &type, &health) != 4)
            throw std::runtime_error(str);
        uint32_t row = index / 17;
        uint32_t column = index % 17;
        Vector2i pos(column, row);

        if(type == -1) {
            hud.reset_background(pos);
            hud.reset_stack(pos);
        } else {
            hud.set_background(pos, 2);
            hud.set_stack(pos, {size, (uint16_t)type, (uint16_t)health});
        }
    }

    void handle_held_stack(const string &str) {
        uint32_t amount, type;
        if(sscanf(str.c_str(), ",%u,%u",
                  &amount, &type) != 2)
            throw std::runtime_error(str);
        if(type == -1) {
            hud.reset_held();
        } else {
            hud.set_held({amount, (uint16_t)type});
        }
    }

    void handle_time(const string &str) {
        long time_value;
        if(sscanf(str.c_str(), ",%lu", &time_value) != 1) {
            throw std::runtime_error(str);
        }
        glfwSetTime((double)time_value);
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


    void show_menu(string message) {
        using namespace nanogui;

        glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glActiveTexture(GL_TEXTURE0);

        FormHelper *gui = new FormHelper(this);
        window = gui->addWindow({0,0}, "Main Menu");
        gui->setFixedSize({125, 20});
        gui->addGroup(message);
        gui->addVariable("Server address", hostname);
        gui->addVariable("Username", username);
        gui->addVariable("Password", password);
        gui->addButton("Connect", [&](){
                if (username != "" &&
                    password != "" &&
                    hostname != "") {
                    hide_menu();
                    setup_connection();
                }
            });

        window->center();
        performLayout(mNVGContext);
        menu_state = true;
    }

    void hide_menu() {
        glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        window->dispose();
        menu_state = false;
    }


    void setup_connection() {
        try {
            client.open_connection(username, password, hostname);
            load_textures();
            client.set_connected(true);
        } catch(const std::exception& ex) {
            show_menu(client.get_error_message());
        }
    }

    std::string hostname;
    std::string username;
    std::string password;
    BlockTypeInfo blocks;
    CrosshairShader crosshair_shader;
    int radius;
    int max_radius;
    float view_distance;
    int fov;
    float near_distance;
    int day_length;
    World world;
    SkyShader sky_shader;
    ChunkShader chunk_shader;
    SelectionShader selection_shader;
    HudShader hud_shader;
    PlayerShader *player_shader;
    ChunkModelFactory model_factory;
    Client client;
    Player player;
    Vector3i player_chunk;
    optional<pair<konstructs::Block, konstructs::Block>> looking_at;
    Hud hud;
    double px;
    double py;
    FPS fps;
    double last_frame;
    bool menu_state;
    bool debug_mode;
    nanogui::Window *window;
    uint32_t frame;
    uint32_t faces;
    uint32_t max_faces;
    double frame_time;
    uint32_t click_delay;
};

#ifdef WIN32
int init_winsock() {
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        printf("WSAStartup failed with error: %d\n", err);
        return 1;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        printf("Could not find a usable version of Winsock.dll\n");
        WSACleanup();
        return 1;
    }

    return 0;
}
#else
int init_winsock() { return 0; }
#endif

void print_usage() {
    printf("OPTIONS: -h/--help                  - Show this help\n");
    printf("         -s/--server   <address>    - Server to enter\n");
    printf("         -u/--username <username>   - Username to login\n");
    printf("         -p/--password <password>   - Passworld to login\n\n");
    exit(0);
}

int main(int argc, char ** argv) {
    std::string hostname = "play.konstructs.org";
    std::string username = "";
    std::string password = "";
    bool debug_mode = false;

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
                print_usage();
            }
            if (strcmp(argv[i], "--server") == 0 || strcmp(argv[i], "-s") == 0) {
                if (!argv[i+1]) {
                    print_usage();
                } else {
                    hostname = argv[i+1];
                    ++i;
                }
            }
            if (strcmp(argv[i], "--username") == 0 || strcmp(argv[i], "-u") == 0) {
                if (!argv[i+1]) {
                    print_usage();
                } else {
                    username = argv[i+1];
                    ++i;
                }
            }
            if (strcmp(argv[i], "--password") == 0 || strcmp(argv[i], "-p") == 0) {
                if (!argv[i+1]) {
                    print_usage();
                } else {
                    password = argv[i+1];
                    ++i;
                }
            }
            if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0) {
                debug_mode = true;
            }
        }

    }

    if (init_winsock()) {
        printf("Failed to load winsock");
        return 1;
    }

    try {
        nanogui::init();

        {
            nanogui::ref<Konstructs> app = new Konstructs(hostname, username, password, debug_mode);
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

#ifdef WIN32
    WSACleanup();
#endif
    return 0;
}

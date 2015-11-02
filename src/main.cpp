
#include <nanogui/nanogui.h>
#if defined(WIN32)
#include <windows.h>
#endif
#include <nanogui/glutil.h>
#include <iostream>
#include <vector>
#include <memory>
#include "matrix.h"
#include "shader.h"
#include "crosshair.h"
#include "chunk.h"
#include "client.h"

#define KONSTRUCTS_APP_TITLE "Konstructs"
#define KONSTRUCTS_APP_WIDTH 1024
#define KONSTRUCTS_APP_HEIGHT 768

using std::cout;
using std::cerr;
using std::endl;
using namespace konstructs;

class CubeData {
public:
    CubeData(GLuint name, MatrixXf m, float i) :
        data(std::make_shared<EigenAttribute>(name, m)),
        intensity(i),
        size(m.cols())
    {}
    const std::shared_ptr<EigenAttribute> data;
    const float intensity;
    const GLuint size;
};

class Cube : public ShaderProgram {
public:
    Cube() :
        ShaderProgram(
            "cube",
            "#version 330\n"
            "uniform mat4 modelViewProj;\n"
            "in vec3 position;\n"
            "void main() {\n"
            "    gl_Position = modelViewProj * vec4(position, 1.0);\n"
            "}",
            "#version 330\n"
            "out vec4 color;\n"
            "uniform float intensity;\n"
            "void main() {\n"
            "    color = vec4(vec3(intensity), 1.0);\n"
            "}"),
        position(attributeId("position")),
        modelViewProj(uniformId("modelViewProj")),
        intensity(uniformId("intensity")) {}
    const GLuint position;
    const GLuint modelViewProj;
    const GLuint intensity;

    void render(std::vector<CubeData> &cubes, int width, int height) {
        Matrix4f mvp = matrix::projection_2d(width, height);

        bind([&](Context c) {
                c.set(modelViewProj, mvp);
                for(auto d : cubes) {
                    c.set(intensity, d.intensity);
                    c.render(d.data, 0, d.size);
                }
            });
    }
};

class Konstructs: public nanogui::Screen {
public:
    Konstructs() :
        nanogui::Screen(Eigen::Vector2i(KONSTRUCTS_APP_WIDTH, KONSTRUCTS_APP_HEIGHT), KONSTRUCTS_APP_TITLE),
        crosshair(mSize.y(), mSize.x()),
        player(0, Vector3f(0.0f, 0.0f, 0.0f), 0.0f, 0.0f),
        px(0), py(0),
        client("tetestte", "123456789", "dev.konstructs.org") {
        using namespace nanogui;

        float *data =new float[10 * 3];
        float *d = data;
        *(d++) = 0.0f - 0.5f;
        *(d++) = 0.0f + 0.5f;
        *(d++) = 0.0f;
        *(d++) = 0.0f;
        *(d++) = 0.0f;
        *(d++) = 1.0f;
        *(d++) = 0.0f;
        *(d++) = 0.0f;
        *(d++) = 0.5f;
        *(d++) = 0.5f;
        *(d++) = 0.0f + 0.5f;
        *(d++) = 0.0f + 0.5f;
        *(d++) = 0.0f;
        *(d++) = 0.0f;
        *(d++) = 0.0f;
        *(d++) = 1.0f;
        *(d++) = 0.0f;
        *(d++) = 0.0f;
        *(d++) = 0.5f;
        *(d++) = 0.5f;
        *(d++) = 0.0f + 0.5f;
        *(d++) = 0.0f - 0.5f;
        *(d++) = 0.0f;
        *(d++) = 0.0f;
        *(d++) = 0.0f;
        *(d++) = 1.0f;
        *(d++) = 0.0f;
        *(d++) = 0.0f;
        *(d++) = 0.5f;
        *(d++) = 0.5f;

        chunk.add(Vector3f(3.0f, 0.0f, -15.0f), data, 3);
        chunk.add(Vector3f(0.0f, 0.0f, -100.0f), data, 3);
        delete[] data;
        performLayout(mNVGContext);
        glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    ~Konstructs() {
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
        }
        return false;
    }

    virtual void draw(NVGcontext *ctx) {
        Screen::draw(ctx);
    }

    virtual void drawContents() {
        using namespace nanogui;
        handle_network();
        handle_keys();
        handle_mouse();
        // cube.render(cubes, mSize.y(), mSize.x());
        chunk.render(player, mSize.y(), mSize.x());
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
        player.update_position(sz, sx);

    }

    void handle_network() {
        for(auto packet : client.receive(10)) {
            handle_packet(packet);
        }
    }

    void handle_packet(shared_ptr<konstructs::Packet> packet) {
        cout << packet->type;
        switch(packet->type) {
        case 'U':
            handle_player_packet(packet->to_string());
            break;
        case 'W':
            handle_block_type(packet->to_string());
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
    }

    void handle_block_type(const string &str) {
        int w, obstacle, transparent, left, right, top, bottom, front, back;
        char shape[16];
        if(sscanf(str.c_str(), ",%d,%15[^,],%d,%d,%d,%d,%d,%d,%d,%d",
                  &w, shape, &obstacle, &transparent, &left, &right,
                  &top, &bottom, &front, &back) != 10)
            throw std::runtime_error(str);
        is_plant[w] = strncmp(shape, "plant", 16) == 0;
        is_obstacle[w] = obstacle;
        is_transparent[w] = transparent;
        blocks[w][0] = left;
        blocks[w][1] = right;
        blocks[w][2] = top;
        blocks[w][3] = bottom;
        blocks[w][4] = front;
        blocks[w][5] = back;
    }

    void init_menu() {
        using namespace nanogui;

        Window *window = new Window(this, "Button demo");
        window->setPosition(Vector2i(15, 15));
        window->setLayout(new GroupLayout());
    }

    Cube cube;
    Crosshair crosshair;
    ChunkShader chunk;
    Client client;
    Player player;
    double px;
    double py;
    std::vector<CubeData> cubes;
    int blocks[256][6];
    char is_plant[256];
    char is_obstacle[256];
    char is_transparent[256];
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

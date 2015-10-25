
#include <nanogui/nanogui.h>
#if defined(WIN32)
#include <windows.h>
#endif
#include <nanogui/glutil.h>
#include <iostream>
#include <vector>
#include <memory>
#include "shader.h"
#define KONSTRUCTS_APP_TITLE "Konstructs"
#define KONSTRUCTS_APP_WIDTH 1024
#define KONSTRUCTS_APP_HEIGHT 768

using std::cout;
using std::cerr;
using std::endl;
using namespace konstructs;

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
};

class Konstructs: public nanogui::Screen {
public:
    Konstructs() : nanogui::Screen(Eigen::Vector2i(KONSTRUCTS_APP_WIDTH, KONSTRUCTS_APP_HEIGHT), KONSTRUCTS_APP_TITLE)
    {
        using namespace nanogui;

        MatrixXf positions(3, 6);
        positions.col(0) << 0, -2, 0;
        positions.col(1) <<  1, -2, 0;
        positions.col(2) <<  1,  1, 0;
        positions.col(3) <<  1,  1, 0;
        positions.col(4) << 0,  1, 0;
        positions.col(5) << 0, -2, 0;

        data.push_back(ShaderData({ std::make_shared<Uniform1f>(cube.intensity, 0.5f) },
                                  { std::make_shared<Attribute>(cube.position, positions) },
                                  6));

        MatrixXf positions2(3, 3);
        positions2.col(0) << 1, -2, 0;
        positions2.col(1) <<  2, -3, 0;
        positions2.col(2) <<  2,  1, 0;

        data.push_back(ShaderData({ std::make_shared<Uniform1f>(cube.intensity, 0.7f) },
                                  { std::make_shared<Attribute>(cube.position, positions2) },
                                  3));

        performLayout(mNVGContext);
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
        Matrix4f mvp;
        mvp.setIdentity();
        mvp.topLeftCorner<3,3>() = Matrix3f(Eigen::AngleAxisf((float) glfwGetTime(),  Vector3f::UnitZ())) * 0.25f;

        mvp.row(0) *= (float) mSize.y() / (float) mSize.x();
        std::vector<std::shared_ptr<Uniform>> uniforms = {
            std::make_shared<UniformMatrix4fv>(cube.modelViewProj, mvp)
        };
        for(auto d : data) {
            cube.render(d, uniforms);
        }
    }

private:

    void init_menu() {
        using namespace nanogui;

        Window *window = new Window(this, "Button demo");
        window->setPosition(Vector2i(15, 15));
        window->setLayout(new GroupLayout());
    }

    Cube cube;
    std::vector<ShaderData> data;
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

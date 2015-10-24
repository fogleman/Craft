
#include <nanogui/nanogui.h>
#if defined(WIN32)
#include <windows.h>
#endif
#include <nanogui/glutil.h>
#include <iostream>
#include <vector>

#define KONSTRUCTS_APP_TITLE "Konstructs"
#define KONSTRUCTS_APP_WIDTH 1024
#define KONSTRUCTS_APP_HEIGHT 768

using std::cout;
using std::cerr;
using std::endl;

class KonstructsShaders {
public:
    KonstructsShaders() {}
    ~KonstructsShaders() {
        for(std::vector<nanogui::GLShader*>::iterator it = shaders.begin(); it != shaders.end(); ++it) {
            cout << "Free shader" << endl;
            (*it)->free();
        }
    }

    nanogui::GLShader* add(const char* name, const char* vertex, const char* fragment) {
        cout << "Add shader " + (std::string)name << endl;
        nanogui::GLShader *shader = new nanogui::GLShader();
        shader->init(name, vertex, fragment);
        shaders.push_back(shader);
        return shader;
    }

    std::vector<nanogui::GLShader*>::iterator begin() {
        return (std::vector<nanogui::GLShader*>::iterator)shaders.begin();
    }

    std::vector<nanogui::GLShader*>::iterator end() {
        return (std::vector<nanogui::GLShader*>::iterator)shaders.end();
    }

private:
    std::vector<nanogui::GLShader*> shaders;
};

class Konstructs: public nanogui::Screen {
public:
    Konstructs() : nanogui::Screen(Eigen::Vector2i(KONSTRUCTS_APP_WIDTH, KONSTRUCTS_APP_HEIGHT), KONSTRUCTS_APP_TITLE) {
        using namespace nanogui;

        shaders = new KonstructsShaders();

        cube1();
        cube2();

        performLayout(mNVGContext);
    }

    void cube1() {
        using namespace nanogui;

        GLShader* shader = shaders->add(
            /* An identifying name */
            "a_simple_shader",

            /* Vertex shader */
            "#version 330\n"
            "uniform mat4 modelViewProj;\n"
            "in vec3 position;\n"
            "void main() {\n"
            "    gl_Position = modelViewProj * vec4(position, 1.0);\n"
            "}",

            /* Fragment shader */
            "#version 330\n"
            "out vec4 color;\n"
            "uniform float intensity;\n"
            "void main() {\n"
            "    color = vec4(vec3(intensity), 1.0);\n"
            "}"
        );

        MatrixXu indices(3, 2); /* Draw 2 triangles */
        indices.col(0) << 0, 1, 2;
        indices.col(1) << 2, 3, 0;

        MatrixXf positions(3, 4);
        positions.col(0) << -1, -1, 0;
        positions.col(1) <<  1, -1, 0;
        positions.col(2) <<  1,  1, 0;
        positions.col(3) << -1,  1, 0;

        shader->bind();
        shader->uploadIndices(indices);
        shader->uploadAttrib("position", positions);
        shader->setUniform("intensity", 0.5f);
    }

    void cube2() {
        using namespace nanogui;

        GLShader* shader = shaders->add(
            /* An identifying name */
            "a_simple_shader_2",

            /* Vertex shader */
            "#version 330\n"
            "uniform mat4 modelViewProj;\n"
            "in vec3 position;\n"
            "void main() {\n"
            "    gl_Position = modelViewProj * vec4(position, 1.0);\n"
            "}",

            /* Fragment shader */
            "#version 330\n"
            "out vec4 color;\n"
            "uniform float intensity;\n"
            "void main() {\n"
            "    color = vec4(vec3(intensity), 1.0);\n"
            "}"
        );

        MatrixXu indices(3, 2); /* Draw 2 triangles */
        indices.col(0) << 0, 1, 2;
        indices.col(1) << 2, 3, 0;

        MatrixXf positions(3, 4);
        positions.col(0) << -0.5, -0.5, 0;
        positions.col(1) <<  0.5, -0.5, 0;
        positions.col(2) <<  0.5,  0.5, 0;
        positions.col(3) << -0.5,  0.5, 0;

        shader->bind();
        shader->uploadIndices(indices);
        shader->uploadAttrib("position", positions);
        shader->setUniform("intensity", 0.7f);
    }


    ~Konstructs() {
        delete shaders;
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

        for(std::vector<GLShader*>::iterator it = shaders->begin(); it != shaders->end(); ++it) {
            GLShader *shader = *it;
            shader->bind();

            Matrix4f mvp;
            mvp.setIdentity();
            mvp.topLeftCorner<3,3>() = Matrix3f(Eigen::AngleAxisf((float) glfwGetTime(),  Vector3f::UnitZ())) * 0.25f;

            mvp.row(0) *= (float) mSize.y() / (float) mSize.x();

            shader->setUniform("modelViewProj", mvp);
            shader->drawIndexed(GL_TRIANGLES, 0, 2);
        }

    }

private:

    void init_menu() {
        using namespace nanogui;

        Window *window = new Window(this, "Button demo");
        window->setPosition(Vector2i(15, 15));
        window->setLayout(new GroupLayout());
    }

    KonstructsShaders *shaders;
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

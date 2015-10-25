#ifndef __SHADER_H__
#define __SHADER_H__
#include <string>
#include <memory>
#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <Eigen/Geometry>

namespace konstructs {
    using namespace Eigen;

    GLuint creater_shader(const GLint type, const std::string &shader);
    GLint attrib(const std::string &name);

    class Uniform {
    public:
        Uniform(const GLuint _name):
            name(_name) {}
        const GLuint name;
        virtual void set() const {}
    };

    class Uniform1f : public Uniform {
    public:
        Uniform1f(const GLuint _name, const float _value) :
            Uniform(_name),
            value(_value) {}
        void set() const;
    private:
        const float value;
    };

    class  UniformMatrix4fv : public Uniform {
    public:
        UniformMatrix4fv(const GLuint _name, const Matrix4f &_value) :
            Uniform(_name),
            value(_value) {}
        void set() const;
    private:
        const Matrix4f value;
    };


    class Attribute {
    public:
        template <typename Matrix> Attribute(const GLuint _name,
                                             const Matrix &m):
            name(_name),
            type((GLuint) nanogui::type_traits<typename Matrix::Scalar>::type),
            integral((bool) nanogui::type_traits<typename Matrix::Scalar>::integral),
            dim(m.rows()),
            size(m.cols()) {
            uint32_t compSize = sizeof(typename Matrix::Scalar);
            GLuint glType = (GLuint) nanogui::type_traits<typename Matrix::Scalar>::type;

            glGenBuffers(1, &mBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
            glBufferData(GL_ARRAY_BUFFER, m.size() * compSize, m.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        const GLuint name;
        const GLuint type;
        const bool integral;
        const GLuint dim;
        const GLuint size;
        const GLuint buffer() const {
            return mBuffer;
        }
    private:
        GLuint mBuffer;
    };

    class ShaderData {
    public:
        ShaderData(const std::vector<std::shared_ptr<Uniform>> _uniforms,
                   const std::vector<std::shared_ptr<Attribute>> _attributes,
                   const GLuint _size):
            uniforms(_uniforms),
            attributes(_attributes),
            size(_size) {
            for(auto attribute : attributes) {
                if(attribute->size != size)
                    throw std::invalid_argument("Invalid attribute size");
            }
        }
        const std::vector<std::shared_ptr<Uniform>> uniforms;
        const std::vector<std::shared_ptr<Attribute>> attributes;
        const GLuint size;
    };

    class ShaderProgram {
    public:
        ShaderProgram(const std::string &shader_name,
                      const std::string &vertex_shader,
                      const std::string &fragment_shader):
            name(shader_name),
            vertex(creater_shader(GL_VERTEX_SHADER, vertex_shader)),
            fragment(creater_shader(GL_FRAGMENT_SHADER, fragment_shader)),
            program(glCreateProgram()) {
            glAttachShader(program, vertex);
            glAttachShader(program, fragment);
            glLinkProgram(program);
            GLint status;
            glGetProgramiv(program, GL_LINK_STATUS, &status);
            if (status != GL_TRUE) {
                char buffer[512];
                glGetProgramInfoLog(program, 512, nullptr, buffer);
                std::cerr << "Linker error: " << std::endl << buffer << std::endl;
                throw std::runtime_error("Shader linking failed!");
            }
            glGenVertexArrays(1, &vao);
            glUseProgram(program);
        }
        const std::string &name;
        const GLuint vertex;
        const GLuint fragment;
        const GLuint program;
        void render(ShaderData &data, std::vector<std::shared_ptr<Uniform>> uniforms);
    protected:
        GLuint uniformId(const std::string &uName);
        GLuint attributeId(const std::string &aName);
    private:
        GLuint vao;
    };

};

#endif

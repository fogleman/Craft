#ifndef __SHADER_H__
#define __SHADER_H__
#include <string>
#include <memory>
#include <iostream>
#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <Eigen/Geometry>

namespace konstructs {
    using namespace Eigen;

    GLuint creater_shader(const GLint type, const std::string &shader);

    class Model {
    public:
        virtual void bind() = 0;
        virtual int vertices() = 0;
    };

    class BufferModel : public Model {
    public:
        ~BufferModel();
    protected:
        GLuint buffer;
    };

    class EigenModel : public BufferModel {
    public:
        template <typename Matrix> EigenModel(const GLuint name,
                                              const Matrix &m):
            name(name),
            type((GLuint) nanogui::type_traits <typename Matrix::Scalar>::type),
            integral((bool) nanogui::type_traits<typename Matrix::Scalar>::integral),
            dim(m.rows()),
            columns(m.cols()) {
            uint32_t compSize = sizeof(typename Matrix::Scalar);
            GLuint glType = (GLuint) nanogui::type_traits<typename Matrix::Scalar>::type;

            glGenBuffers(1, &buffer);
            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            glBufferData(GL_ARRAY_BUFFER, m.size() * compSize, m.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        virtual void bind();
        virtual int vertices();
        const GLuint name;
    private:
        const GLuint type;
        const bool integral;
        const GLuint dim;
        const int columns;
    };

    class Context {
    public:
        Context(const GLenum _draw_mode) :
            draw_mode(_draw_mode) {}
        void draw(Model *model);
        void draw(Model &model);
        void set(const GLuint name, const float value);
        void set(const GLuint name, const Matrix4f &value);
        void set(const GLuint name, const int value);
        void set(const GLuint name, const Vector2f &v);
        void set(const GLuint name, const Vector3f &v);
        void set(const GLuint name, const Vector4f &v);
        void logic_op(const GLenum opcode);
        void enable(const GLenum cap);
        void disable(const GLenum cap);
    private:
        const GLenum draw_mode;
    };

    class ShaderProgram {
    public:
        ShaderProgram(const std::string &shader_name,
                      const std::string &vertex_shader,
                      const std::string &fragment_shader,
                      const GLenum _draw_mode = GL_TRIANGLES);
        ~ShaderProgram();

        const std::string &name;
        const GLuint vertex;
        const GLuint fragment;
        const GLuint program;
        const GLenum draw_mode;
        void bind(std::function<void(Context context)> f);
    protected:
        GLuint uniformId(const std::string &uName);
        GLuint attributeId(const std::string &aName);
    private:
        GLuint vao;
    };

};

#endif

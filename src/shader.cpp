#include <string>
#include "shader.h"

namespace konstructs {
    GLuint creater_shader(const GLint type, const std::string &shader) {
        if (shader.empty())
            throw std::invalid_argument( "Shader code must not be empty" );
        GLuint id = glCreateShader(type);
        const char *shader_const = shader.c_str();
        glShaderSource(id, 1, &shader_const, nullptr);
        glCompileShader(id);

        GLint status;
        glGetShaderiv(id, GL_COMPILE_STATUS, &status);

        if (status != GL_TRUE) {
            char buffer[512];
            if (type == GL_VERTEX_SHADER)
                std::cerr << "Vertex shader:" << std::endl;
            else if (type == GL_FRAGMENT_SHADER)
                std::cerr << "Fragment shader:" << std::endl;
            else if (type == GL_GEOMETRY_SHADER)
                std::cerr << "Geometry shader:" << std::endl;
            std::cerr << shader << std::endl << std::endl;
            glGetShaderInfoLog(id, 512, nullptr, buffer);
            std::cerr << "Error: " << std::endl << buffer << std::endl;
            throw std::runtime_error("Shader compilation failed!");
        }

        return id;
    }

    ShaderProgram::ShaderProgram(const std::string &shader_name,
                                 const std::string &vertex_shader,
                                 const std::string &fragment_shader,
                                 const GLenum _draw_mode):
        name(shader_name),
        vertex(creater_shader(GL_VERTEX_SHADER, vertex_shader)),
        fragment(creater_shader(GL_FRAGMENT_SHADER, fragment_shader)),
        program(glCreateProgram()),
        draw_mode(_draw_mode) {
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
    ShaderProgram::~ShaderProgram() {
        glDeleteVertexArrays(1, &vao);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        glDeleteProgram(program);
    }

    GLuint ShaderProgram::uniformId(const std::string &uName) {
        GLint id = glGetUniformLocation(program, uName.c_str());
        if (id == -1) {
            std::cerr << name << "Warning: did not find uniform " << uName << std::endl;
            throw std::runtime_error("Could not find uniform for program");
        }
        return id;
    }

    GLuint ShaderProgram::attributeId(const std::string &aName) {
        GLint id = glGetAttribLocation(program, aName.c_str());
        if (id == -1) {
            std::cerr << name << ": warning: did not find attrib " << aName << std::endl;
            throw std::runtime_error("Could not find attribute for program");
        }
        return id;
    }

    void ShaderProgram::bind(std::function<void(Context context)> f) {
        glUseProgram(program);
        glBindVertexArray(vao);
        Context c(draw_mode);
        f(c);
    }

    void Context::draw(Attribute *attribute,
                       const GLuint offset, const GLuint size) {
        attribute->bind();
        glDrawArrays(draw_mode, offset, size);
    }

    void Context::draw(std::shared_ptr<Attribute> attribute,
                       const GLuint offset, const GLuint size) {
        std::vector<std::shared_ptr<Attribute>> attributes = { attribute };
        draw(attributes, offset, size);
    }

    void Context::draw(const std::vector<std::shared_ptr<Attribute>> &attributes,
                       const GLuint offset, const GLuint size) {
        for(auto attribute : attributes) {
            attribute->bind();
        }
        glDrawArrays(draw_mode, offset, size);
    }

    void Context::set(const GLuint name, const float value) {
        glUniform1f(name, value);
    }

    void Context::set(const GLuint name, const Matrix4f &value) {
        glUniformMatrix4fv(name, 1, GL_FALSE, value.data());
    }

    void Context::set(const GLuint name, const int value) {
        glUniform1i(name, value);
    }

    void Context::set(const GLuint name, const Vector2f &v) {
        glUniform2f(name, v.x(), v.y());
    }

    void Context::set(const GLuint name, const Vector3f &v) {
        glUniform3f(name, v.x(), v.y(), v.z());
    }

    void Context::set(const GLuint name, const Vector4f &v) {
        glUniform4f(name, v.x(), v.y(), v.z(), v.w());
    }

    void Context::logic_op(const GLenum opcode) {
        glLogicOp(opcode);
    }

    void Context::enable(const GLenum cap) {
        glEnable(cap);
    }

    void Context::disable(const GLenum cap) {
        glDisable(cap);
    }

    EigenAttribute::~EigenAttribute() {
        glDeleteBuffers(1, &buffer);
    }

    void EigenAttribute::bind() {
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glEnableVertexAttribArray(name);
        glVertexAttribPointer(name, dim,
                              type, integral, 0, 0);
    }
};

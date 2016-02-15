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

    void Context::draw(Model *model) {
        model->bind();
        if(model->is_indexed()) {
            glDrawElements(draw_mode, model->vertices(), GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(draw_mode, 0, model->vertices());
        }
    }

    void Context::draw(Model &model) {
        model.bind();
        if(model.is_indexed()) {
            glDrawElements(draw_mode, model.vertices(), GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(draw_mode, 0, model.vertices());
        }
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

    void Context::blend_func(const GLenum sfactor, const GLenum dfactor) {
        glBlendFunc(sfactor, dfactor);
    }

    BufferModel::~BufferModel() {
        glDeleteBuffers(1, &buffer);
    }

    bool BufferModel::is_indexed() {
        return false;
    }

    int EigenModel::vertices() {
        return columns;
    }

    void EigenModel::bind() {
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glEnableVertexAttribArray(name);
        glVertexAttribPointer(name, dim,
                              type, integral, 0, 0);
    }

    ShapeModel::ShapeModel(GLuint position_attr, GLuint normal_attr,
                           GLuint uv_attr, tinyobj::shape_t &shape) :
        position_attr(position_attr),
        normal_attr(normal_attr),
        uv_attr(uv_attr) {

        glGenBuffers(1, &position_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
        glBufferData(GL_ARRAY_BUFFER, shape.mesh.positions.size() * sizeof(GLfloat),
                     shape.mesh.positions.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &normal_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
        glBufferData(GL_ARRAY_BUFFER, shape.mesh.normals.size() * sizeof(GLfloat),
                     shape.mesh.normals.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &uv_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
        glBufferData(GL_ARRAY_BUFFER, shape.mesh.texcoords.size() * sizeof(GLfloat),
                     shape.mesh.texcoords.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &index_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, shape.mesh.indices.size() * sizeof(GLint),
                     shape.mesh.indices.data(), GL_STATIC_DRAW);

        indices = shape.mesh.indices.size();

    }

    void ShapeModel::bind() {
        glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
        glEnableVertexAttribArray(position_attr);
        glVertexAttribPointer(position_attr, 3, GL_FLOAT,
                              GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
        glEnableVertexAttribArray(normal_attr);
        glVertexAttribPointer(normal_attr, 3, GL_FLOAT,
                              GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
        glEnableVertexAttribArray(uv_attr);
        glVertexAttribPointer(uv_attr, 2, GL_FLOAT,
                              GL_FALSE, 0, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    }

    int ShapeModel::vertices() {
        return indices;
    }

    bool ShapeModel::is_indexed() {
        return true;
    }

    ShapeModel::~ShapeModel() {
        glDeleteBuffers(1, &position_buffer);
        glDeleteBuffers(1, &normal_buffer);
        glDeleteBuffers(1, &uv_buffer);
        glDeleteBuffers(1, &index_buffer);
    }
};

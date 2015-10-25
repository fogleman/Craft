#include <string>
#include <iostream>
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

    void ShaderProgram::render(ShaderData &data,
                               std::vector<std::shared_ptr<Uniform>> uniforms) {
        glUseProgram(program);
        glBindVertexArray(vao);
        for(auto &uniform : uniforms) {
            uniform->set();
        }
        for(auto &uniform : data.uniforms) {
            uniform->set();
        }
        for(auto &attribute : data.attributes) {
            glBindBuffer(GL_ARRAY_BUFFER, attribute->buffer());
            glEnableVertexAttribArray(attribute->name);
            glVertexAttribPointer(attribute->name, attribute->dim,
                                  attribute->type, attribute->integral, 0, 0);
        }
        glDrawArrays(GL_TRIANGLES, 0, data.size);
    }

    void Uniform1f::set() const {
        glUniform1f(name, value);
    }

    void UniformMatrix4fv::set() const {
        glUniformMatrix4fv(name, 1, GL_FALSE, value.data());
    }



    // /// Initialize a uniform parameter with an integer value
    // void UniformSetter::set(int value) const {
    //     glUniform1i(name, value);
    // }

    // /// Initialize a uniform parameter with a 2D vector
    // void UniformSetter::set(const Vector2f &v) const {
    //     glUniform2f(name, v.x(), v.y());
    // }

    // /// Initialize a uniform parameter with a 3D vector
    // void UniformSetter::set(const Vector3f &v) const {
    //     glUniform3f(name, v.x(), v.y(), v.z());
    // }

    // /// Initialize a uniform parameter with a 4D vector
    // void UniformSetter::set(const Vector4f &v) const {
    //     glUniform4f(name, v.x(), v.y(), v.z(), v.w());
    // }

};

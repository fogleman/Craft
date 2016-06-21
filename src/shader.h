#ifndef __SHADER_H__
#define __SHADER_H__
#include <string>
#include <memory>
#include <iostream>
#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <Eigen/Geometry>
#include "tiny_obj_loader.h"

namespace konstructs {
    using namespace Eigen;

    /** Generic method to load a shader of a specific type from a
     *  string.  This method is used internally by the Shader class,
     *  but can also be used directly to compile a shader and
     *  associate it with an id.
     *  @param type Type of shader (as described in glCreateShader)
     *  @param shader The source code of the shader
     *  @return The id of the newly created shader
     *  @throws std::runtime_error on compilation failure
     */
    GLuint creater_shader(const GLint type, const std::string &shader);

    /** Base class for models. A model is responsible for keeping
     *  track of a VBO and the number of vertices it contains. It is
     *  also responsible to bind the VBO to the appropriate attributes
     *  (in variables) when its bind method is called.
     */
    class Model {
    public:
        /** Bind the VBO associated with the Model */
        virtual void bind() = 0;
        /** Return the number of vertices in the VBO */
        virtual int vertices() = 0;
        /** Return if model is indexed */
        virtual bool is_indexed() = 0;
    };

    /** Base class for that require no special handling for their VBO.
     *  The variable buffer contains the handler to a VBO. It will
     *  automatically be deleted by the desctructor of this class.
     */
    class BufferModel : public Model {
    public:
        ~BufferModel();
        virtual bool is_indexed();
    protected:
        GLuint buffer;
    };

    /** Model that is based on any Eigen Matrix type.
     *  Each column represents a vertices in the VBO
     */
    class EigenModel : public BufferModel {
    public:
        /** Creates a EigenModel
         *  @param name Attribute id that the VBO should be attached to.
         *  @param m Eigen Matrix to be stored in the VBO
         */
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

    class ShapeModel : public Model {
    public:
        ShapeModel(GLuint position_attr, GLuint normal_attr, GLuint uv_attr,
                   tinyobj::shape_t &shape);
        ~ShapeModel();
        virtual void bind();
        virtual int vertices();
        virtual bool is_indexed();
    private:
        GLuint position_attr;
        GLuint normal_attr;
        GLuint uv_attr;
        GLuint position_buffer;
        GLuint normal_buffer;
        GLuint uv_buffer;
        GLuint index_buffer;
        int indices;
    };

    /** A Context instance provides access to Open GL functionality in
     *  safe way. When it is available the different methods are
     *  allowed to be used (i.e. the correct shader program has been
     *  loaded and the correct VAO attached).
     */
    class Context {
    public:
        Context(const GLenum _draw_mode) :
            draw_mode(_draw_mode) {}
        /** Draw a model
         *  @param model A pointer to the Model to be drawn
         */
        void draw(Model *model);
        /** Draw a model
         *  @param model A reference to the Model to be drawn
         */
        void draw(Model &model);
        /** Set a uniform to a float value */
        void set(const GLuint name, const float value);
        /** Set a uniform to a Matrix4f */
        void set(const GLuint name, const Matrix4f &value);
        /** Set a uniform to a integer value */
        void set(const GLuint name, const int value);
        /** Set a uniform to a GL unsigned integer value */
        void set(const GLuint name, const GLuint value);
        /** Set a uniform to a Vector2f */
        void set(const GLuint name, const Vector2f &v);
        /** Set a uniform to a Vector3f */
        void set(const GLuint name, const Vector3f &v);
        /** Set a uniform to a Vector4f */
        void set(const GLuint name, const Vector4f &v);
        /** See glLogicOp */
        void logic_op(const GLenum opcode);
        /** See glEnable */
        void enable(const GLenum cap);
        /** See glDisable */
        void disable(const GLenum cap);
        /** See glBlendFunc*/
        void blend_func(const GLenum sfactor, const GLenum dfactor);
    private:
        const GLenum draw_mode;
    };

    /** A ShaderProgram is a combination of a vertex and a fragment
     *  shader.  The class keeps track of loading the program and the
     *  VAO as well as providing a Context to be used for drawing
     *  Models associated with the shader.
     */
    class ShaderProgram {
    public:
        /** Creates a ShaderProgram instance.
         *  @param shader_name Name of the shader
         *  @param vertex_shader Code for the vertex shader
         *  @param fragment_shader Code for the fragment shader
         *  @param draw_mode The drawing mode used (see glDrawArrays),
         *         defaults to GL_TRIANGLES
         */
        ShaderProgram(const std::string &shader_name,
                      const std::string &vertex_shader,
                      const std::string &fragment_shader,
                      const GLenum draw_mode = GL_TRIANGLES);
        ~ShaderProgram();
        /** Binds the shader and the VAO and calls the provided function with a Context
         *  @param context The function that will draw models using this shader
         */
        void bind(std::function<void(Context context)> f);
    protected:
        /** Look up a uniform id by name.  This is requires the program
         *  to be bound. It is typically used by the constructor or
         *  initialization list of a sub class to get the uniform id
         *  of uniforms provided in the shader code.
         */
        GLuint uniformId(const std::string &uName);
        /** Look up a attribute id by name.  This is requires the program
         *  to be bound. It is typically used by the constructor or
         *  initialization list of a sub class to get the attribute id
         *  of attributes provided in the vertex shader code.
         */
        GLuint attributeId(const std::string &aName);
    private:
        const std::string &name;
        const GLuint vertex;
        const GLuint fragment;
        const GLuint program;
        const GLenum draw_mode;
        GLuint vao;
    };

};

#endif

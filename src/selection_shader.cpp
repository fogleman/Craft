#include "selection_shader.h"
#include "matrix.h"
#include "util.h"

namespace konstructs {
    using namespace Eigen;

    static Matrix<float, 3, 24> wireframe(const float scale) {
        Matrix<float, 3, 24> m;
        m.col(0) << -1, -1, -1;
        m.col(1) << -1, -1, +1;
        m.col(2) << -1, -1, -1;
        m.col(3) << -1, +1, -1;
        m.col(4) << -1, -1, -1;
        m.col(5) << +1, -1, -1;
        m.col(6) << -1, -1, +1;
        m.col(7) << -1, +1, +1;
        m.col(8) << -1, -1, +1;
        m.col(9) << +1, -1, +1;
        m.col(10) << -1, +1, -1;
        m.col(11) << -1, +1, +1;
        m.col(12) << -1, +1, -1;
        m.col(13) << +1, +1, -1;
        m.col(14) << -1, +1, +1;
        m.col(15) << +1, +1, +1;
        m.col(16) << +1, -1, -1;
        m.col(17) << +1, -1, +1;
        m.col(18) << +1, -1, -1;
        m.col(19) << +1, +1, -1;
        m.col(20) << +1, -1, +1;
        m.col(21) << +1, +1, +1;
        m.col(22) << +1, +1, -1;
        m.col(23) << +1, +1, +1;
        m *= scale;
        return m;
    }

    SelectionShader::SelectionShader(const int _radius, const float _fov,
                                     const float _near_distance, const float scale) :
        ShaderProgram(
        "chunk",
        "#version 330\n"
        "uniform mat4 matrix;\n"
        "uniform mat4 translation;\n"
        "in vec4 position;\n"
        "void main() {\n"
        "    vec4 global_position = translation * position;\n"
        "    gl_Position = matrix * global_position;\n"
        "}\n",
        "#version 330\n"
        "out vec4 frag_color;\n"
        "const float pi = 3.14159265;\n"
        "void main() {\n"
        "    frag_color = vec4(1.0, 1.0, 1.0, 1.0);\n"
        "}\n",
        GL_LINES),
        position_attr(attributeId("position")),
        matrix(uniformId("matrix")),
        translation(uniformId("translation")),
        radius(_radius),
        fov(_fov),
        near_distance(_near_distance),
        model(position_attr, wireframe(scale))
    {}

    void SelectionShader::render(const Player &p, const int width, const int height,
                                const Vector3i &selected) {
        bind([&](Context c) {
                c.enable(GL_DEPTH_TEST);
                float aspect_ratio = (float)width / (float)height;
                float max_distance = (radius - 1) * CHUNK_SIZE;
                const Matrix4f m = matrix::projection_perspective(fov, aspect_ratio, near_distance, max_distance) * p.view();
                c.set(matrix, m);
                c.set(translation, Affine3f(Translation3f(selected.cast<float>())).matrix());
                c.draw(&model, 0, model.size);
                c.disable(GL_DEPTH_TEST);
            });
    }

};

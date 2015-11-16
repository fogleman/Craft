#include "matrix.h"
#include "crosshair.h"

namespace konstructs {
    using matrix::projection_2d;
    void Crosshair::render() {
        bind([&](Context c) {
                c.enable(GL_COLOR_LOGIC_OP);
                c.set(projection, projection_2d(width, height));
                c.set(color, Vector4f( 0.0, 0.0, 0.0, 1.0));
                c.draw(data, 0, 4);
                c.disable(GL_COLOR_LOGIC_OP);
            });
    }
};

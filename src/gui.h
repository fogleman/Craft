#include <GLFW/glfw3.h>

namespace Konstructs {

    class Window {
        GLFWwindow* window;

        public:

        Window() {
            window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
        }

        ~Window() {
            glfwDestroyWindow(window);
        }

        int is_ok() {
            if (window) return 1;
            return 0;
        }

        GLFWwindow* get() {
            return window;
        }
    };

}


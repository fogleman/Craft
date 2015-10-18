#include <nanogui/nanogui.h>
#include <iostream>

class Konstructs : public nanogui::Screen {
public:
    Konstructs() : nanogui::Screen(Eigen::Vector2i(1024, 768), "Konstructs") {
        using namespace nanogui;
    }
};

int main(int argc, char **argv) {
    nanogui::init();

    {
        nanogui::ref<Konstructs> app = new Konstructs();
        app->drawAll();
        app->setVisible(true);
        nanogui::mainloop();
    }

    nanogui::shutdown();
    return 0;
}

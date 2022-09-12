#include "dc/dc_app.hpp"

int main()
{
    eng::DCApp app(1280, 720, "Dual Contouring", false);
    app.run();
    return 0;
}
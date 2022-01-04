#include "application.hpp"

int main()
{
    eng::Application application(1280, 720, "Title");
    application.run();
    return 0;
}
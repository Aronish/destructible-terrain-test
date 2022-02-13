#include "application.hpp"

int main()
{
    eng::Application application(1280, 720, "Engineering Game");
    application.run();
    return 0;
}
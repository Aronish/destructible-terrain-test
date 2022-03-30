#include "application.hpp"

int main()
{
    eng::Application application(1280, 720, "Engineering Game", true);
    application.run();
    return 0;
}
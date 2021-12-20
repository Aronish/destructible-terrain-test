#include "application.hpp"

int main()
{
    eng::Application application(1900, 1000, "Title");
    application.run();
    return 0;
}
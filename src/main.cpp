#include "application.hpp"
#include "dc/dc_app.hpp"

int main()
{
    //eng::Application application(1280, 720, "Engineering Game", false);
    //application.run();
    eng::DCApplication dc_application;
    dc_application.run();
    return 0;
}
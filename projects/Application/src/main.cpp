#include "Application.h"

int main(int argc, char * argv[], char * env[])
{
    Application app(argc, argv, env);
    return app.run();
}
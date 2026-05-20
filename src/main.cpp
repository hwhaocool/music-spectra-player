#include "app.h"
#include <iostream>

int main()
{
    App app;

    if (!app.init(1280, 720)) {
        std::cerr << "Initialization failed.\n";
        return -1;
    }

    app.run();
    app.shutdown();
    return 0;
}

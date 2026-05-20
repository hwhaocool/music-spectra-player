#include "app.h"
#include <iostream>
#include "unicode_utils.h"

int main()
{

    // 第一件事：把控制台设为 UTF-8，后续所有 std::cout 输出 UTF-8 即可正常显示
    setupConsoleUtf8();

    
    App app;

    if (!app.init(1280, 720)) {
        std::cerr << "Initialization failed.\n";
        return -1;
    }

    app.run();
    app.shutdown();
    return 0;
}

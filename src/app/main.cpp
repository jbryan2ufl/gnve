#include "engine.h"
#include <engine.h>

int main()
{
    try {
        GNVEngine app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

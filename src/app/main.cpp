#include "engine.h"

int main()
{
    Engine e{};
    e.init();
    e.run();
    e.clean_up();

    return 0;
}


#include "../events.h"

void test(float x, float y)
{
    std::cout << x << ", " << y << std::endl;
}

int main()
{
    Event<void(float x, float y)> ev;
    
    Listener blah(ev.connect(test));
    
    ev();
}

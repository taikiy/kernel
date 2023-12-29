#include "sys.h"
#include "terminal/terminal.h"

void
panic(const char* message)
{
    print("PANIC: ");
    print(message);
    print("\n");
    while (1) {};
}

#include "utils.h"

int debug_tab = 0;
void increase_debug_tab()
{
    debug_tab++;
}

void decrease_debug_tab()
{
    debug_tab--;
}

void reset_debug_tab()
{
    debug_tab = 0;
}

void print_debug_tab()
{
    int i;
    for(i = 0; i < debug_tab; i++)
        printf("\t");
}

bool enable_debug;
void set_debug(bool new_val)
{
    enable_debug = new_val;

    DEBUG("Debugging enabled.");
    DEBUG("Compiled at: %s %s", __DATE__, __TIME__);
    #ifdef __GNUC__
        DEBUG("Compiler used: GCC %s", __VERSION__);
    #else
        DEBUG("Compiler used: non-GCC");
    #endif
}

bool get_debug()
{
    return enable_debug;
}


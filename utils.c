#include "utils.h"

guint  debug_handler;

void debug_func(const gchar *log_domain,
                GLogLevelFlags log_level,
                const gchar *message,
                gpointer user_data)
{
    printf("DEBUG: %s\n", message);
}

void set_debug(bool new_val)
{
    if(new_val && !debug_handler)
    {
        debug_handler = g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, debug_func, NULL);
    }
    else if(debug_handler)
    {
        g_log_remove_handler(G_LOG_DOMAIN, debug_handler);
        debug_handler = 0;
    }

    DEBUG("Debugging enabled.", NULL);
    DEBUG("Compiled at: %s %s", __DATE__, __TIME__, NULL);
    #ifdef __GNUC__
        DEBUG("Compiler used: GCC %s", __VERSION__, NULL);
    #else
        DEBUG("Compiler used: non-GCC", NULL);
    #endif
}

bool get_debug()
{
    return debug_handler != 0;
}


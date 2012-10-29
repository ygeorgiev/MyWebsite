#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <config.h>

#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <gio/gio.h>

#define DEBUG(format, ...) if(get_debug()){g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, g_strdup_printf("(%s): %s", "%s@%s:%d", format), __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);} 
void set_debug(bool new_value);
bool get_debug();

#endif //UTILS_H_INCLUDED

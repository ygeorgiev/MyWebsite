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

#define DEBUG(...) if(get_debug()){printf("DEBUG(%s@%s:%d): ", __FUNCTION__, __FILE__, __LINE__);print_debug_tab();printf(__VA_ARGS__);printf("\n");}
#define DEBUG_INCREASE_TAB increase_debug_tab();
#define DEBUG_DECREASE_TAB decrease_debug_tab();
#define DEBUG_RESET_TAB debug_reset_tab();
void increase_debug_tab();
void decrease_debug_tab();
void reset_debug_tab();
void print_debug_tab();
void set_debug(bool new_value);
bool get_debug();

#endif //UTILS_H_INCLUDED

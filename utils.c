/**
 * Copyright (c) 2012, 2013,  Patrick Uiterwijk <puiterwijk@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of Patrick Uiterwijk nor the
 *      names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Patrick Uiterwijk BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


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


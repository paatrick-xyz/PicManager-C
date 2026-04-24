#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>

//  Global state
extern int   debug_mode;
extern char  base_path[256];

//  Debug logging 
#define LOG_DEBUG(format, ...) do {                                 \
    if (debug_mode) {                                               \
        FILE *f_debug = fopen("debug.log", "a");                   \
        if (f_debug) {                                              \
            fprintf(f_debug, "[DEBUG] " format "\n",               \
                    ##__VA_ARGS__);                                 \
            fclose(f_debug);                                        \
        }                                                           \
    }                                                               \
} while (0)

//  Functions 
void loadConfig();

#endif
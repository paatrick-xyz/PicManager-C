#include "config.h"
#include "menu.h"   // pause_screen()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int  debug_mode = 1;
char base_path[256];

void loadConfig() {
    FILE *f = fopen("config.cfg", "r");
    if (!f) {
        printf("Config file not found. Please enter the albums path: ");
        scanf("%s", base_path);

        LOG_DEBUG("Config file not found, creating one");

        FILE *f_new = fopen("config.cfg", "w");
        if (f_new) {
            fprintf(f_new, "ALBUMS_PATH=%s\n", base_path);
            fprintf(f_new, "DEBUG_MODE=%d\n", debug_mode);
            fclose(f_new);
        }
        printf("Config file created successfully.");
        pause_screen();
        return;
    }

    char line[300];
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "ALBUMS_PATH=", 12) == 0) {
            sscanf(line, "ALBUMS_PATH=%s", base_path);
            LOG_DEBUG("Path set as: %s", base_path);
        }
        if (strncmp(line, "DEBUG_MODE=", 11) == 0) {
            sscanf(line, "DEBUG_MODE=%d", &debug_mode);
            LOG_DEBUG("Debug mode set as: %d", debug_mode);
        }
    }
    fclose(f);
}
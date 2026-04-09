#ifndef LOGIC_H
#define LOGIC_H

extern int debug_mode;
extern char base_path[256];
#define LOG_DEBUG(format, ...) do{\
    if(debug_mode){\
        FILE *f_debug=fopen("debug.log","a");\
        if(f_debug){\
            fprintf(f_debug,"[DEBUG]" format "\n", ##__VA_ARGS__);\
        }\
    }\
} while(0)

typedef struct Menu Menu;

void loadConfig();
int getXResolution(char* path);
int getYResolution(char* path);
char* getFileInfo(char* path);
Menu* createAlbumMenu(char* path,char* name);
void openImage(char* path);
void createAlbum(char* path);
char* openFileDialog();
void addImageToAlbum(char* path);
#endif
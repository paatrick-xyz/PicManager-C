#ifndef LOGIC_H
#define LOGIC_H

typedef struct Menu Menu;

void loadConfig();
void action_op11();
void action_op12();
void action_add();
void action_subs();
void list_directory(const char* dirname);
void action_list_directory();
extern char base_path[256];
char* getFileInfo(char* path);
Menu* createAlbumMenu(char* path,char* name);
void openImage(char* path);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <time.h>
#include <sys/stat.h>
#endif
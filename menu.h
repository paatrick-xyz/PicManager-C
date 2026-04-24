
#ifndef MENU_H
#define MENU_H

#include <stdio.h>
#include <conio.h> // _getch() get character from console without echo
#include <stdlib.h>


typedef struct Menu Menu;
typedef struct Option Option;

typedef void (*ActionMenu)(); //placeholder for function

typedef enum{
    OPTION_NORMAL,
    OPTION_NEW_ALBUM,
    OPTION_ADD_IMAGE,
    OPTION_OPEN_IMAGE,
    OPTION_RENAME_IMAGE,
    OPTION_DELETE_IMAGE,
    OPTION_MOVE_IMAGE,
} OptionType;

typedef struct Menu {
    char *title;
    char *path;
    Option *options;
    int n;
}Menu;
typedef struct Option{
    char *name;
    char *details;
    char *path;
    Menu *submenu;
    ActionMenu action;
    OptionType type;
}Option;


void clear_screen();
void pause_screen();
void display_menu(Menu *menu, int selected);
int handle_input(int *selected, int max_option);
int menu_loop(Menu *menu);
void free_menu(Menu *m);

#endif
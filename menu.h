
#ifndef MENU_H
#define MENU_H

#include <stdio.h>
#include <conio.h> // _getch() get character from console without echo
#include <stdlib.h>


typedef struct Menu Menu;
typedef struct Option Option;

typedef void (*ActionMenu)(); //placeholder for function


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
}Option;


void clear_screen();
void pause_screen();
void display_menu(Menu *menu, int selected);
int handle_input(int *selected, int max_option);
int menu_loop(Menu *menu);
void free_menu(Menu *m);

#endif
#include "menu.h"
#include "logic.h"

#define max_option 3

/*
Option op_submenu1[]={
    {"Option 1.1", NULL,NULL},
    {"Option 1.2", NULL,action_op12},
    {"Return to main menu", NULL,NULL}
};

Option op_submenu2[]={
    {"Add 2 numbers", NULL, action_add},
    {"Subtract 2 numbers", NULL, action_subs},
    {"Return to main menu", NULL, NULL}
    };


Menu submenu1={"Option 1 submenu", op_submenu1, 3};
Menu submenu2={"Option 2 submenu", op_submenu2, 3};

Option main_op[]={
    {"Option 1", &submenu1, NULL},
    {"Option 2", &submenu2, NULL},
    {"Exit", NULL, NULL}
};

Menu main_menu={"Main Menu", main_op, max_option};
*/

int main(void) {
    loadConfig();
    Menu *dynamic_main=NULL;
    while(1){
       if(dynamic_main!=NULL) free_menu(dynamic_main);
       dynamic_main=createAlbumMenu(base_path,"Main Menu");
        if(dynamic_main==NULL){
            LOG_DEBUG("Error creating main menu");
            break;
        }
        int redo= menu_loop(dynamic_main);
        if(redo==0) break;
    }
    if(dynamic_main!=NULL) free_menu(dynamic_main);
    //menu_loop(&main_menu);
    return 0;
}

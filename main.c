#include "menu.h"
#include "config.h"   // LOG_DEBUG()
#include "config.h"
#include "album.h"    // createAlbum(), addImageToAlbum()
#include "image.h"    // openImage(), renameImage(), deleteImage(), moveImage()

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

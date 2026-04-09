#include "menu.h"
#include "logic.h"
#include <string.h>
void clear_screen() {
    system("cls"); // clear console screen
}

void pause_screen(){
    printf("Press any key to continue...");
    _getch(); // wait for user input without carring it
}


void display_menu(Menu *menu, int selected_option){
    clear_screen();
    printf("=== %s ===\n\n", menu->title);

    for(int i=0;i<menu->n;i++){
        if(selected_option == i+1){
            printf("> %-20s %10s <\n", menu->options[i].name,menu->options[i].details);
        }
        else{
            printf(" %-20s %10s \n", menu->options[i].name,menu->options[i].details);
        }
    }
}


int handle_input(int *selected_option, int max_options) {
    int input = _getch();

    if (input == 0 || input == 224) {
        input = _getch(); // Get the actual key code
        if (input == 72) { // Up
            (*selected_option)--;
            if (*selected_option < 1) *selected_option = max_options;
        } else if (input == 80) { // Down
            (*selected_option)++;
            if (*selected_option > max_options) *selected_option = 1;
        } else if (input == 75){ // LEFT
            return -1;
        } else if (input == 77){ // RIGHTa
            return 1;
        }
        return 0; // Signal that we just moved, didn't "select"
    } 
    
    if (input == 13) { // Enter key
        return 1; // Signal that user confirmed selection
    }if (input == 27){
        return -1; //BACK
    }
    if (input == 'a' || input == 'A'){
        return 2;
    }
    return 0;
}

int menu_loop(Menu *menu){
    int selected_option=1;
    int conf=0;
    //loadConfig();
    while (1){
        display_menu(menu, selected_option);
        conf=handle_input(&selected_option,menu->n);
        if(conf == -1) return 0;
        if(conf == 1){
            Option *selected = &menu->options[selected_option-1];
            if(strcmp(selected->name,"[NEW ALBUM]")==0){
                createAlbum(menu->path);
                return 1;
            }
            if(strcmp(selected->name,"[ADD IMAGE]")==0){
                addImageToAlbum(menu->path);
                return 1;
            }
            if(selected->submenu==NULL && selected->path!=NULL) {
                openImage(selected->path);
                conf=0;
            }
            if(selected->submenu==NULL && selected->action==NULL && conf !=0) return 0; // EXIT
            
            if(selected->submenu !=NULL){
                if(menu_loop(selected->submenu)) return 1; //acces submenu
            }
            else if (selected->action!=NULL){
                clear_screen();
                selected->action();           //acces option
            }
            conf=0;
        }
        if (conf == 2){
            addImageToAlbum(menu->options[0].path);
            return 1;
        }
    }
    return 0;
}

void free_menu(Menu *m) {
    if (m == NULL) return;

    // Eliberăm fiecare opțiune
    for (int i = 0; i < m->n; i++) {
        if (m->options[i].name) free(m->options[i].name);
        if (m->options[i].details) free(m->options[i].details);
        if (m->options[i].path) free(m->options[i].path);
        
        // Dacă opțiunea are un submeniu, îl eliberăm recursiv
        if (m->options[i].submenu) {
            free_menu(m->options[i].submenu);
        }
    }

    // Eliberăm array-ul de opțiuni și restul structurii Menu
    if (m->options) free(m->options);
    if (m->title) free(m->title);
    if (m->path) free(m->path);
    
    free(m);
}
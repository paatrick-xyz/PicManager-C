#include "menu.h"

void clear_screen() {
    system("cls"); // clear console screen
}

void pause_screen(){
    printf("Press any key to continue...");
    _getch(); // wait for user input without carring it
}


void display_menu(Menu *menu, int selected_option){
    clear_screen();
    printf("%s\n", menu->title);
    for (int i = 0; i < menu->n; i++) {
        printf("%s %s %s\n", selected_option == i + 1 ? ">" : " ", menu->options[i].name, selected_option == i + 1 ? "<" : " ");
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
        }
        return 0; // Signal that we just moved, didn't "select"
    } 
    
    if (input == 13) { // Enter key
        return 1; // Signal that user confirmed selection
    }

    return 0;
}

int menu_loop(Menu *menu){
    int selected_option=1;
    int conf=0;
    while (1){
        display_menu(menu, selected_option);
        conf=handle_input(&selected_option,menu->n);
        if(conf){
            Option *selected = &menu->options[selected_option-1];
            
            if(selected->submenu == NULL && selected->action == NULL) break; //return or exit

            if(selected->submenu !=NULL){
                menu_loop(selected->submenu); //acces submenu
            }
            else if (selected->action!=NULL){
                clear_screen();
                selected->action();           //acces option
            }
            conf=0;
        }
    }
    return 0;
}
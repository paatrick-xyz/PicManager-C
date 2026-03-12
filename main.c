#include <stdio.h>
#include <conio.h> // _getch() get character from console without echo
#include <stdlib.h> // system("cls") clear console screen

#define max_option 3
//setup
typedef struct Menu Menu;
typedef struct Option Option;

typedef void (*ActionMenu)(void); //placeholder for function


typedef struct Menu {
    char *title;
    Option *options;
    int n;
}Menu;
typedef struct Option{
    char *name;
    Menu *submenu;
    ActionMenu action;
}Option;


//func
void clear_screen() {
    system("cls"); // clear console screen
}

void pause_screen(){
    printf("Press any key to continue...");
    _getch(); // wait for user input without carring it
}


void action_op11(){
    printf("\nExecuting Task 1.1...");
    pause_screen();
}
void action_op12(){
    printf("\nExecuting Task 1.2...");
    pause_screen();
}


void action_add(){
    printf("Enter two numbers to add: ");
    int a, b;
    scanf("%d %d", &a, &b);
    printf("Sum of the numbers %d\n",a+b);
    pause_screen();
}

void action_subs(){
    printf("Enter two numbers to subtract: ");
    int a, b;
    scanf("%d %d", &a, &b);
    printf("Substaction of the numbers %d\n",a-b);
    pause_screen();
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

// MENU

Option op_submenu1[]={
    {"Option 1.1", NULL,action_op11},
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


int main(void) {
    menu_loop(&main_menu);
    return 0;
}

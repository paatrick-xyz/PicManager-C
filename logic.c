#include "logic.h"
#include "menu.h"

#include <stdio.h>

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

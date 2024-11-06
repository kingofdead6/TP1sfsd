#include <stdio.h>
#include "functions.h"

int main(){
    int choice = 0; 
    int key;
    welcome();
    system("cls");
    while (1) {
        display_Main_Menu(choice);

        key = getch();

        if (key == 224) {
            key = getch();
            switch (key) {
                case 72: 
                    choice = (choice - 1 + 4) % 4; 
                    break;
                case 80: 
                    choice = (choice + 1) % 4; 
                    break;
            }
        } else if (key == 13) { 
            switch (choice) {
                case 0:
                    system("cls");
                    TOVS();
                    break;
                case 1:
                    system("cls");
                    TnOVS();
                    break;
                case 2:
                   exit(0);
                    break ;
                default:
                    printf("Invalid option\n");
                    break;
            }
            printf("Press any key to continue...\n");
            getch(); 

        }
    }
}

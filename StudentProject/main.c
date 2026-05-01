#include <stdio.h>
#include "student.h"
#include "test.h"
#include "login.h"

int main() {
    int ch;

    if(!login()) return 0;

    while(1) {
        printf("\n===== MAIN MENU =====\n");
        printf("1.Student\n2.Test\n3.Exit\n");
        scanf("%d", &ch);

        if(ch==1) studentMenu();
        else if(ch==2) testMenu();
        else return 0;
    }
}
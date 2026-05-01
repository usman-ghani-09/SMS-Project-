#include <stdio.h>
#include <string.h>
#include "login.h"

int login() {
    char u[20], p[20];

    printf("\n--- LOGIN ---\n");
    printf("Username: ");
    scanf("%s", u);

    printf("Password: ");
    scanf("%s", p);

    if(strcmp(u, "admin") == 0 && strcmp(p, "1234") == 0) {
        printf("Login Successful!\n");
        return 1;
    }

    printf("Invalid Login!\n");
    return 0;
}
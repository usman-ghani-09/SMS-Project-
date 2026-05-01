#include <stdio.h>
#include "test.h"

typedef struct {
    int student_id;
    char subject[50];
    int marks;
    char date[12];
} Test;

void addTest() {
    FILE *fp;
    Test t;

    fp = fopen("test.dat", "ab");

    printf("Student ID: ");
    scanf("%d", &t.student_id);

    printf("Subject: ");
    scanf(" %[^\n]", t.subject);

    printf("Marks: ");
    scanf("%d", &t.marks);

    printf("Date: ");
    scanf("%s", t.date);

    fwrite(&t, sizeof(Test), 1, fp);
    fclose(fp);

    printf("Saved!\n");
}

void viewTests() {
    FILE *fp;
    Test t;

    fp = fopen("test.dat", "rb");

    while(fread(&t, sizeof(Test), 1, fp)) {
        printf("\nID:%d Sub:%s Marks:%d Date:%s\n",
               t.student_id, t.subject, t.marks, t.date);
    }

    fclose(fp);
}

void testMenu() {
    int ch;

    while(1) {
        printf("\n--- TEST MENU ---\n1.Add\n2.View\n3.Back\n");
        scanf("%d", &ch);

        if(ch==1) addTest();
        else if(ch==2) viewTests();
        else return;
    }
}
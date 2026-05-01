#include <stdio.h>
#include <string.h>
#include "student.h"

//Duplicate Check
int isDuplicateID(int id) {
    FILE *fp;
    Student s;

    fp = fopen("students.dat", "rb");
    if(fp == NULL) return 0;

    while(fread(&s, sizeof(Student), 1, fp)) {
        if(s.student_id == id) {
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

//Add Studen
void addStudent() {
    FILE *fp;
    Student s;

    printf("Enter ID: ");
    scanf("%d", &s.student_id);

    while(isDuplicateID(s.student_id)) {
        printf("ID exists! Enter new ID: ");
        scanf("%d", &s.student_id);
    }

    printf("Enter Name: ");
    getchar(); // buffer clear
    fgets(s.name, sizeof(s.name), stdin);
    s.name[strcspn(s.name, "\n")] = 0; // remove newline

    printf("Enter Age: ");
    scanf("%d", &s.age);

    printf("Enter Class: ");
    scanf("%s", s.class_name);

    fp = fopen("students.dat", "ab");
    fwrite(&s, sizeof(Student), 1, fp);
    fclose(fp);

    printf("Student Saved!\n");
}

//View All
void viewStudents() {
    FILE *fp;
    Student s;

    fp = fopen("students.dat", "rb");

    if(fp == NULL) {
        printf("No data!\n");
        return;
    }

    while(fread(&s, sizeof(Student), 1, fp)) {
        printf("\nID: %d\n", s.student_id);
        printf("Name: %s\n", s.name);
        printf("Age: %d\n", s.age);
        printf("Class: %s\n", s.class_name);
    }

    fclose(fp);
}

//View Single
void viewSingleStudent() {
    FILE *fp;
    Student s;
    int id, found = 0;

    printf("Enter ID: ");
    scanf("%d", &id);

    fp = fopen("students.dat", "rb");

    if(fp == NULL) {
        printf("No data found!\n");
        return;
    }

    while(fread(&s, sizeof(Student), 1, fp)) {
        if(s.student_id == id) {
            printf("\n--- Student Found ---\n");
            printf("ID: %d\n", s.student_id);
            printf("Name: %s\n", s.name);
            printf("Age: %d\n", s.age);
            printf("Class: %s\n", s.class_name);
            found = 1;
            break;
        }
    }

    fclose(fp);

    if(!found) printf("Student not found!\n");
}

//Update
void updateStudent() {
    FILE *fp, *temp;
    Student s;
    int id, found = 0;

    printf("Enter ID to update: ");
    scanf("%d", &id);

    fp = fopen("students.dat", "rb");
    temp = fopen("temp.dat", "wb");

    while(fread(&s, sizeof(Student), 1, fp)) {
        if(s.student_id == id) {
            printf("Enter new name: ");
            getchar();
            fgets(s.name, sizeof(s.name), stdin);
            s.name[strcspn(s.name, "\n")] = 0;

            printf("Enter new age: ");
            scanf("%d", &s.age);

            found = 1;
        }
        fwrite(&s, sizeof(Student), 1, temp);
    }

    fclose(fp);
    fclose(temp);

    remove("students.dat");
    rename("temp.dat", "students.dat");

    if(found) printf("Updated!\n");
    else printf("Not found!\n");
}

//Delete
void deleteStudent() {
    FILE *fp, *temp;
    Student s;
    int id, found = 0;

    printf("Enter ID to delete: ");
    scanf("%d", &id);

    fp = fopen("students.dat", "rb");
    temp = fopen("temp.dat", "wb");

    while(fread(&s, sizeof(Student), 1, fp)) {
        if(s.student_id == id) {
            found = 1;
        } else {
            fwrite(&s, sizeof(Student), 1, temp);
        }
    }

    fclose(fp);
    fclose(temp);

    remove("students.dat");
    rename("temp.dat", "students.dat");

    if(found) printf("Deleted!\n");
    else printf("Not found!\n");
}

//Menu
void studentMenu() {
    int ch;

    while(1) {
        printf("\n--- STUDENT MENU ---\n");
        printf("1. Add\n2. View All\n3. View One\n4. Update\n5. Delete\n6. Back\n");
        printf("Enter choice: ");
        scanf("%d", &ch);

        switch(ch) {
            case 1: addStudent(); break;
            case 2: viewStudents(); break;
            case 3: viewSingleStudent(); break;
            case 4: updateStudent(); break;
            case 5: deleteStudent(); break;
            case 6: return;
            default: printf("Invalid choice!\n");
        }
    }
}

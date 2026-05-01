#ifndef STUDENT_H
#define STUDENT_H

typedef struct {
    
    int student_id;
    char name[50];
    int age;
    char class_name[20];

} Student;

void addStudent();
void viewStudents();
void viewSingleStudent();
void updateStudent();
void deleteStudent();
void studentMenu();
int isDuplicateID(int id);

#endif
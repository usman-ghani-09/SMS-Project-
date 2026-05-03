#ifndef SMS_H
#define SMS_H
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_STUDENTS   "students.dat"
#define FILE_EXAMS      "exams.dat"
#define FILE_TESTS      "tests.dat"
#define FILE_FEES       "fees.dat"
#define FILE_USERS      "users.dat"
#define FILE_ATTENDANCE "attendance.dat"

typedef struct{
    int student_id,roll_no,age;
    char name[50],class_name[20],section[5];
    char gender[10],phone[15],email[50],dob[12],enroll_date[12];
}Student;
typedef struct{
    int exam_id,student_id,total_marks,obtained;
    float percentage;
    char subject[50],exam_date[12],grade[5],remarks[100];
}StudentExam;
typedef struct{
    int test_id,student_id,total_marks,obtained;
    float percentage;
    char test_name[50],subject[50],test_date[12],status[10];
}StudentTest;
typedef struct{
    int fee_id,student_id;
    float amount,paid_amount,balance;
    char fee_type[30],due_date[12],pay_date[12],payment_mode[20],status[10];
}StudentFee;
typedef struct{
    int user_id,is_active;
    char username[30],password[30],role[20];
}User;

typedef struct{
    int  att_id,student_id;
    char date[12];      
        char subject[50];
    char status[10];    
    char remarks[80];
}Attendance;

extern GtkWidget *main_stack,*main_window,*content_stack;
extern int        profile_student_id;

int     getNextStudentId(void);
int     getNextExamId(void);
int     getNextTestId(void);
int     getNextFeeId(void);
int     getNextAttId(void);
char   *gradeFromPercent(float p);
Student findStudentById(int id);
Student findStudentByRoll(int roll);

GtkWidget *build_login_page(void);
GtkWidget *build_sidebar(void);
GtkWidget *build_home_page(void);
GtkWidget *build_students_page(void);
GtkWidget *build_exams_page(void);
GtkWidget *build_tests_page(void);
GtkWidget *build_fees_page(void);
GtkWidget *build_profile_page(void);
GtkWidget *build_attendance_page(void);
void       refresh_profile(int student_id);

void show_message_dialog(GtkWindow *parent,const char *title,
                         const char *msg,GtkMessageType t);
void switch_to_content(const char *name);
void switch_to_login(void);
void refresh_dashboard(void);
#endif

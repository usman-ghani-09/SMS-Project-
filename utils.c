#include "sms.h"
int profile_student_id=0;

int getNextStudentId(void){FILE*f=fopen(FILE_STUDENTS,"rb");if(!f)return 1;Student s;int m=0;while(fread(&s,sizeof s,1,f))if(s.student_id>m)m=s.student_id;fclose(f);return m+1;}
int getNextExamId(void){FILE*f=fopen(FILE_EXAMS,"rb");if(!f)return 1;StudentExam e;int m=0;while(fread(&e,sizeof e,1,f))if(e.exam_id>m)m=e.exam_id;fclose(f);return m+1;}
int getNextTestId(void){FILE*f=fopen(FILE_TESTS,"rb");if(!f)return 1;StudentTest t;int m=0;while(fread(&t,sizeof t,1,f))if(t.test_id>m)m=t.test_id;fclose(f);return m+1;}
int getNextFeeId(void){FILE*f=fopen(FILE_FEES,"rb");if(!f)return 1;StudentFee v;int m=0;while(fread(&v,sizeof v,1,f))if(v.fee_id>m)m=v.fee_id;fclose(f);return m+1;}
char *gradeFromPercent(float p){if(p>=90)return"A+";if(p>=80)return"A";if(p>=70)return"B";if(p>=60)return"C";if(p>=50)return"D";return"F";}
Student findStudentById(int id){FILE*f=fopen(FILE_STUDENTS,"rb");Student s,e;memset(&e,0,sizeof e);if(!f)return e;while(fread(&s,sizeof s,1,f))if(s.student_id==id){fclose(f);return s;}fclose(f);return e;}
Student findStudentByRoll(int r){FILE*f=fopen(FILE_STUDENTS,"rb");Student s,e;memset(&e,0,sizeof e);if(!f)return e;while(fread(&s,sizeof s,1,f))if(s.roll_no==r){fclose(f);return s;}fclose(f);return e;}

void show_message_dialog(GtkWindow *parent,const char *title,const char *msg,GtkMessageType t){
    (void)title;(void)t;
    GtkAlertDialog *d=gtk_alert_dialog_new("%s",msg);
    gtk_alert_dialog_show(d,parent);g_object_unref(d);
}
void switch_to_content(const char *name){gtk_stack_set_visible_child_name(GTK_STACK(content_stack),name);}
void switch_to_login(void){gtk_stack_set_visible_child_name(GTK_STACK(main_stack),"login");}

int getNextAttId(void){
    FILE *f=fopen(FILE_ATTENDANCE,"rb");if(!f)return 1;
    Attendance a;int m=0;
    while(fread(&a,sizeof a,1,f))if(a.att_id>m)m=a.att_id;
    fclose(f);return m+1;
}

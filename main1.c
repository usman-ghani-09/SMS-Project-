/*
 * ============================================================
 *   STUDENT MANAGEMENT SYSTEM
 *   Modules: FEE | EXAM | RESULT
 *   Author : Your Name  (Team Member - Modules 5,6,7)
 *   Language: C  (C99)
 * ============================================================
 *
 *  WHY THIS DESIGN?
 *  ----------------
 *  1. FILE-BASED PERSISTENCE  – no database needed; each module
 *     writes/reads its own binary file so data survives program
 *     restarts and other team modules stay independent.
 *
 *  2. STRUCT-CENTRED DATA     – one struct per entity keeps
 *     related fields together, making read/write trivial with
 *     fread/fwrite (fixed-size records → O(1) seek).
 *
 *  3. SHARED STUDENT-ID KEY   – every module links to the same
 *     student_id that the core "Student" module (another team
 *     member) manages.  No duplicate name/roll storage needed.
 *
 *  4. MENU-DRIVEN CLI         – clean numbered menus; easy to
 *     extend later with a GUI or network layer.
 *
 *  COMPILE:
 *     gcc -std=c99 -Wall -o sms student_management.c
 *  RUN:
 *     ./sms
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ──────────────────────────────────────────────
   CONSTANTS
────────────────────────────────────────────── */
#define MAX_SUBJECTS   6
#define PASS_MARK      40      /* minimum marks to pass a subject */
#define FULL_MARK      100

/* File names – change paths to match team's shared directory */
#define FEE_FILE    "fee_records.dat"
#define EXAM_FILE   "exam_records.dat"
#define RESULT_FILE "result_records.dat"

/* ──────────────────────────────────────────────
   DATA STRUCTURES
   WHY structs?  → bundle related data, easy I/O
────────────────────────────────────────────── */

/* ---- FEE MODULE ---- */
typedef struct {
    int    student_id;           /* links to Student module  */
    char   student_name[50];
    float  total_fee;            /* full semester fee        */
    float  paid_amount;          /* cumulative amount paid   */
    float  pending_amount;       /* total_fee - paid_amount  */
    char   payment_status[20];   /* "PAID","PARTIAL","UNPAID"*/
    char   last_payment_date[15];/* DD/MM/YYYY               */
} FeeRecord;

/* ---- EXAM MODULE ---- */
typedef struct {
    int    student_id;
    char   student_name[50];
    char   exam_type[20];        /* "MID","FINAL","INTERNAL" */
    int    semester;
    char   subject[MAX_SUBJECTS][30];
    float  marks[MAX_SUBJECTS];  /* marks obtained           */
    int    num_subjects;
    char   exam_date[15];
} ExamRecord;

/* ---- RESULT MODULE ---- */
typedef struct {
    int    student_id;
    char   student_name[50];
    int    semester;
    float  marks[MAX_SUBJECTS];
    float  total_marks;
    float  percentage;
    float  gpa;                  /* 0.0 – 10.0 scale         */
    char   grade;                /* A+ A B C D F             */
    char   pass_fail[5];         /* "PASS" or "FAIL"         */
    char   subjects[MAX_SUBJECTS][30];
    int    num_subjects;
} ResultRecord;

/* ──────────────────────────────────────────────
   UTILITY HELPERS
────────────────────────────────────────────── */
void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pause_screen() {
    printf("\n  Press ENTER to continue...");
    while(getchar() != '\n');
    getchar();
}

void print_line(char c, int n) {
    for (int i = 0; i < n; i++) putchar(c);
    putchar('\n');
}

/* ══════════════════════════════════════════════
   ███████╗███████╗███████╗
   ██╔════╝██╔════╝██╔════╝
   █████╗  █████╗  █████╗
   ██╔══╝  ██╔══╝  ██╔══╝
   ██║     ███████╗███████╗
   ╚═╝     ╚══════╝╚══════╝
   FEE MODULE
   LOGIC:
     paid_amount += new payment
     pending = total - paid
     status  = PAID / PARTIAL / UNPAID
══════════════════════════════════════════════ */

/* ---------- update payment status helper ---------- */
void update_fee_status(FeeRecord *f) {
    f->pending_amount = f->total_fee - f->paid_amount;
    if (f->pending_amount <= 0) {
        f->pending_amount = 0;
        strcpy(f->payment_status, "PAID");
    } else if (f->paid_amount > 0) {
        strcpy(f->payment_status, "PARTIAL");
    } else {
        strcpy(f->payment_status, "UNPAID");
    }
}

/* ---------- ADD fee record ---------- */
void add_fee_record() {
    FeeRecord f;
    printf("\n  ── ADD FEE RECORD ──\n");
    printf("  Student ID   : "); scanf("%d",  &f.student_id);
    printf("  Student Name : "); scanf(" %49[^\n]", f.student_name);
    printf("  Total Fee    : "); scanf("%f",  &f.total_fee);
    printf("  Paid Amount  : "); scanf("%f",  &f.paid_amount);
    printf("  Payment Date (DD/MM/YYYY): "); scanf(" %14s", f.last_payment_date);

    update_fee_status(&f);

    FILE *fp = fopen(FEE_FILE, "ab");  /* append binary */
    if (!fp) { printf("  ERROR: Cannot open fee file.\n"); return; }
    fwrite(&f, sizeof(FeeRecord), 1, fp);
    fclose(fp);

    printf("\n  ✔ Fee record saved. Status: %s | Pending: %.2f\n",
           f.payment_status, f.pending_amount);
}

/* ---------- UPDATE (add payment) ---------- */
void update_fee_payment() {
    int sid; float amount;
    printf("\n  Student ID to update : "); scanf("%d", &sid);
    printf("  Payment Amount       : "); scanf("%f", &amount);

    /* Read all → modify matching record → write all back */
    FILE *fp = fopen(FEE_FILE, "rb");
    if (!fp) { printf("  No fee records found.\n"); return; }

    FeeRecord records[500];
    int count = 0, found = 0;
    while (fread(&records[count], sizeof(FeeRecord), 1, fp))
        count++;
    fclose(fp);

    for (int i = 0; i < count; i++) {
        if (records[i].student_id == sid) {
            records[i].paid_amount += amount;
            printf("  Payment Date (DD/MM/YYYY): ");
            scanf(" %14s", records[i].last_payment_date);
            update_fee_status(&records[i]);
            found = 1;
            printf("  ✔ Payment updated. New status: %s | Pending: %.2f\n",
                   records[i].payment_status, records[i].pending_amount);
        }
    }
    if (!found) { printf("  Student ID %d not found.\n", sid); return; }

    fp = fopen(FEE_FILE, "wb");
    fwrite(records, sizeof(FeeRecord), count, fp);
    fclose(fp);
}

/* ---------- VIEW all fee records ---------- */
void view_all_fees() {
    FILE *fp = fopen(FEE_FILE, "rb");
    if (!fp) { printf("  No fee records found.\n"); return; }

    FeeRecord f;
    printf("\n");
    print_line('=', 80);
    printf("  %-6s %-20s %10s %10s %10s %-10s %12s\n",
           "ID","Name","Total","Paid","Pending","Status","Last Payment");
    print_line('-', 80);
    while (fread(&f, sizeof(FeeRecord), 1, fp)) {
        printf("  %-6d %-20s %10.2f %10.2f %10.2f %-10s %12s\n",
               f.student_id, f.student_name,
               f.total_fee, f.paid_amount, f.pending_amount,
               f.payment_status, f.last_payment_date);
    }
    print_line('=', 80);
    fclose(fp);
}

/* ---------- Search fee by student ID ---------- */
void search_fee() {
    int sid; int found = 0;
    printf("\n  Search Student ID: "); scanf("%d", &sid);

    FILE *fp = fopen(FEE_FILE, "rb");
    if (!fp) { printf("  No records.\n"); return; }

    FeeRecord f;
    while (fread(&f, sizeof(FeeRecord), 1, fp)) {
        if (f.student_id == sid) {
            found = 1;
            printf("\n");
            print_line('-', 50);
            printf("  Student ID   : %d\n",   f.student_id);
            printf("  Name         : %s\n",   f.student_name);
            printf("  Total Fee    : %.2f\n", f.total_fee);
            printf("  Paid         : %.2f\n", f.paid_amount);
            printf("  Pending      : %.2f\n", f.pending_amount);
            printf("  Status       : %s\n",   f.payment_status);
            printf("  Last Payment : %s\n",   f.last_payment_date);
            print_line('-', 50);
        }
    }
    if (!found) printf("  Student ID %d not found in fee records.\n", sid);
    fclose(fp);
}

/* ---------- Fee defaulters report ---------- */
void fee_defaulters() {
    FILE *fp = fopen(FEE_FILE, "rb");
    if (!fp) { printf("  No records.\n"); return; }

    FeeRecord f;
    int found = 0;
    printf("\n  ── FEE DEFAULTERS (UNPAID / PARTIAL) ──\n");
    print_line('-', 60);
    while (fread(&f, sizeof(FeeRecord), 1, fp)) {
        if (strcmp(f.payment_status, "PAID") != 0) {
            printf("  ID:%-5d | %-20s | Pending: %.2f | %s\n",
                   f.student_id, f.student_name,
                   f.pending_amount, f.payment_status);
            found = 1;
        }
    }
    if (!found) printf("  No defaulters. All fees paid!\n");
    print_line('-', 60);
    fclose(fp);
}

/* ---------- FEE MENU ---------- */
void fee_menu() {
    int ch;
    do {
        clear_screen();
        printf("\n");
        print_line('=', 45);
        printf("       FEE MANAGEMENT MODULE\n");
        print_line('=', 45);
        printf("  1. Add Fee Record\n");
        printf("  2. Update Payment\n");
        printf("  3. View All Fee Records\n");
        printf("  4. Search Fee by Student ID\n");
        printf("  5. View Fee Defaulters\n");
        printf("  0. Back to Main Menu\n");
        print_line('-', 45);
        printf("  Choice: "); scanf("%d", &ch);
        switch(ch) {
            case 1: add_fee_record();    pause_screen(); break;
            case 2: update_fee_payment();pause_screen(); break;
            case 3: view_all_fees();     pause_screen(); break;
            case 4: search_fee();        pause_screen(); break;
            case 5: fee_defaulters();    pause_screen(); break;
            case 0: break;
            default: printf("  Invalid choice.\n");
        }
    } while(ch != 0);
}


/* ══════════════════════════════════════════════
   ███████╗██╗  ██╗ █████╗ ███╗   ███╗
   ██╔════╝╚██╗██╔╝██╔══██╗████╗ ████║
   █████╗   ╚███╔╝ ███████║██╔████╔██║
   ██╔══╝   ██╔██╗ ██╔══██║██║╚██╔╝██║
   ███████╗██╔╝ ██╗██║  ██║██║ ╚═╝ ██║
   ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝
   EXAM MODULE
   LOGIC:
     Store marks per subject per student per exam-type.
     Validate: marks 0–100.
     Support MID-TERM / FINAL / INTERNAL exam types.
══════════════════════════════════════════════ */

/* ---------- ADD exam record ---------- */
void add_exam_record() {
    ExamRecord e;
    printf("\n  ── ADD EXAM RECORD ──\n");
    printf("  Student ID   : "); scanf("%d",  &e.student_id);
    printf("  Student Name : "); scanf(" %49[^\n]", e.student_name);

    printf("  Exam Type (MID/FINAL/INTERNAL): "); scanf(" %19s", e.exam_type);
    printf("  Semester     : "); scanf("%d",  &e.semester);
    printf("  Exam Date (DD/MM/YYYY): "); scanf(" %14s", e.exam_date);
    printf("  Number of Subjects (max %d): ", MAX_SUBJECTS);
    scanf("%d", &e.num_subjects);

    if (e.num_subjects < 1 || e.num_subjects > MAX_SUBJECTS) {
        printf("  Invalid number of subjects. Must be 1-%d.\n", MAX_SUBJECTS);
        return;
    }

    for (int i = 0; i < e.num_subjects; i++) {
        printf("  Subject %d Name  : ", i+1); scanf(" %29s", e.subject[i]);
        do {
            printf("  Marks (0-100)   : "); scanf("%f", &e.marks[i]);
            if (e.marks[i] < 0 || e.marks[i] > FULL_MARK)
                printf("  ⚠ Marks must be 0-100. Try again.\n");
        } while (e.marks[i] < 0 || e.marks[i] > FULL_MARK);
    }

    FILE *fp = fopen(EXAM_FILE, "ab");
    if (!fp) { printf("  ERROR: Cannot open exam file.\n"); return; }
    fwrite(&e, sizeof(ExamRecord), 1, fp);
    fclose(fp);
    printf("  ✔ Exam record saved successfully.\n");
}

/* ---------- VIEW all exam records ---------- */
void view_all_exams() {
    FILE *fp = fopen(EXAM_FILE, "rb");
    if (!fp) { printf("  No exam records found.\n"); return; }

    ExamRecord e;
    while (fread(&e, sizeof(ExamRecord), 1, fp)) {
        printf("\n");
        print_line('-', 55);
        printf("  ID: %-5d | Name: %-20s | %s | Sem:%d\n",
               e.student_id, e.student_name, e.exam_type, e.semester);
        printf("  Date: %s\n", e.exam_date);
        printf("  %-20s  %6s\n", "Subject", "Marks");
        print_line('.', 30);
        for (int i = 0; i < e.num_subjects; i++)
            printf("  %-20s  %6.1f\n", e.subject[i], e.marks[i]);
        print_line('-', 55);
    }
    fclose(fp);
}

/* ---------- Search exam by student ID ---------- */
void search_exam() {
    int sid; int found = 0;
    printf("\n  Search Student ID: "); scanf("%d", &sid);

    FILE *fp = fopen(EXAM_FILE, "rb");
    if (!fp) { printf("  No records.\n"); return; }

    ExamRecord e;
    while (fread(&e, sizeof(ExamRecord), 1, fp)) {
        if (e.student_id == sid) {
            found = 1;
            printf("\n");
            print_line('-', 55);
            printf("  ID: %-5d | Name: %s\n", e.student_id, e.student_name);
            printf("  Exam: %-15s | Semester: %d | Date: %s\n",
                   e.exam_type, e.semester, e.exam_date);
            printf("  %-20s %8s  %6s\n","Subject","Marks","Status");
            print_line('.', 38);
            for (int i = 0; i < e.num_subjects; i++)
                printf("  %-20s %8.1f  %s\n",
                       e.subject[i], e.marks[i],
                       e.marks[i] >= PASS_MARK ? "PASS" : "FAIL");
            print_line('-', 55);
        }
    }
    if (!found) printf("  No exam record for ID %d.\n", sid);
    fclose(fp);
}

/* ---------- EXAM MENU ---------- */
void exam_menu() {
    int ch;
    do {
        clear_screen();
        printf("\n");
        print_line('=', 45);
        printf("       EXAM MANAGEMENT MODULE\n");
        print_line('=', 45);
        printf("  1. Add Exam Record\n");
        printf("  2. View All Exam Records\n");
        printf("  3. Search by Student ID\n");
        printf("  0. Back to Main Menu\n");
        print_line('-', 45);
        printf("  Choice: "); scanf("%d", &ch);
        switch(ch) {
            case 1: add_exam_record();  pause_screen(); break;
            case 2: view_all_exams();   pause_screen(); break;
            case 3: search_exam();      pause_screen(); break;
            case 0: break;
            default: printf("  Invalid choice.\n");
        }
    } while(ch != 0);
}


/* ══════════════════════════════════════════════
   ██████╗ ███████╗███████╗██╗   ██╗██╗  ████████╗
   ██╔══██╗██╔════╝██╔════╝██║   ██║██║  ╚══██╔══╝
   ██████╔╝█████╗  ███████╗██║   ██║██║     ██║
   ██╔══██╗██╔══╝  ╚════██║██║   ██║██║     ██║
   ██║  ██║███████╗███████║╚██████╔╝███████╗██║
   ╚═╝  ╚═╝╚══════╝╚══════╝ ╚═════╝ ╚══════╝╚═╝
   RESULT MODULE
   LOGIC:
     percentage = (sum of marks / total possible) * 100
     GPA        = percentage / 10   (0–10 scale)
     Grade      = A+(>=90) A(>=80) B(>=70) C(>=60) D(>=50) F(<40)
     PASS       = ALL subjects >= PASS_MARK (40)
     Pulls exam data automatically if student ID exists.
══════════════════════════════════════════════ */

/* ---------- Grade calculator helper ---------- */
char calculate_grade(float percentage) {
    if (percentage >= 90) return 'S';   /* S = Outstanding */
    if (percentage >= 80) return 'A';
    if (percentage >= 70) return 'B';
    if (percentage >= 60) return 'C';
    if (percentage >= 50) return 'D';
    return 'F';
}

/* ---------- GENERATE result from exam data ---------- */
/*  WHY auto-generate?  → avoids duplicate data entry.
    We read the exam file, find the student's FINAL exam,
    compute totals, grades, and save to result file.       */
void generate_result() {
    int sid;
    printf("\n  Generate Result for Student ID: "); scanf("%d", &sid);

    /* Step 1: Find FINAL exam record for this student */
    FILE *efp = fopen(EXAM_FILE, "rb");
    if (!efp) { printf("  No exam records.\n"); return; }

    ExamRecord e;
    int found = 0;
    while (fread(&e, sizeof(ExamRecord), 1, efp)) {
        /* Use FINAL exam for result; change condition for other types */
        if (e.student_id == sid &&
            strcmp(e.exam_type, "FINAL") == 0) {
            found = 1;
            break;
        }
    }
    fclose(efp);

    if (!found) {
        printf("  No FINAL exam record for Student ID %d.\n", sid);
        printf("  Tip: Add a FINAL exam record first.\n");
        return;
    }

    /* Step 2: Compute result fields */
    ResultRecord r;
    r.student_id  = e.student_id;
    r.semester    = e.semester;
    r.num_subjects = e.num_subjects;
    strcpy(r.student_name, e.student_name);

    float sum = 0;
    int all_pass = 1;
    for (int i = 0; i < e.num_subjects; i++) {
        r.marks[i] = e.marks[i];
        strcpy(r.subjects[i], e.subject[i]);
        sum += e.marks[i];
        if (e.marks[i] < PASS_MARK) all_pass = 0;
    }

    r.total_marks = sum;
    r.percentage  = (sum / (e.num_subjects * FULL_MARK)) * 100.0f;
    r.gpa         = r.percentage / 10.0f;
    r.grade       = calculate_grade(r.percentage);
    strcpy(r.pass_fail, all_pass ? "PASS" : "FAIL");

    /* Step 3: Save result */
    FILE *rfp = fopen(RESULT_FILE, "ab");
    if (!rfp) { printf("  ERROR: Cannot open result file.\n"); return; }
    fwrite(&r, sizeof(ResultRecord), 1, rfp);
    fclose(rfp);

    /* Step 4: Display the generated result */
    printf("\n");
    print_line('=', 55);
    printf("  RESULT CARD – Semester %d\n", r.semester);
    print_line('=', 55);
    printf("  Student ID  : %d\n",   r.student_id);
    printf("  Name        : %s\n",   r.student_name);
    print_line('-', 55);
    printf("  %-22s  %6s  %6s\n", "Subject", "Marks", "Status");
    print_line('.', 38);
    for (int i = 0; i < r.num_subjects; i++)
        printf("  %-22s  %6.1f  %s\n",
               r.subjects[i], r.marks[i],
               r.marks[i] >= PASS_MARK ? "PASS" : "FAIL");
    print_line('-', 55);
    printf("  Total Marks : %.1f / %d\n",
           r.total_marks, r.num_subjects * FULL_MARK);
    printf("  Percentage  : %.2f%%\n", r.percentage);
    printf("  GPA         : %.2f / 10\n", r.gpa);
    printf("  Grade       : %c\n",    r.grade);
    printf("  Result      : %s\n",    r.pass_fail);
    print_line('=', 55);
    printf("  ✔ Result saved.\n");
}

/* ---------- VIEW all results ---------- */
void view_all_results() {
    FILE *fp = fopen(RESULT_FILE, "rb");
    if (!fp) { printf("  No result records found.\n"); return; }

    ResultRecord r;
    printf("\n");
    print_line('=', 70);
    printf("  %-6s %-20s %5s %8s %6s %5s %6s\n",
           "ID","Name","Sem","Total%","GPA","Grade","Result");
    print_line('-', 70);
    while (fread(&r, sizeof(ResultRecord), 1, fp)) {
        printf("  %-6d %-20s %5d %7.2f%% %6.2f %5c %6s\n",
               r.student_id, r.student_name,
               r.semester, r.percentage,
               r.gpa, r.grade, r.pass_fail);
    }
    print_line('=', 70);
    fclose(fp);
}

/* ---------- Search result by ID ---------- */
void search_result() {
    int sid; int found = 0;
    printf("\n  Search Student ID: "); scanf("%d", &sid);

    FILE *fp = fopen(RESULT_FILE, "rb");
    if (!fp) { printf("  No records.\n"); return; }

    ResultRecord r;
    while (fread(&r, sizeof(ResultRecord), 1, fp)) {
        if (r.student_id == sid) {
            found = 1;
            printf("\n");
            print_line('=', 55);
            printf("  RESULT CARD – Semester %d\n", r.semester);
            print_line('=', 55);
            printf("  ID: %d  |  Name: %s\n", r.student_id, r.student_name);
            print_line('-', 55);
            for (int i = 0; i < r.num_subjects; i++)
                printf("  %-22s  %6.1f  %s\n",
                       r.subjects[i], r.marks[i],
                       r.marks[i] >= PASS_MARK ? "PASS" : "FAIL");
            print_line('-', 55);
            printf("  Percentage : %.2f%%  |  GPA: %.2f  |  Grade: %c  |  %s\n",
                   r.percentage, r.gpa, r.grade, r.pass_fail);
            print_line('=', 55);
        }
    }
    if (!found) printf("  No result for Student ID %d.\n", sid);
    fclose(fp);
}

/* ---------- Toppers list (sorted by %) ---------- */
void toppers_list() {
    FILE *fp = fopen(RESULT_FILE, "rb");
    if (!fp) { printf("  No result records.\n"); return; }

    ResultRecord records[500];
    int count = 0;
    while (fread(&records[count], sizeof(ResultRecord), 1, fp))
        count++;
    fclose(fp);

    /* Bubble sort descending by percentage — simple & clear */
    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - i - 1; j++)
            if (records[j].percentage < records[j+1].percentage) {
                ResultRecord tmp = records[j];
                records[j] = records[j+1];
                records[j+1] = tmp;
            }

    printf("\n  ── TOPPERS LIST ──\n");
    print_line('-', 55);
    printf("  %-4s %-6s %-20s %8s %6s\n","Rank","ID","Name","Score%","Grade");
    print_line('.', 48);
    for (int i = 0; i < count && i < 10; i++)
        printf("  %-4d %-6d %-20s %7.2f%% %6c\n",
               i+1, records[i].student_id, records[i].student_name,
               records[i].percentage, records[i].grade);
    print_line('-', 55);
}

/* ---------- RESULT MENU ---------- */
void result_menu() {
    int ch;
    do {
        clear_screen();
        printf("\n");
        print_line('=', 45);
        printf("      RESULT MANAGEMENT MODULE\n");
        print_line('=', 45);
        printf("  1. Generate Result (from Exam data)\n");
        printf("  2. View All Results\n");
        printf("  3. Search Result by Student ID\n");
        printf("  4. View Toppers List (Top 10)\n");
        printf("  0. Back to Main Menu\n");
        print_line('-', 45);
        printf("  Choice: "); scanf("%d", &ch);
        switch(ch) {
            case 1: generate_result();  pause_screen(); break;
            case 2: view_all_results(); pause_screen(); break;
            case 3: search_result();    pause_screen(); break;
            case 4: toppers_list();     pause_screen(); break;
            case 0: break;
            default: printf("  Invalid choice.\n");
        }
    } while(ch != 0);
}


/* ══════════════════════════════════════════════
   MAIN – Entry Point
   Shows the top-level menu and delegates to modules
══════════════════════════════════════════════ */
int main() {
    int choice;

    do {
        clear_screen();
        printf("\n");
        print_line('=', 50);
        printf("      STUDENT MANAGEMENT SYSTEM\n");
        printf("      [ Fee | Exam | Result Modules ]\n");
        print_line('=', 50);
        printf("  1. Fee Management\n");
        printf("  2. Exam Management\n");
        printf("  3. Result Management\n");
        printf("  0. Exit\n");
        print_line('-', 50);
        printf("  Enter choice: ");
        scanf("%d", &choice);

        switch(choice) {
            case 1: fee_menu();    break;
            case 2: exam_menu();   break;
            case 3: result_menu(); break;
            case 0:
                printf("\n  Goodbye! Data saved to .dat files.\n\n");
                break;
            default:
                printf("  Invalid choice. Try again.\n");
        }
    } while(choice != 0);

    return 0;
}

#include <time.h>
#include "sms.h"

/* ── Column indices ── */
enum{
    COL_A_ID=0, COL_A_ROLL, COL_A_NAME, COL_A_CLASS,
    COL_A_DATE, COL_A_SUBJECT, COL_A_STATUS, COL_A_REMARKS,
    N_A_COLS
};

static GtkListStore *a_store;
static GtkWidget    *a_filter_entry;   /* date filter */
static GtkWidget    *a_status_filter;  /* dropdown    */
static GtkWidget    *a_class_filter;   /* class filter */
static GtkWidget    *a_section_filter; /* section filter */

/* ────────────────────────────────────────
   RELOAD
──────────────────────────────────────── */
static void reload_attendance(void){
    gtk_list_store_clear(a_store);
    FILE *fp=fopen(FILE_ATTENDANCE,"rb"); if(!fp)return;

    const char *date_q  = gtk_editable_get_text(GTK_EDITABLE(a_filter_entry));
    const char *class_q = gtk_editable_get_text(GTK_EDITABLE(a_class_filter));
    const char *sec_q   = gtk_editable_get_text(GTK_EDITABLE(a_section_filter));
    const char *stat_q  = gtk_drop_down_get_selected(GTK_DROP_DOWN(a_status_filter))==0
                          ? "" :
                          gtk_drop_down_get_selected(GTK_DROP_DOWN(a_status_filter))==1
                          ? "Present" :
                          gtk_drop_down_get_selected(GTK_DROP_DOWN(a_status_filter))==2
                          ? "Absent" : "Late";

    Attendance a;
    while(fread(&a,sizeof a,1,fp)){
        Student s=findStudentById(a.student_id);
        /* apply filters */
        if(*date_q  && strstr(a.date,date_q)==NULL)              continue;
        if(*class_q && strcasecmp(s.class_name,class_q)!=0)      continue;
        if(*sec_q   && strcasecmp(s.section,   sec_q  )!=0)      continue;
        if(*stat_q  && strcmp(a.status,stat_q)!=0)               continue;

        GtkTreeIter it; gtk_list_store_append(a_store,&it);
        gtk_list_store_set(a_store,&it,
            COL_A_ID,      a.att_id,
            COL_A_ROLL,    s.roll_no,
            COL_A_NAME,    s.name,
            COL_A_CLASS,   s.class_name,
            COL_A_DATE,    a.date,
            COL_A_SUBJECT, a.subject,
            COL_A_STATUS,  a.status,
            COL_A_REMARKS, a.remarks,
            -1);
    }
    fclose(fp);
}

static void on_filter_changed(GtkEditable *e,gpointer d){(void)e;(void)d;reload_attendance();}
static void on_status_changed(GtkDropDown *dd,GParamSpec *p,gpointer d){
    (void)dd;(void)p;(void)d; reload_attendance();}
static void on_refresh_a(GtkButton *b,gpointer d){(void)b;(void)d;reload_attendance();}

/* ────────────────────────────────────────
   GET SELECTED ATT_ID
──────────────────────────────────────── */
static int get_sel_aid(GtkTreeView *tv){
    GtkTreeSelection *sel=gtk_tree_view_get_selection(tv);
    GtkTreeModel *m; GtkTreeIter it;
    if(!gtk_tree_selection_get_selected(sel,&m,&it)) return -1;
    int id; gtk_tree_model_get(m,&it,COL_A_ID,&id,-1); return id;
}

/* ────────────────────────────────────────
   FORM ROW HELPER
──────────────────────────────────────── */
static void frow_a(GtkWidget *g,int row,const char *l,GtkWidget **o){
    GtkWidget *lw=gtk_label_new(l);
    gtk_widget_add_css_class(lw,"field-label");
    gtk_widget_set_halign(lw,GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(g),lw,0,row,1,1);
    GtkWidget *e=gtk_entry_new();
    gtk_widget_set_hexpand(e,TRUE);
    gtk_grid_attach(GTK_GRID(g),e,1,row,1,1);
    if(o)*o=e;
}

/* ────────────────────────────────────────
   ADD ATTENDANCE DIALOG
──────────────────────────────────────── */
typedef struct{
    GtkWidget *e_roll,*e_class,*e_section,*e_date,*e_subject,*e_remarks;
    GtkWidget *dd_status;
    GtkWidget *err,*dlg;
}AddAttCtx;

static void cb_save_att(GtkButton *b,gpointer ud){
    (void)b; AddAttCtx *c=ud;
    const char *rs   =gtk_editable_get_text(GTK_EDITABLE(c->e_roll));
    const char *cls  =gtk_editable_get_text(GTK_EDITABLE(c->e_class));
    const char *sec  =gtk_editable_get_text(GTK_EDITABLE(c->e_section));
    const char *date =gtk_editable_get_text(GTK_EDITABLE(c->e_date));
    const char *subj =gtk_editable_get_text(GTK_EDITABLE(c->e_subject));
    const char *rem  =gtk_editable_get_text(GTK_EDITABLE(c->e_remarks));
    guint stat_idx   =gtk_drop_down_get_selected(GTK_DROP_DOWN(c->dd_status));

    if(!*rs||!*cls||!*sec||!*date||!*subj){
        gtk_label_set_text(GTK_LABEL(c->err),"⚠ Roll No, Class, Section, Date and Subject are required.");
        return;
    }
    Student s=findStudentByRollClassSec(atoi(rs),cls,sec);
    if(s.student_id==0){
        gtk_label_set_text(GTK_LABEL(c->err),
            "⚠ No student with this Roll No in that Class & Section.");return;}

    const char *statuses[]={"Present","Absent","Late"};
    Attendance a; memset(&a,0,sizeof a);
    a.att_id    =getNextAttId();
    a.student_id=s.student_id;
    strncpy(a.date,   date,11);
    strncpy(a.subject,subj,49);
    strncpy(a.status, statuses[stat_idx<3?stat_idx:0],9);
    strncpy(a.remarks,rem, 79);

    FILE *fp=fopen(FILE_ATTENDANCE,"ab");
    if(fp){fwrite(&a,sizeof a,1,fp);fclose(fp);}
    reload_attendance(); refresh_dashboard();
    gtk_window_destroy(GTK_WINDOW(c->dlg));
    g_free(c);
}

static void on_add_att(GtkButton *btn,gpointer tv){
    (void)btn;(void)tv;
    GtkWidget *dlg=gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dlg),"Mark Attendance");
    gtk_window_set_modal(GTK_WINDOW(dlg),TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
    gtk_window_set_default_size(GTK_WINDOW(dlg),380,-1);

    GtkWidget *vb=gtk_box_new(GTK_ORIENTATION_VERTICAL,10);
    gtk_widget_set_margin_start(vb,20);gtk_widget_set_margin_end(vb,20);
    gtk_widget_set_margin_top(vb,16); gtk_widget_set_margin_bottom(vb,16);
    gtk_window_set_child(GTK_WINDOW(dlg),vb);

    GtkWidget *h=gtk_label_new("Mark Attendance");
    gtk_widget_add_css_class(h,"page-title");
    gtk_widget_set_halign(h,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vb),h);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid),8);
    gtk_grid_set_column_spacing(GTK_GRID(grid),10);
    gtk_box_append(GTK_BOX(vb),grid);

    AddAttCtx *ctx=g_new0(AddAttCtx,1);
    frow_a(grid,0,"Student Roll No *",&ctx->e_roll);
    frow_a(grid,1,"Class *",&ctx->e_class);
    frow_a(grid,2,"Section *",&ctx->e_section);
    frow_a(grid,3,"Date (DD-MM-YYYY) *",&ctx->e_date);
    frow_a(grid,4,"Subject *",&ctx->e_subject);

    /* Status dropdown */
    GtkWidget *stat_lbl=gtk_label_new("Status *");
    gtk_widget_add_css_class(stat_lbl,"field-label");
    gtk_widget_set_halign(stat_lbl,GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(grid),stat_lbl,0,5,1,1);

    const char *opts[]={"Present","Absent","Late",NULL};
    GtkStringList *sl=gtk_string_list_new(opts);
    ctx->dd_status=gtk_drop_down_new(G_LIST_MODEL(sl),NULL);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(ctx->dd_status),0);
    gtk_widget_set_hexpand(ctx->dd_status,TRUE);
    gtk_grid_attach(GTK_GRID(grid),ctx->dd_status,1,5,1,1);

    frow_a(grid,6,"Remarks",&ctx->e_remarks);
    ctx->dlg=dlg;

    ctx->err=gtk_label_new("");
    gtk_widget_add_css_class(ctx->err,"login-err");
    gtk_box_append(GTK_BOX(vb),ctx->err);

    GtkWidget *br=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8);
    gtk_widget_set_halign(br,GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(vb),br);

    GtkWidget *cancel=gtk_button_new_with_label("Cancel");
    gtk_box_append(GTK_BOX(br),cancel);
    g_signal_connect_swapped(cancel,"clicked",G_CALLBACK(gtk_window_destroy),dlg);

    GtkWidget *save=gtk_button_new_with_label("Save");
    gtk_widget_add_css_class(save,"btn-primary");
    gtk_box_append(GTK_BOX(br),save);
    g_signal_connect(save,"clicked",G_CALLBACK(cb_save_att),ctx);

    gtk_window_present(GTK_WINDOW(dlg));
}

/* ────────────────────────────────────────
   BULK ATTENDANCE DIALOG
   Mark all students in a class for a date
──────────────────────────────────────── */
typedef struct{
    GtkWidget *e_class,*e_section,*e_date,*e_subject;
    GtkWidget *dd_status;
    GtkWidget *err,*dlg;
}BulkAttCtx;

static void cb_save_bulk(GtkButton *b,gpointer ud){
    (void)b; BulkAttCtx *c=ud;
    const char *cls  =gtk_editable_get_text(GTK_EDITABLE(c->e_class));
    const char *sec  =gtk_editable_get_text(GTK_EDITABLE(c->e_section));
    const char *date =gtk_editable_get_text(GTK_EDITABLE(c->e_date));
    const char *subj =gtk_editable_get_text(GTK_EDITABLE(c->e_subject));
    guint stat_idx   =gtk_drop_down_get_selected(GTK_DROP_DOWN(c->dd_status));

    if(!*cls||!*date||!*subj){
        gtk_label_set_text(GTK_LABEL(c->err),"⚠ Class, Date and Subject are required.");
        return;
    }
    const char *statuses[]={"Present","Absent","Late"};
    const char *status=statuses[stat_idx<3?stat_idx:0];

    /* Find all students in that class/section */
    FILE *sfp=fopen(FILE_STUDENTS,"rb");
    if(!sfp){gtk_label_set_text(GTK_LABEL(c->err),"⚠ Cannot open students file.");return;}
    FILE *afp=fopen(FILE_ATTENDANCE,"ab");
    if(!afp){fclose(sfp);gtk_label_set_text(GTK_LABEL(c->err),"⚠ Cannot open attendance file.");return;}

    Student s; int count=0;
    while(fread(&s,sizeof s,1,sfp)){
        if(strcmp(s.class_name,cls)!=0) continue;
        if(*sec && strcmp(s.section,sec)!=0) continue;
        Attendance a; memset(&a,0,sizeof a);
        a.att_id    =getNextAttId()+count;
        a.student_id=s.student_id;
        strncpy(a.date,   date,  11);
        strncpy(a.subject,subj,  49);
        strncpy(a.status, status, 9);
        fwrite(&a,sizeof a,1,afp);
        count++;
    }
    fclose(sfp); fclose(afp);

    if(count==0){
        gtk_label_set_text(GTK_LABEL(c->err),"⚠ No students found for that class/section.");
        return;
    }
    reload_attendance(); refresh_dashboard();
    gtk_window_destroy(GTK_WINDOW(c->dlg));
    g_free(c);
}

static void on_bulk_att(GtkButton *btn,gpointer tv){
    (void)btn;(void)tv;
    GtkWidget *dlg=gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dlg),"Bulk Attendance");
    gtk_window_set_modal(GTK_WINDOW(dlg),TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
    gtk_window_set_default_size(GTK_WINDOW(dlg),380,-1);

    GtkWidget *vb=gtk_box_new(GTK_ORIENTATION_VERTICAL,10);
    gtk_widget_set_margin_start(vb,20);gtk_widget_set_margin_end(vb,20);
    gtk_widget_set_margin_top(vb,16);gtk_widget_set_margin_bottom(vb,16);
    gtk_window_set_child(GTK_WINDOW(dlg),vb);

    GtkWidget *h=gtk_label_new("Bulk Attendance (Whole Class)");
    gtk_widget_add_css_class(h,"page-title");
    gtk_widget_set_halign(h,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vb),h);

    GtkWidget *note=gtk_label_new("Marks all students in the class with the selected status.");
    gtk_widget_add_css_class(note,"page-subtitle");
    gtk_widget_set_halign(note,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vb),note);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid),8);
    gtk_grid_set_column_spacing(GTK_GRID(grid),10);
    gtk_box_append(GTK_BOX(vb),grid);

    BulkAttCtx *ctx=g_new0(BulkAttCtx,1);
    frow_a(grid,0,"Class *",          &ctx->e_class);
    frow_a(grid,1,"Section (opt)",    &ctx->e_section);
    frow_a(grid,2,"Date (DD-MM-YYYY) *",&ctx->e_date);
    frow_a(grid,3,"Subject *",        &ctx->e_subject);

    GtkWidget *sl2=gtk_label_new("Status *");
    gtk_widget_add_css_class(sl2,"field-label");
    gtk_widget_set_halign(sl2,GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(grid),sl2,0,4,1,1);
    const char *opts[]={"Present","Absent","Late",NULL};
    GtkStringList *sl=gtk_string_list_new(opts);
    ctx->dd_status=gtk_drop_down_new(G_LIST_MODEL(sl),NULL);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(ctx->dd_status),0);
    gtk_widget_set_hexpand(ctx->dd_status,TRUE);
    gtk_grid_attach(GTK_GRID(grid),ctx->dd_status,1,4,1,1);

    ctx->dlg=dlg;
    ctx->err=gtk_label_new("");
    gtk_widget_add_css_class(ctx->err,"login-err");
    gtk_box_append(GTK_BOX(vb),ctx->err);

    GtkWidget *br=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8);
    gtk_widget_set_halign(br,GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(vb),br);
    GtkWidget *cancel=gtk_button_new_with_label("Cancel");
    gtk_box_append(GTK_BOX(br),cancel);
    g_signal_connect_swapped(cancel,"clicked",G_CALLBACK(gtk_window_destroy),dlg);
    GtkWidget *save=gtk_button_new_with_label("Mark All");
    gtk_widget_add_css_class(save,"btn-warning");
    gtk_box_append(GTK_BOX(br),save);
    g_signal_connect(save,"clicked",G_CALLBACK(cb_save_bulk),ctx);
    gtk_window_present(GTK_WINDOW(dlg));
}

/* ────────────────────────────────────────
   DELETE
──────────────────────────────────────── */
static void on_delete_att(GtkButton *btn,gpointer tv_ptr){
    (void)btn;
    int aid=get_sel_aid(GTK_TREE_VIEW(tv_ptr));
    if(aid<0){
        show_message_dialog(GTK_WINDOW(main_window),"",
            "Select an attendance record first.",GTK_MESSAGE_WARNING);
        return;
    }
    FILE *fp =fopen(FILE_ATTENDANCE,"rb");
    FILE *tmp=fopen("tmp_a.dat","wb");
    if(fp&&tmp){
        Attendance a;
        while(fread(&a,sizeof a,1,fp))
            if(a.att_id!=aid) fwrite(&a,sizeof a,1,tmp);
        fclose(fp);fclose(tmp);
        remove(FILE_ATTENDANCE); rename("tmp_a.dat",FILE_ATTENDANCE);
    }
    reload_attendance(); refresh_dashboard();
}

/* ────────────────────────────────────────
   SUMMARY DIALOG — attendance % per student
──────────────────────────────────────── */
static void on_show_summary(GtkButton *btn,gpointer d){
    (void)btn;(void)d;
    GtkWidget *dlg=gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dlg),"Attendance Summary");
    gtk_window_set_modal(GTK_WINDOW(dlg),TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
    gtk_window_set_default_size(GTK_WINDOW(dlg),500,400);

    GtkWidget *vb=gtk_box_new(GTK_ORIENTATION_VERTICAL,10);
    gtk_widget_set_margin_start(vb,16);gtk_widget_set_margin_end(vb,16);
    gtk_widget_set_margin_top(vb,14); gtk_widget_set_margin_bottom(vb,14);
    gtk_window_set_child(GTK_WINDOW(dlg),vb);

    GtkWidget *h=gtk_label_new("Attendance Summary");
    gtk_widget_add_css_class(h,"page-title");
    gtk_widget_set_halign(h,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vb),h);

    /* Build summary store */
    GtkListStore *ss=gtk_list_store_new(5,
        G_TYPE_INT,G_TYPE_STRING,G_TYPE_INT,G_TYPE_INT,G_TYPE_STRING);
    enum{SC_ROLL,SC_NAME,SC_PRESENT,SC_TOTAL,SC_PCT};

    FILE *fp=fopen(FILE_ATTENDANCE,"rb");
    if(fp){
        /* collect unique students */
        int sids[500]; int nsids=0;
        Attendance recs[5000]; int nrecs=0;
        Attendance a;
        while(fread(&a,sizeof a,1,fp)&&nrecs<5000){
            recs[nrecs++]=a;
            int found=0;
            for(int i=0;i<nsids;i++) if(sids[i]==a.student_id){found=1;break;}
            if(!found&&nsids<500) sids[nsids++]=a.student_id;
        }
        fclose(fp);

        for(int si=0;si<nsids;si++){
            int sid=sids[si];
            Student s=findStudentById(sid);
            int total=0,present=0;
            for(int i=0;i<nrecs;i++){
                if(recs[i].student_id!=sid) continue;
                total++;
                if(strcmp(recs[i].status,"Present")==0||
                   strcmp(recs[i].status,"Late")==0) present++;
            }
            float pct=total>0?(float)present/total*100.f:0;
            char pct_s[16]; snprintf(pct_s,sizeof pct_s,"%.1f%%",pct);
            GtkTreeIter it; gtk_list_store_append(ss,&it);
            gtk_list_store_set(ss,&it,
                SC_ROLL,s.roll_no,SC_NAME,s.name,
                SC_PRESENT,present,SC_TOTAL,total,SC_PCT,pct_s,-1);
        }
    }

    GtkWidget *tv=gtk_tree_view_new_with_model(GTK_TREE_MODEL(ss));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tv),TRUE);
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(tv),GTK_TREE_VIEW_GRID_LINES_HORIZONTAL);

    struct{const char*t;int c;}scols[]={
        {"Roll",SC_ROLL},{"Student",SC_NAME},
        {"Present",SC_PRESENT},{"Total",SC_TOTAL},{"Attendance%",SC_PCT}};
    for(int i=0;i<5;i++){
        GtkCellRenderer *r=gtk_cell_renderer_text_new();
        g_object_set(r,"xpad",6,"ypad",4,
            "foreground","#1e293b","foreground-set",TRUE,NULL);
        GtkTreeViewColumn *c=gtk_tree_view_column_new_with_attributes(
            scols[i].t,r,"text",scols[i].c,NULL);
        gtk_tree_view_column_set_resizable(c,TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(tv),c);
    }

    GtkWidget *scroll=gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll,TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll),tv);
    gtk_box_append(GTK_BOX(vb),scroll);

    GtkWidget *close_btn=gtk_button_new_with_label("Close");
    gtk_widget_set_halign(close_btn,GTK_ALIGN_END);
    g_signal_connect_swapped(close_btn,"clicked",G_CALLBACK(gtk_window_destroy),dlg);
    gtk_box_append(GTK_BOX(vb),close_btn);

    gtk_window_present(GTK_WINDOW(dlg));
}

/* ────────────────────────────────────────
   STATUS CELL RENDERER — colour by status
──────────────────────────────────────── */
static void status_cell_func(GtkTreeViewColumn *col,GtkCellRenderer *r,
                              GtkTreeModel *model,GtkTreeIter *iter,gpointer d){
    (void)col;(void)d;
    char *status=NULL;
    gtk_tree_model_get(model,iter,COL_A_STATUS,&status,-1);
    if(!status)return;
    const char *fg="#1e293b";
    if(strcmp(status,"Present")==0)      fg="#16a34a";
    else if(strcmp(status,"Absent")==0)  fg="#dc2626";
    else if(strcmp(status,"Late")==0)    fg="#d97706";
    g_object_set(r,"text",status,"foreground",fg,"foreground-set",TRUE,NULL);
    g_free(status);
}

/* ────────────────────────────────────────
   CLEAR FILTERS — resets to today
──────────────────────────────────────── */
static void on_clear_filters(GtkButton *b,gpointer d){
    (void)b;(void)d;
    time_t t=time(NULL);
    struct tm *tm_=localtime(&t);
    char today[12];
    strftime(today,sizeof today,"%d-%m-%Y",tm_);
    gtk_editable_set_text(GTK_EDITABLE(a_filter_entry),today);
    gtk_editable_set_text(GTK_EDITABLE(a_class_filter),"");
    gtk_editable_set_text(GTK_EDITABLE(a_section_filter),"");
    gtk_drop_down_set_selected(GTK_DROP_DOWN(a_status_filter),0);
}

/* ────────────────────────────────────────
   BUILD PAGE
──────────────────────────────────────── */
GtkWidget *build_attendance_page(void){
    GtkWidget *vbox=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);

    /* ── Header ── */
    GtkWidget *hdr=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8);
    gtk_widget_add_css_class(hdr,"page-header");
    gtk_box_append(GTK_BOX(vbox),hdr);

    GtkWidget *tb=gtk_box_new(GTK_ORIENTATION_VERTICAL,2);
    gtk_widget_set_hexpand(tb,TRUE);
    gtk_box_append(GTK_BOX(hdr),tb);

    GtkWidget *title=gtk_label_new("Attendance");
    gtk_widget_add_css_class(title,"page-title");
    gtk_widget_set_halign(title,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(tb),title);

    GtkWidget *sub=gtk_label_new("Track student attendance by date and subject");
    gtk_widget_add_css_class(sub,"page-subtitle");
    gtk_widget_set_halign(sub,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(tb),sub);

    GtkWidget *bulk_btn=gtk_button_new_with_label("Bulk");
    gtk_widget_add_css_class(bulk_btn,"btn-warning");
    gtk_box_append(GTK_BOX(hdr),bulk_btn);

    GtkWidget *add_btn=gtk_button_new_with_label("Mark");
    gtk_widget_add_css_class(add_btn,"btn-primary");
    gtk_box_append(GTK_BOX(hdr),add_btn);

    /* ── Filter toolbar — row 1: Date, Class, Section, Status ── */
    GtkWidget *toolbar=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8);
    gtk_widget_add_css_class(toolbar,"toolbar");
    gtk_box_append(GTK_BOX(vbox),toolbar);

    /* Get today's date in DD-MM-YYYY format */
    char today_str[12]={0};
    {
        time_t t=time(NULL);
        struct tm *tm_=localtime(&t);
        strftime(today_str,sizeof today_str,"%d-%m-%Y",tm_);
    }

    GtkWidget *date_lbl=gtk_label_new("Date:");
    gtk_widget_add_css_class(date_lbl,"field-label");
    gtk_box_append(GTK_BOX(toolbar),date_lbl);

    a_filter_entry=gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(a_filter_entry),"DD-MM-YYYY");
    gtk_editable_set_text(GTK_EDITABLE(a_filter_entry),today_str);
    gtk_widget_set_size_request(a_filter_entry,110,-1);
    gtk_box_append(GTK_BOX(toolbar),a_filter_entry);

    GtkWidget *cls_lbl=gtk_label_new("Class:");
    gtk_widget_add_css_class(cls_lbl,"field-label");
    gtk_box_append(GTK_BOX(toolbar),cls_lbl);

    a_class_filter=gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(a_class_filter),"e.g. 10");
    gtk_widget_set_size_request(a_class_filter,70,-1);
    gtk_box_append(GTK_BOX(toolbar),a_class_filter);

    GtkWidget *sec_lbl=gtk_label_new("Section:");
    gtk_widget_add_css_class(sec_lbl,"field-label");
    gtk_box_append(GTK_BOX(toolbar),sec_lbl);

    a_section_filter=gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(a_section_filter),"e.g. A");
    gtk_widget_set_size_request(a_section_filter,55,-1);
    gtk_box_append(GTK_BOX(toolbar),a_section_filter);

    GtkWidget *stat_lbl2=gtk_label_new("Status:");
    gtk_widget_add_css_class(stat_lbl2,"field-label");
    gtk_box_append(GTK_BOX(toolbar),stat_lbl2);

    const char *filter_opts[]={"All","Present","Absent","Late",NULL};
    GtkStringList *fsl=gtk_string_list_new(filter_opts);
    a_status_filter=gtk_drop_down_new(G_LIST_MODEL(fsl),NULL);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(a_status_filter),0);
    gtk_box_append(GTK_BOX(toolbar),a_status_filter);

    GtkWidget *clr_btn=gtk_button_new_with_label("✕ Clear");
    gtk_box_append(GTK_BOX(toolbar),clr_btn);

    GtkWidget *sp=gtk_label_new("");
    gtk_widget_set_hexpand(sp,TRUE);
    gtk_box_append(GTK_BOX(toolbar),sp);

    GtkWidget *sum_btn=gtk_button_new_with_label("Summary");
    gtk_widget_add_css_class(sum_btn,"btn-info");
    gtk_box_append(GTK_BOX(toolbar),sum_btn);

    GtkWidget *ref=gtk_button_new_with_label("↺");
    gtk_box_append(GTK_BOX(toolbar),ref);

    /* ── List store ── */
    a_store=gtk_list_store_new(N_A_COLS,
        G_TYPE_INT,G_TYPE_INT,G_TYPE_STRING,G_TYPE_STRING,
        G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING);

    GtkWidget *tv=gtk_tree_view_new_with_model(GTK_TREE_MODEL(a_store));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tv),TRUE);
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(tv),GTK_TREE_VIEW_GRID_LINES_HORIZONTAL);

    /* Regular columns */
    struct{const char *t;int c;}cols[]={
        {"Roll",COL_A_ROLL},{"Student",COL_A_NAME},{"Class",COL_A_CLASS},
        {"Date",COL_A_DATE},{"Subject",COL_A_SUBJECT},{"Remarks",COL_A_REMARKS}};
    for(int i=0;i<6;i++){
        GtkCellRenderer *r=gtk_cell_renderer_text_new();
        g_object_set(r,"xpad",6,"ypad",4,
            "foreground","#1e293b","foreground-set",TRUE,NULL);
        GtkTreeViewColumn *c=gtk_tree_view_column_new_with_attributes(
            cols[i].t,r,"text",cols[i].c,NULL);
        gtk_tree_view_column_set_resizable(c,TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(tv),c);
    }

    /* Status column with colour */
    GtkCellRenderer *sr=gtk_cell_renderer_text_new();
    g_object_set(sr,"xpad",6,"ypad",4,"font-weight",700,NULL);
    GtkTreeViewColumn *sc=gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(sc,"Status");
    gtk_tree_view_column_pack_start(sc,sr,TRUE);
    gtk_tree_view_column_set_cell_data_func(sc,sr,status_cell_func,NULL,NULL);
    gtk_tree_view_column_set_resizable(sc,TRUE);
    /* insert Status before Remarks — reorder: Roll,Student,Class,Date,Subject,Status,Remarks */
    gtk_tree_view_insert_column(GTK_TREE_VIEW(tv),sc,5);

    GtkWidget *scroll=gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll,TRUE);
    gtk_widget_set_margin_start(scroll,12);gtk_widget_set_margin_end(scroll,12);
    gtk_widget_set_margin_top(scroll,8);  gtk_widget_set_margin_bottom(scroll,8);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll),tv);
    gtk_box_append(GTK_BOX(vbox),scroll);

    /* ── Bottom bar ── */
    GtkWidget *bot=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8);
    gtk_widget_add_css_class(bot,"toolbar");
    gtk_box_append(GTK_BOX(vbox),bot);
    GtkWidget *sp2=gtk_label_new("");
    gtk_widget_set_hexpand(sp2,TRUE);
    gtk_box_append(GTK_BOX(bot),sp2);
    GtkWidget *del_btn=gtk_button_new_with_label("Delete");
    gtk_widget_add_css_class(del_btn,"btn-danger");
    gtk_box_append(GTK_BOX(bot),del_btn);

    /* ── Connect signals ── */
    g_signal_connect(add_btn,  "clicked",  G_CALLBACK(on_add_att),      tv);
    g_signal_connect(bulk_btn, "clicked",  G_CALLBACK(on_bulk_att),     tv);
    g_signal_connect(del_btn,  "clicked",  G_CALLBACK(on_delete_att),   tv);
    g_signal_connect(sum_btn,  "clicked",  G_CALLBACK(on_show_summary), NULL);
    g_signal_connect(ref,      "clicked",  G_CALLBACK(on_refresh_a),    NULL);
    g_signal_connect(a_filter_entry,   "changed",G_CALLBACK(on_filter_changed),NULL);
    g_signal_connect(a_class_filter,   "changed",G_CALLBACK(on_filter_changed),NULL);
    g_signal_connect(a_section_filter, "changed",G_CALLBACK(on_filter_changed),NULL);
    g_signal_connect(a_status_filter,  "notify::selected",
                     G_CALLBACK(on_status_changed),NULL);
    g_signal_connect(clr_btn,"clicked",G_CALLBACK(on_clear_filters),NULL);

    reload_attendance(); refresh_dashboard();
    return vbox;
}

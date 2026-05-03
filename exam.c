#include "sms.h"




enum{COL_E_ROLL=0,COL_E_NAME,COL_E_CLASS,COL_E_SUBJECTS,
     COL_E_AVG,COL_E_GRADE,COL_E_SID,N_E_COLS};
static GtkListStore *e_store;

static void reload_exams(void){
    gtk_list_store_clear(e_store);
    FILE *fp=fopen(FILE_EXAMS,"rb");if(!fp)return;

    StudentExam exams[2000]; int nexams=0;
    StudentExam e;
    while(fread(&e,sizeof e,1,fp)&&nexams<2000) exams[nexams++]=e;
    fclose(fp);

    int sids[500]; int nsids=0;
    for(int i=0;i<nexams;i++){
        int found=0;
    for(int j=0;j<nsids;j++) if(sids[j]==exams[i].student_id){found=1;break;}
        if(!found&&nsids<500) sids[nsids++]=exams[i].student_id;
    }

    for(int si=0;si<nsids;si++){
        int sid=sids[si];
        Student s=findStudentById(sid);


        char subj_buf[512]=""; float tot=0; int cnt=0;
        for(int i=0;i<nexams;i++){
            if(exams[i].student_id!=sid) continue;
            char part[80];
            snprintf(part,sizeof part,"%s(%.0f%%)",
                     exams[i].subject,exams[i].percentage);
            if(cnt>0) strncat(subj_buf,"  |  ",sizeof subj_buf-strlen(subj_buf)-1);
            strncat(subj_buf,part,sizeof subj_buf-strlen(subj_buf)-1);
            tot+=exams[i].percentage; cnt++;
        }
        char avg_s[16]; float avg=cnt>0?tot/cnt:0;
        snprintf(avg_s,sizeof avg_s,"%.1f%%",avg);

        GtkTreeIter it; gtk_list_store_append(e_store,&it);
        gtk_list_store_set(e_store,&it,
            COL_E_ROLL, s.roll_no,
            COL_E_NAME, s.name,
            COL_E_CLASS,s.class_name,
            COL_E_SUBJECTS,subj_buf,
            COL_E_AVG, avg_s,
            COL_E_GRADE,gradeFromPercent(avg),
            COL_E_SID, sid,-1);
    }
}


typedef struct{GtkWidget*e[6];GtkWidget*err,*dlg;}AddExamCtx;
static void cb_save_exam(GtkButton *b,gpointer ud){
    (void)b;AddExamCtx *c=ud;
    const char *rs=gtk_editable_get_text(GTK_EDITABLE(c->e[0]));
    const char *su=gtk_editable_get_text(GTK_EDITABLE(c->e[1]));
    const char *da=gtk_editable_get_text(GTK_EDITABLE(c->e[2]));
    const char *to=gtk_editable_get_text(GTK_EDITABLE(c->e[3]));
    const char *ob=gtk_editable_get_text(GTK_EDITABLE(c->e[4]));
    const char *re=gtk_editable_get_text(GTK_EDITABLE(c->e[5]));
    if(!*rs||!*su||!*da||!*to||!*ob){
        gtk_label_set_text(GTK_LABEL(c->err),"⚠ Fill all required fields.");return;}
    Student s=findStudentByRoll(atoi(rs));
    if(s.student_id==0){gtk_label_set_text(GTK_LABEL(c->err),"⚠ Student not found.");return;}
    StudentExam e;memset(&e,0,sizeof e);
    e.exam_id=getNextExamId();e.student_id=s.student_id;
    e.total_marks=atoi(to);e.obtained=atoi(ob);
    e.percentage=e.total_marks>0?(float)e.obtained/e.total_marks*100.f:0;
    strncpy(e.subject,su,49);strncpy(e.exam_date,da,11);strncpy(e.remarks,re,99);
    strncpy(e.grade,gradeFromPercent(e.percentage),4);
    FILE *fp=fopen(FILE_EXAMS,"ab");
    if(fp){fwrite(&e,sizeof e,1,fp);fclose(fp);}
    reload_exams();gtk_window_destroy(GTK_WINDOW(c->dlg));g_free(c);
}

static void frow_e(GtkWidget *g,int row,const char *l,GtkWidget **o){
    GtkWidget *lw=gtk_label_new(l);gtk_widget_add_css_class(lw,"field-label");
    gtk_widget_set_halign(lw,GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(g),lw,0,row,1,1);
    GtkWidget *e=gtk_entry_new();gtk_widget_set_hexpand(e,TRUE);
    gtk_grid_attach(GTK_GRID(g),e,1,row,1,1);if(o)*o=e;
}

static void on_add_exam(GtkButton *btn,gpointer tv){
    (void)btn;(void)tv;
    GtkWidget *dlg=gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dlg),"Add Exam Record");
    gtk_window_set_modal(GTK_WINDOW(dlg),TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
    gtk_window_set_default_size(GTK_WINDOW(dlg),380,-1);
    GtkWidget *vb=gtk_box_new(GTK_ORIENTATION_VERTICAL,10);
    gtk_widget_set_margin_start(vb,20);gtk_widget_set_margin_end(vb,20);
    gtk_widget_set_margin_top(vb,16);gtk_widget_set_margin_bottom(vb,16);
    gtk_window_set_child(GTK_WINDOW(dlg),vb);
    GtkWidget *h=gtk_label_new("Add Exam Record");
    gtk_widget_add_css_class(h,"page-title");gtk_widget_set_halign(h,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vb),h);
    GtkWidget *note=gtk_label_new("Add one subject at a time per student.");
    gtk_widget_add_css_class(note,"page-subtitle");gtk_widget_set_halign(note,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vb),note);
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid),8);
    gtk_grid_set_column_spacing(GTK_GRID(grid),10);
    gtk_box_append(GTK_BOX(vb),grid);
    AddExamCtx *ctx=g_new0(AddExamCtx,1);
    frow_e(grid,0,"Student Roll No *",&ctx->e[0]);
    frow_e(grid,1,"Subject *",&ctx->e[1]);
    frow_e(grid,2,"Exam Date *",&ctx->e[2]);
    frow_e(grid,3,"Total Marks *",&ctx->e[3]);
    frow_e(grid,4,"Marks Obtained *",&ctx->e[4]);
    frow_e(grid,5,"Remarks",&ctx->e[5]);
    ctx->dlg=dlg;ctx->err=gtk_label_new("");
    gtk_widget_add_css_class(ctx->err,"login-err");gtk_box_append(GTK_BOX(vb),ctx->err);
    GtkWidget *br=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8);
    gtk_widget_set_halign(br,GTK_ALIGN_END);gtk_box_append(GTK_BOX(vb),br);
    GtkWidget *cancel=gtk_button_new_with_label("Cancel");
    gtk_box_append(GTK_BOX(br),cancel);
    g_signal_connect_swapped(cancel,"clicked",G_CALLBACK(gtk_window_destroy),dlg);
    GtkWidget *save=gtk_button_new_with_label("Save");
    gtk_widget_add_css_class(save,"btn-success");gtk_box_append(GTK_BOX(br),save);
    g_signal_connect(save,"clicked",G_CALLBACK(cb_save_exam),ctx);
    gtk_window_present(GTK_WINDOW(dlg));
}

static void on_delete_exam(GtkButton *btn,gpointer tv_ptr){
    (void)btn;
    GtkTreeView *tv=GTK_TREE_VIEW(tv_ptr);
    GtkTreeSelection *sel=gtk_tree_view_get_selection(tv);
    GtkTreeModel *m;GtkTreeIter it;
    if(!gtk_tree_selection_get_selected(sel,&m,&it)){
        show_message_dialog(GTK_WINDOW(main_window),"","Select a student row first.",GTK_MESSAGE_WARNING);return;}
    int sid;gtk_tree_model_get(m,&it,COL_E_SID,&sid,-1);
    FILE *fp=fopen(FILE_EXAMS,"rb");FILE *tmp=fopen("tmp_e.dat","wb");
    if(fp&&tmp){StudentExam e;
        while(fread(&e,sizeof e,1,fp))if(e.student_id!=sid)fwrite(&e,sizeof e,1,tmp);
        fclose(fp);fclose(tmp);remove(FILE_EXAMS);rename("tmp_e.dat",FILE_EXAMS);}
    reload_exams();
}

static void on_refresh_e(GtkButton *b,gpointer d){(void)b;(void)d;reload_exams();}

GtkWidget *build_exams_page(void){
    GtkWidget *vbox=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);

    GtkWidget *hdr=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,10);
    gtk_widget_add_css_class(hdr,"page-header");gtk_box_append(GTK_BOX(vbox),hdr);
    GtkWidget *tb=gtk_box_new(GTK_ORIENTATION_VERTICAL,2);
    gtk_widget_set_hexpand(tb,TRUE);gtk_box_append(GTK_BOX(hdr),tb);
    GtkWidget *title=gtk_label_new("📝  Exams");
    gtk_widget_add_css_class(title,"page-title");gtk_widget_set_halign(title,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(tb),title);
    GtkWidget *sub=gtk_label_new("Each row shows all subjects for a student");
    gtk_widget_add_css_class(sub,"page-subtitle");gtk_widget_set_halign(sub,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(tb),sub);
    GtkWidget *add_btn=gtk_button_new_with_label("＋ Add Exam");
    gtk_widget_add_css_class(add_btn,"btn-primary");gtk_box_append(GTK_BOX(hdr),add_btn);

    GtkWidget *toolbar=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8);
    gtk_widget_add_css_class(toolbar,"toolbar");gtk_box_append(GTK_BOX(vbox),toolbar);
    GtkWidget *ref=gtk_button_new_with_label("↺ Refresh");
    g_signal_connect(ref,"clicked",G_CALLBACK(on_refresh_e),NULL);
    gtk_box_append(GTK_BOX(toolbar),ref);

    e_store=gtk_list_store_new(N_E_COLS,
        G_TYPE_INT,G_TYPE_STRING,G_TYPE_STRING,
        G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_INT);
    GtkWidget *tv=gtk_tree_view_new_with_model(GTK_TREE_MODEL(e_store));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tv),TRUE);
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(tv),GTK_TREE_VIEW_GRID_LINES_HORIZONTAL);

    struct{const char*t;int c;int wrap;}cols[]={
        {"Roll",COL_E_ROLL,0},{"Student",COL_E_NAME,0},{"Class",COL_E_CLASS,0},
        {"Subjects (Score%)",COL_E_SUBJECTS,1},{"Average",COL_E_AVG,0},{"Grade",COL_E_GRADE,0}};
    for(int i=0;i<6;i++){
        GtkCellRenderer *r=gtk_cell_renderer_text_new();
        g_object_set(r,"xpad",6,"ypad",4,"foreground","#1e293b","foreground-set",TRUE,NULL);
        if(cols[i].wrap) g_object_set(r,"wrap-mode",PANGO_WRAP_WORD,"wrap-width",320,NULL);
        GtkTreeViewColumn *c=gtk_tree_view_column_new_with_attributes(
            cols[i].t,r,"text",cols[i].c,NULL);
        gtk_tree_view_column_set_resizable(c,TRUE);
        if(cols[i].wrap) gtk_tree_view_column_set_expand(c,TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(tv),c);
    }

    GtkWidget *scroll=gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll,TRUE);
    gtk_widget_set_margin_start(scroll,12);gtk_widget_set_margin_end(scroll,12);
    gtk_widget_set_margin_top(scroll,8);gtk_widget_set_margin_bottom(scroll,8);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll),tv);
    gtk_box_append(GTK_BOX(vbox),scroll);

    GtkWidget *bot=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8);
    gtk_widget_add_css_class(bot,"toolbar");gtk_box_append(GTK_BOX(vbox),bot);
    GtkWidget *sp=gtk_label_new("");gtk_widget_set_hexpand(sp,TRUE);gtk_box_append(GTK_BOX(bot),sp);
    GtkWidget *del_btn=gtk_button_new_with_label("🗑 Delete Student's Exams");
    gtk_widget_add_css_class(del_btn,"btn-danger");gtk_box_append(GTK_BOX(bot),del_btn);

    g_signal_connect(add_btn,"clicked",G_CALLBACK(on_add_exam),tv);
    g_signal_connect(del_btn,"clicked",G_CALLBACK(on_delete_exam),tv);

    reload_exams();
    return vbox;
}

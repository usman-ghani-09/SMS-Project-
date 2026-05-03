#include "sms.h"
enum{COL_T_ID=0,COL_T_ROLL,COL_T_SNAME,COL_T_TNAME,COL_T_SUBJECT,
     COL_T_DATE,COL_T_TOTAL,COL_T_OBTAINED,COL_T_PCT,COL_T_STATUS,N_T_COLS};
static GtkListStore *t_store;
static void reload_tests(void){
    gtk_list_store_clear(t_store);
    FILE *fp=fopen(FILE_TESTS,"rb");if(!fp)return;
    StudentTest t;
    while(fread(&t,sizeof t,1,fp)){
        Student s=findStudentById(t.student_id);
        char ps[16];snprintf(ps,sizeof ps,"%.1f%%",t.percentage);
        GtkTreeIter it;gtk_list_store_append(t_store,&it);
        gtk_list_store_set(t_store,&it,
            COL_T_ID,t.test_id,COL_T_ROLL,s.roll_no,COL_T_SNAME,s.name,
            COL_T_TNAME,t.test_name,COL_T_SUBJECT,t.subject,COL_T_DATE,t.test_date,
            COL_T_TOTAL,t.total_marks,COL_T_OBTAINED,t.obtained,
            COL_T_PCT,ps,COL_T_STATUS,t.status,-1);
    }
    fclose(fp);
}
static int get_sel_tid(GtkTreeView *tv){
    GtkTreeSelection *sel=gtk_tree_view_get_selection(tv);
    GtkTreeModel *m;GtkTreeIter it;
    if(!gtk_tree_selection_get_selected(sel,&m,&it))return -1;
    int id;gtk_tree_model_get(m,&it,COL_T_ID,&id,-1);return id;
}
static void frow_t(GtkWidget *g,int row,const char *l,GtkWidget **o){
    GtkWidget *lw=gtk_label_new(l);gtk_widget_add_css_class(lw,"field-label");
    gtk_widget_set_halign(lw,GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(g),lw,0,row,1,1);
    GtkWidget *e=gtk_entry_new();gtk_widget_set_hexpand(e,TRUE);
    gtk_grid_attach(GTK_GRID(g),e,1,row,1,1);if(o)*o=e;
}
typedef struct{GtkWidget*e[6];GtkWidget*err,*dlg;}AddTestCtx;
static void cb_save_test(GtkButton *b,gpointer ud){
    (void)b;AddTestCtx *c=ud;
    const char *rs=gtk_editable_get_text(GTK_EDITABLE(c->e[0]));
    const char *tn=gtk_editable_get_text(GTK_EDITABLE(c->e[1]));
    const char *su=gtk_editable_get_text(GTK_EDITABLE(c->e[2]));
    const char *da=gtk_editable_get_text(GTK_EDITABLE(c->e[3]));
    const char *to=gtk_editable_get_text(GTK_EDITABLE(c->e[4]));
    const char *ob=gtk_editable_get_text(GTK_EDITABLE(c->e[5]));
    if(!*rs||!*tn||!*su||!*da||!*to||!*ob){
        gtk_label_set_text(GTK_LABEL(c->err),"⚠ Fill all required fields.");return;}
    Student s=findStudentByRoll(atoi(rs));
    if(s.student_id==0){gtk_label_set_text(GTK_LABEL(c->err),"⚠ Student not found.");return;}
    StudentTest t;memset(&t,0,sizeof t);
    t.test_id=getNextTestId();t.student_id=s.student_id;
    t.total_marks=atoi(to);t.obtained=atoi(ob);
    t.percentage=t.total_marks>0?(float)t.obtained/t.total_marks*100.f:0;
    strncpy(t.test_name,tn,49);strncpy(t.subject,su,49);strncpy(t.test_date,da,11);
    strncpy(t.status,t.percentage>=50.f?"Pass":"Fail",9);
    FILE *fp=fopen(FILE_TESTS,"ab");if(fp){fwrite(&t,sizeof t,1,fp);fclose(fp);}
    reload_tests();gtk_window_destroy(GTK_WINDOW(c->dlg));g_free(c);
}
static void on_add_test(GtkButton *btn,gpointer tv){
    (void)btn;(void)tv;
    GtkWidget *dlg=gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dlg),"Add Test");
    gtk_window_set_modal(GTK_WINDOW(dlg),TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
    gtk_window_set_default_size(GTK_WINDOW(dlg),380,-1);
    GtkWidget *vb=gtk_box_new(GTK_ORIENTATION_VERTICAL,10);
    gtk_widget_set_margin_start(vb,20);gtk_widget_set_margin_end(vb,20);
    gtk_widget_set_margin_top(vb,16);gtk_widget_set_margin_bottom(vb,16);
    gtk_window_set_child(GTK_WINDOW(dlg),vb);
    GtkWidget *h=gtk_label_new("Add Test Record");
    gtk_widget_add_css_class(h,"page-title");gtk_widget_set_halign(h,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vb),h);
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid),8);gtk_grid_set_column_spacing(GTK_GRID(grid),10);
    gtk_box_append(GTK_BOX(vb),grid);
    AddTestCtx *ctx=g_new0(AddTestCtx,1);
    frow_t(grid,0,"Student Roll No *",&ctx->e[0]);
    frow_t(grid,1,"Test Name *",&ctx->e[1]);
    frow_t(grid,2,"Subject *",&ctx->e[2]);
    frow_t(grid,3,"Test Date *",&ctx->e[3]);
    frow_t(grid,4,"Total Marks *",&ctx->e[4]);
    frow_t(grid,5,"Marks Obtained *",&ctx->e[5]);
    ctx->dlg=dlg;ctx->err=gtk_label_new("");
    gtk_widget_add_css_class(ctx->err,"login-err");gtk_box_append(GTK_BOX(vb),ctx->err);
    GtkWidget *br=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8);
    gtk_widget_set_halign(br,GTK_ALIGN_END);gtk_box_append(GTK_BOX(vb),br);
    GtkWidget *cancel=gtk_button_new_with_label("Cancel");gtk_box_append(GTK_BOX(br),cancel);
    g_signal_connect_swapped(cancel,"clicked",G_CALLBACK(gtk_window_destroy),dlg);
    GtkWidget *save=gtk_button_new_with_label("Save");
    gtk_widget_add_css_class(save,"btn-success");gtk_box_append(GTK_BOX(br),save);
    g_signal_connect(save,"clicked",G_CALLBACK(cb_save_test),ctx);
    gtk_window_present(GTK_WINDOW(dlg));
}
static void on_delete_test(GtkButton *btn,gpointer tv_ptr){
    (void)btn;int tid=get_sel_tid(GTK_TREE_VIEW(tv_ptr));
    if(tid<0){show_message_dialog(GTK_WINDOW(main_window),"","Select a test first.",GTK_MESSAGE_WARNING);return;}
    FILE *fp=fopen(FILE_TESTS,"rb");FILE *tmp=fopen("tmp_t.dat","wb");
    if(fp&&tmp){StudentTest t;
        while(fread(&t,sizeof t,1,fp))if(t.test_id!=tid)fwrite(&t,sizeof t,1,tmp);
        fclose(fp);fclose(tmp);remove(FILE_TESTS);rename("tmp_t.dat",FILE_TESTS);}
    reload_tests();
}
static void on_refresh_t(GtkButton *b,gpointer d){(void)b;(void)d;reload_tests();}
GtkWidget *build_tests_page(void){
    GtkWidget *vbox=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    GtkWidget *hdr=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,10);
    gtk_widget_add_css_class(hdr,"page-header");gtk_box_append(GTK_BOX(vbox),hdr);
    GtkWidget *tb=gtk_box_new(GTK_ORIENTATION_VERTICAL,2);
    gtk_widget_set_hexpand(tb,TRUE);gtk_box_append(GTK_BOX(hdr),tb);
    GtkWidget *title=gtk_label_new("📋  Tests");
    gtk_widget_add_css_class(title,"page-title");gtk_widget_set_halign(title,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(tb),title);
    GtkWidget *sub=gtk_label_new("Manage class test records");
    gtk_widget_add_css_class(sub,"page-subtitle");gtk_widget_set_halign(sub,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(tb),sub);
    GtkWidget *add_btn=gtk_button_new_with_label("＋ Add Test");
    gtk_widget_add_css_class(add_btn,"btn-primary");gtk_box_append(GTK_BOX(hdr),add_btn);
    GtkWidget *toolbar=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8);
    gtk_widget_add_css_class(toolbar,"toolbar");gtk_box_append(GTK_BOX(vbox),toolbar);
    GtkWidget *ref=gtk_button_new_with_label("↺ Refresh");
    g_signal_connect(ref,"clicked",G_CALLBACK(on_refresh_t),NULL);gtk_box_append(GTK_BOX(toolbar),ref);
    t_store=gtk_list_store_new(N_T_COLS,G_TYPE_INT,G_TYPE_INT,G_TYPE_STRING,
        G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_INT,G_TYPE_INT,G_TYPE_STRING,G_TYPE_STRING);
    GtkWidget *tv=gtk_tree_view_new_with_model(GTK_TREE_MODEL(t_store));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tv),TRUE);
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(tv),GTK_TREE_VIEW_GRID_LINES_HORIZONTAL);
    struct{const char*t;int c;}cols[]={
        {"ID",COL_T_ID},{"Roll",COL_T_ROLL},{"Student",COL_T_SNAME},
        {"Test",COL_T_TNAME},{"Subject",COL_T_SUBJECT},{"Date",COL_T_DATE},
        {"Total",COL_T_TOTAL},{"Obtained",COL_T_OBTAINED},{"%",COL_T_PCT},{"Status",COL_T_STATUS}};
    for(int i=0;i<10;i++){
        GtkCellRenderer *r=gtk_cell_renderer_text_new();
        g_object_set(r,"xpad",6,"ypad",4,"foreground","#1e293b","foreground-set",TRUE,NULL);
        GtkTreeViewColumn *c=gtk_tree_view_column_new_with_attributes(cols[i].t,r,"text",cols[i].c,NULL);
        gtk_tree_view_column_set_resizable(c,TRUE);gtk_tree_view_append_column(GTK_TREE_VIEW(tv),c);
    }
    GtkWidget *scroll=gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll,TRUE);
    gtk_widget_set_margin_start(scroll,12);gtk_widget_set_margin_end(scroll,12);
    gtk_widget_set_margin_top(scroll,8);gtk_widget_set_margin_bottom(scroll,8);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll),tv);gtk_box_append(GTK_BOX(vbox),scroll);
    GtkWidget *bot=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8);
    gtk_widget_add_css_class(bot,"toolbar");gtk_box_append(GTK_BOX(vbox),bot);
    GtkWidget *sp=gtk_label_new("");gtk_widget_set_hexpand(sp,TRUE);gtk_box_append(GTK_BOX(bot),sp);
    GtkWidget *del_btn=gtk_button_new_with_label("🗑 Delete");
    gtk_widget_add_css_class(del_btn,"btn-danger");gtk_box_append(GTK_BOX(bot),del_btn);
    g_signal_connect(add_btn,"clicked",G_CALLBACK(on_add_test),tv);
    g_signal_connect(del_btn,"clicked",G_CALLBACK(on_delete_test),tv);
    reload_tests();return vbox;
}

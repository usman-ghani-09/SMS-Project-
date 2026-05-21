#include "sms.h"
enum{COL_S_ID=0,COL_S_ROLL,COL_S_NAME,COL_S_CLASS,COL_S_SEC,
     COL_S_AGE,COL_S_GENDER,COL_S_PHONE,COL_S_EMAIL,N_S_COLS};
static GtkListStore *s_store;

static void reload_students(void){
    gtk_list_store_clear(s_store);
    FILE *fp=fopen(FILE_STUDENTS,"rb");if(!fp)return;
    Student s;
    while(fread(&s,sizeof s,1,fp)){
        GtkTreeIter it;gtk_list_store_append(s_store,&it);
        gtk_list_store_set(s_store,&it,
            COL_S_ID,s.student_id,COL_S_ROLL,s.roll_no,COL_S_NAME,s.name,
            COL_S_CLASS,s.class_name,COL_S_SEC,s.section,COL_S_AGE,s.age,
            COL_S_GENDER,s.gender,COL_S_PHONE,s.phone,COL_S_EMAIL,s.email,-1);
    }
    fclose(fp);
}
static int get_sel_id(GtkTreeView *tv){
    GtkTreeSelection *sel=gtk_tree_view_get_selection(tv);
    GtkTreeModel *m;GtkTreeIter it;
    if(!gtk_tree_selection_get_selected(sel,&m,&it))return -1;
    int id;gtk_tree_model_get(m,&it,COL_S_ID,&id,-1);return id;
}
static int get_sel_roll(GtkTreeView *tv){
    GtkTreeSelection *sel=gtk_tree_view_get_selection(tv);
    GtkTreeModel *m;GtkTreeIter it;
    if(!gtk_tree_selection_get_selected(sel,&m,&it))return -1;
    int r;gtk_tree_model_get(m,&it,COL_S_ROLL,&r,-1);return r;
}
static void frow(GtkWidget *g,int row,const char *l,GtkWidget **o){
    GtkWidget *lw=gtk_label_new(l);
    gtk_widget_add_css_class(lw,"field-label");
    gtk_widget_set_halign(lw,GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(g),lw,0,row,1,1);
    GtkWidget *e=gtk_entry_new();
    gtk_widget_set_hexpand(e,TRUE);
    gtk_grid_attach(GTK_GRID(g),e,1,row,1,1);
    if(o)*o=e;
}

/* ── ADD ── */
typedef struct{
    GtkWidget *e_roll,*e_name,*e_class,*e_sec,*e_age;
    GtkWidget *e_gender,*e_dob,*e_phone,*e_email,*e_enroll;
    GtkWidget *err,*dlg;
}AddCtx;
static void cb_save(GtkButton *b,gpointer ud){
    (void)b;AddCtx *c=ud;
    const char *rs=gtk_editable_get_text(GTK_EDITABLE(c->e_roll));
    const char *nm=gtk_editable_get_text(GTK_EDITABLE(c->e_name));
    const char *cl=gtk_editable_get_text(GTK_EDITABLE(c->e_class));
    const char *sc=gtk_editable_get_text(GTK_EDITABLE(c->e_sec));
    const char *ag=gtk_editable_get_text(GTK_EDITABLE(c->e_age));
    const char *gn=gtk_editable_get_text(GTK_EDITABLE(c->e_gender));
    if(!*rs||!*nm||!*cl||!*sc||!*ag||!*gn){
        gtk_label_set_text(GTK_LABEL(c->err),"⚠ Fill required fields.");return;}
    int roll=atoi(rs);
    /* Roll must be unique within same class AND section only */
    {
        FILE *_chk=fopen(FILE_STUDENTS,"rb"); Student _s;
        while(_chk && fread(&_s,sizeof _s,1,_chk)){
            if(_s.roll_no==roll &&
               strcasecmp(_s.class_name,cl)==0 &&
               strcasecmp(_s.section,sc)==0){
                fclose(_chk);
                gtk_label_set_text(GTK_LABEL(c->err),
                    "⚠ Roll No already exists in this Class & Section.");return;
            }
        }
        if(_chk) fclose(_chk);
    }
    Student s;memset(&s,0,sizeof s);
    s.student_id=getNextStudentId();s.roll_no=roll;s.age=atoi(ag);
    strncpy(s.name,nm,49);strncpy(s.class_name,cl,19);
    strncpy(s.section,sc,4);strncpy(s.gender,gn,9);
    strncpy(s.dob,gtk_editable_get_text(GTK_EDITABLE(c->e_dob)),11);
    strncpy(s.phone,gtk_editable_get_text(GTK_EDITABLE(c->e_phone)),14);
    strncpy(s.email,gtk_editable_get_text(GTK_EDITABLE(c->e_email)),49);
    strncpy(s.enroll_date,gtk_editable_get_text(GTK_EDITABLE(c->e_enroll)),11);
    FILE *fp=fopen(FILE_STUDENTS,"ab");
    if(fp){fwrite(&s,sizeof s,1,fp);fclose(fp);}
    reload_students(); refresh_dashboard();
    gtk_window_destroy(GTK_WINDOW(c->dlg));g_free(c);
}
static void on_add(GtkButton *btn,gpointer tv){
    (void)btn;(void)tv;
    GtkWidget *dlg=gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dlg),"Add Student");
    gtk_window_set_modal(GTK_WINDOW(dlg),TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
    gtk_window_set_default_size(GTK_WINDOW(dlg),400,-1);
    GtkWidget *vb=gtk_box_new(GTK_ORIENTATION_VERTICAL,10);
    gtk_widget_set_margin_start(vb,20);gtk_widget_set_margin_end(vb,20);
    gtk_widget_set_margin_top(vb,16);gtk_widget_set_margin_bottom(vb,16);
    gtk_window_set_child(GTK_WINDOW(dlg),vb);
    GtkWidget *h=gtk_label_new("Add New Student");
    gtk_widget_add_css_class(h,"page-title");gtk_widget_set_halign(h,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vb),h);
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid),8);
    gtk_grid_set_column_spacing(GTK_GRID(grid),10);
    gtk_box_append(GTK_BOX(vb),grid);
    AddCtx *ctx=g_new0(AddCtx,1);
    frow(grid,0,"Roll No *",&ctx->e_roll);
    frow(grid,1,"Full Name *",&ctx->e_name);
    frow(grid,2,"Class *",&ctx->e_class);
    frow(grid,3,"Section *",&ctx->e_sec);
    frow(grid,4,"Age *",&ctx->e_age);
    frow(grid,5,"Gender (M/F) *",&ctx->e_gender);
    frow(grid,6,"DOB (DD-MM-YYYY)",&ctx->e_dob);
    frow(grid,7,"Phone",&ctx->e_phone);
    frow(grid,8,"Email",&ctx->e_email);
    frow(grid,9,"Enroll Date",&ctx->e_enroll);
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
    g_signal_connect(save,"clicked",G_CALLBACK(cb_save),ctx);
    gtk_window_present(GTK_WINDOW(dlg));
}

/* ── EDIT ── */
typedef struct{GtkWidget *e_name,*e_class,*e_sec,*e_age,*e_phone,*e_email,*dlg;int student_id;}EditCtx;
static void cb_update(GtkButton *b,gpointer ud){
    (void)b;EditCtx *c=ud;
    Student u=findStudentById(c->student_id);
    strncpy(u.name,gtk_editable_get_text(GTK_EDITABLE(c->e_name)),49);
    strncpy(u.class_name,gtk_editable_get_text(GTK_EDITABLE(c->e_class)),19);
    strncpy(u.section,gtk_editable_get_text(GTK_EDITABLE(c->e_sec)),4);
    u.age=atoi(gtk_editable_get_text(GTK_EDITABLE(c->e_age)));
    strncpy(u.phone,gtk_editable_get_text(GTK_EDITABLE(c->e_phone)),14);
    strncpy(u.email,gtk_editable_get_text(GTK_EDITABLE(c->e_email)),49);
    FILE *fp=fopen(FILE_STUDENTS,"rb");FILE *tmp=fopen("tmp_s.dat","wb");
    if(fp&&tmp){Student s;while(fread(&s,sizeof s,1,fp))fwrite(s.student_id==u.student_id?&u:&s,sizeof s,1,tmp);
        fclose(fp);fclose(tmp);remove(FILE_STUDENTS);rename("tmp_s.dat",FILE_STUDENTS);}
    reload_students(); refresh_dashboard();gtk_window_destroy(GTK_WINDOW(c->dlg));g_free(c);
}
static void on_edit(GtkButton *btn,gpointer tv_ptr){
    (void)btn;int sid=get_sel_id(GTK_TREE_VIEW(tv_ptr));
    if(sid<0){show_message_dialog(GTK_WINDOW(main_window),"","Select a student first.",GTK_MESSAGE_WARNING);return;}
    Student old=findStudentById(sid);if(old.student_id==0)return;
    GtkWidget *dlg=gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dlg),"Edit Student");
    gtk_window_set_modal(GTK_WINDOW(dlg),TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
    gtk_window_set_default_size(GTK_WINDOW(dlg),380,-1);
    GtkWidget *vb=gtk_box_new(GTK_ORIENTATION_VERTICAL,10);
    gtk_widget_set_margin_start(vb,20);gtk_widget_set_margin_end(vb,20);
    gtk_widget_set_margin_top(vb,16);gtk_widget_set_margin_bottom(vb,16);
    gtk_window_set_child(GTK_WINDOW(dlg),vb);
    GtkWidget *h=gtk_label_new("Edit Student");
    gtk_widget_add_css_class(h,"page-title");gtk_widget_set_halign(h,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vb),h);
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid),8);
    gtk_grid_set_column_spacing(GTK_GRID(grid),10);
    gtk_box_append(GTK_BOX(vb),grid);
    EditCtx *ctx=g_new0(EditCtx,1);
    frow(grid,0,"Full Name",&ctx->e_name);frow(grid,1,"Class",&ctx->e_class);
    frow(grid,2,"Section",&ctx->e_sec);frow(grid,3,"Age",&ctx->e_age);
    frow(grid,4,"Phone",&ctx->e_phone);frow(grid,5,"Email",&ctx->e_email);
    ctx->dlg=dlg;ctx->student_id=old.student_id;
    char ab[8];snprintf(ab,sizeof ab,"%d",old.age);
    gtk_editable_set_text(GTK_EDITABLE(ctx->e_name),old.name);
    gtk_editable_set_text(GTK_EDITABLE(ctx->e_class),old.class_name);
    gtk_editable_set_text(GTK_EDITABLE(ctx->e_sec),old.section);
    gtk_editable_set_text(GTK_EDITABLE(ctx->e_age),ab);
    gtk_editable_set_text(GTK_EDITABLE(ctx->e_phone),old.phone);
    gtk_editable_set_text(GTK_EDITABLE(ctx->e_email),old.email);
    GtkWidget *br=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8);
    gtk_widget_set_halign(br,GTK_ALIGN_END);gtk_box_append(GTK_BOX(vb),br);
    GtkWidget *cancel=gtk_button_new_with_label("Cancel");
    gtk_box_append(GTK_BOX(br),cancel);
    g_signal_connect_swapped(cancel,"clicked",G_CALLBACK(gtk_window_destroy),dlg);
    GtkWidget *save=gtk_button_new_with_label("Update");
    gtk_widget_add_css_class(save,"btn-warning");gtk_box_append(GTK_BOX(br),save);
    g_signal_connect(save,"clicked",G_CALLBACK(cb_update),ctx);
    gtk_window_present(GTK_WINDOW(dlg));
}

/* ── DELETE ── */
typedef struct{GtkWidget *dlg;int student_id;}DelCtx;
static void cb_del(GtkButton *b,gpointer ud){
    (void)b;DelCtx *c=ud;
    FILE *fp=fopen(FILE_STUDENTS,"rb");FILE *tmp=fopen("tmp_s.dat","wb");
    if(fp&&tmp){Student s;while(fread(&s,sizeof s,1,fp))if(s.student_id!=c->student_id)fwrite(&s,sizeof s,1,tmp);
        fclose(fp);fclose(tmp);remove(FILE_STUDENTS);rename("tmp_s.dat",FILE_STUDENTS);}
    reload_students(); refresh_dashboard();gtk_window_destroy(GTK_WINDOW(c->dlg));g_free(c);
}
static void on_delete(GtkButton *btn,gpointer tv_ptr){
    (void)btn;int sid=get_sel_id(GTK_TREE_VIEW(tv_ptr));
    if(sid<0){show_message_dialog(GTK_WINDOW(main_window),"","Select a student first.",GTK_MESSAGE_WARNING);return;}
    Student _sd=findStudentById(sid);int roll=_sd.roll_no;
    GtkWidget *dlg=gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dlg),"Confirm Delete");
    gtk_window_set_modal(GTK_WINDOW(dlg),TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
    gtk_window_set_default_size(GTK_WINDOW(dlg),320,-1);
    GtkWidget *vb=gtk_box_new(GTK_ORIENTATION_VERTICAL,14);
    gtk_widget_set_margin_start(vb,20);gtk_widget_set_margin_end(vb,20);
    gtk_widget_set_margin_top(vb,16);gtk_widget_set_margin_bottom(vb,16);
    gtk_window_set_child(GTK_WINDOW(dlg),vb);
    char msg[128];snprintf(msg,sizeof msg,"Delete student with Roll No %d?\nThis cannot be undone.",roll);
    GtkWidget *lbl=gtk_label_new(msg);gtk_label_set_wrap(GTK_LABEL(lbl),TRUE);
    gtk_box_append(GTK_BOX(vb),lbl);
    GtkWidget *row=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8);
    gtk_widget_set_halign(row,GTK_ALIGN_END);gtk_box_append(GTK_BOX(vb),row);
    GtkWidget *cancel=gtk_button_new_with_label("Cancel");
    gtk_box_append(GTK_BOX(row),cancel);
    g_signal_connect_swapped(cancel,"clicked",G_CALLBACK(gtk_window_destroy),dlg);
    GtkWidget *del=gtk_button_new_with_label("Delete");
    gtk_widget_add_css_class(del,"btn-danger");gtk_box_append(GTK_BOX(row),del);
    DelCtx *ctx=g_new0(DelCtx,1);ctx->dlg=dlg;ctx->student_id=sid;
    g_signal_connect(del,"clicked",G_CALLBACK(cb_del),ctx);
    gtk_window_present(GTK_WINDOW(dlg));
}

/* ── VIEW PROFILE ── */
static void on_view_profile(GtkButton *btn,gpointer tv_ptr){
    (void)btn;
    int id=get_sel_id(GTK_TREE_VIEW(tv_ptr));
    if(id<0){show_message_dialog(GTK_WINDOW(main_window),"","Select a student first.",GTK_MESSAGE_WARNING);return;}
    refresh_profile(id);
    switch_to_content("profile");
}

/* ── SEARCH ── */
static void on_search(GtkSearchEntry *se,gpointer ud){
    (void)ud;
    const char *q=gtk_editable_get_text(GTK_EDITABLE(se));
    if(!*q){reload_students(); refresh_dashboard();return;}
    gtk_list_store_clear(s_store);
    FILE *fp=fopen(FILE_STUDENTS,"rb");if(!fp)return;
    char ql[64];strncpy(ql,q,63);ql[63]='\0';
    for(int i=0;ql[i];i++)if(ql[i]>='A'&&ql[i]<='Z')ql[i]+=32;
    Student s;
    while(fread(&s,sizeof s,1,fp)){
        char nl[50];strncpy(nl,s.name,49);nl[49]='\0';
        for(int i=0;nl[i];i++)if(nl[i]>='A'&&nl[i]<='Z')nl[i]+=32;
        char rs[16];snprintf(rs,sizeof rs,"%d",s.roll_no);
        if(strstr(nl,ql)||strstr(rs,q)||strstr(s.class_name,q)||strstr(s.phone,q)){
            GtkTreeIter it;gtk_list_store_append(s_store,&it);
            gtk_list_store_set(s_store,&it,
                COL_S_ID,s.student_id,COL_S_ROLL,s.roll_no,COL_S_NAME,s.name,
                COL_S_CLASS,s.class_name,COL_S_SEC,s.section,COL_S_AGE,s.age,
                COL_S_GENDER,s.gender,COL_S_PHONE,s.phone,COL_S_EMAIL,s.email,-1);
        }
    }
    fclose(fp);
}
static void on_refresh_s(GtkButton *b,gpointer d){(void)b;(void)d;reload_students(); refresh_dashboard();}

GtkWidget *build_students_page(void){
    GtkWidget *vbox=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);

    /* Header */
    GtkWidget *hdr=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,10);
    gtk_widget_add_css_class(hdr,"page-header");
    gtk_box_append(GTK_BOX(vbox),hdr);
    GtkWidget *tb=gtk_box_new(GTK_ORIENTATION_VERTICAL,2);
    gtk_widget_set_hexpand(tb,TRUE);gtk_box_append(GTK_BOX(hdr),tb);



    GtkWidget *title=gtk_label_new("Students");
    gtk_widget_add_css_class(title,"page-title");gtk_widget_set_halign(title,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(tb),title);
    GtkWidget *sub=gtk_label_new("Manage student records");
    gtk_widget_add_css_class(sub,"page-subtitle");gtk_widget_set_halign(sub,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(tb),sub);
    GtkWidget *add_btn=gtk_button_new_with_label("Add Student");
    gtk_widget_add_css_class(add_btn,"btn-primary");gtk_box_append(GTK_BOX(hdr),add_btn);

    /* Toolbar */
    GtkWidget *tb2=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8);
    gtk_widget_add_css_class(tb2,"toolbar");gtk_box_append(GTK_BOX(vbox),tb2);
    GtkWidget *se=gtk_search_entry_new();
    gtk_search_entry_set_placeholder_text(GTK_SEARCH_ENTRY(se),"Search by name, roll, class…");
    gtk_widget_set_hexpand(se,TRUE);gtk_box_append(GTK_BOX(tb2),se);
    GtkWidget *ref=gtk_button_new_with_label("↺");gtk_box_append(GTK_BOX(tb2),ref);

    /* List store */
    s_store=gtk_list_store_new(N_S_COLS,G_TYPE_INT,G_TYPE_INT,G_TYPE_STRING,
        G_TYPE_STRING,G_TYPE_STRING,G_TYPE_INT,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING);
    GtkWidget *tv=gtk_tree_view_new_with_model(GTK_TREE_MODEL(s_store));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tv),TRUE);
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(tv),GTK_TREE_VIEW_GRID_LINES_HORIZONTAL);

    struct{const char*t;int c;}cols[]={
        {"Roll",COL_S_ROLL},{"Name",COL_S_NAME},{"Class",COL_S_CLASS},
        {"Sec",COL_S_SEC},{"Age",COL_S_AGE},{"Gender",COL_S_GENDER},
        {"Phone",COL_S_PHONE},{"Email",COL_S_EMAIL}};
    for(int i=0;i<8;i++){
        GtkCellRenderer *r=gtk_cell_renderer_text_new();
        g_object_set(r,"xpad",6,"ypad",4,"foreground","#1e293b","foreground-set",TRUE,NULL);
        GtkTreeViewColumn *c=gtk_tree_view_column_new_with_attributes(cols[i].t,r,"text",cols[i].c,NULL);
        gtk_tree_view_column_set_resizable(c,TRUE);
        gtk_tree_view_column_set_sort_column_id(c,cols[i].c);
        gtk_tree_view_append_column(GTK_TREE_VIEW(tv),c);
    }

    GtkWidget *scroll=gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll,TRUE);
    gtk_widget_set_margin_start(scroll,12);gtk_widget_set_margin_end(scroll,12);
    gtk_widget_set_margin_top(scroll,8);gtk_widget_set_margin_bottom(scroll,8);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll),tv);
    gtk_box_append(GTK_BOX(vbox),scroll);

    /* Bottom bar */
    GtkWidget *bot=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8);
    gtk_widget_add_css_class(bot,"toolbar");gtk_box_append(GTK_BOX(vbox),bot);
    GtkWidget *sp=gtk_label_new("");gtk_widget_set_hexpand(sp,TRUE);gtk_box_append(GTK_BOX(bot),sp);
    GtkWidget *prof_btn=gtk_button_new_with_label("Profile");
    gtk_widget_add_css_class(prof_btn,"btn-info");gtk_box_append(GTK_BOX(bot),prof_btn);
    GtkWidget *edit_btn=gtk_button_new_with_label("Edit");
    gtk_widget_add_css_class(edit_btn,"btn-warning");gtk_box_append(GTK_BOX(bot),edit_btn);
    GtkWidget *del_btn=gtk_button_new_with_label("Delete");
    gtk_widget_add_css_class(del_btn,"btn-danger");gtk_box_append(GTK_BOX(bot),del_btn);

    g_signal_connect(add_btn,"clicked",G_CALLBACK(on_add),tv);
    g_signal_connect(prof_btn,"clicked",G_CALLBACK(on_view_profile),tv);
    g_signal_connect(edit_btn,"clicked",G_CALLBACK(on_edit),tv);
    g_signal_connect(del_btn,"clicked",G_CALLBACK(on_delete),tv);
    g_signal_connect(ref,"clicked",G_CALLBACK(on_refresh_s),NULL);
    g_signal_connect(se,"search-changed",G_CALLBACK(on_search),NULL);

    reload_students(); refresh_dashboard();
    return vbox;
}

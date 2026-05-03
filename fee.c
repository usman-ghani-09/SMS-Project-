#include "sms.h"
enum{COL_F_ID=0,COL_F_ROLL,COL_F_SNAME,COL_F_TYPE,COL_F_AMOUNT,
     COL_F_PAID,COL_F_BALANCE,COL_F_DUE,COL_F_MODE,COL_F_STATUS,N_F_COLS};
static GtkListStore *f_store;
static void reload_fees(void){
    gtk_list_store_clear(f_store);
    FILE *fp=fopen(FILE_FEES,"rb");if(!fp)return;
    StudentFee f;
    while(fread(&f,sizeof f,1,fp)){
        Student s=findStudentById(f.student_id);
        char am[16],pa[16],ba[16];
        snprintf(am,sizeof am,"Rs.%.0f",f.amount);
        snprintf(pa,sizeof pa,"Rs.%.0f",f.paid_amount);
        snprintf(ba,sizeof ba,"Rs.%.0f",f.balance);
        GtkTreeIter it;gtk_list_store_append(f_store,&it);
        gtk_list_store_set(f_store,&it,
            COL_F_ID,f.fee_id,COL_F_ROLL,s.roll_no,COL_F_SNAME,s.name,
            COL_F_TYPE,f.fee_type,COL_F_AMOUNT,am,COL_F_PAID,pa,
            COL_F_BALANCE,ba,COL_F_DUE,f.due_date,
            COL_F_MODE,f.payment_mode,COL_F_STATUS,f.status,-1);
    }
    fclose(fp);
}
static int get_sel_fid(GtkTreeView *tv){
    GtkTreeSelection *sel=gtk_tree_view_get_selection(tv);
    GtkTreeModel *m;GtkTreeIter it;
    if(!gtk_tree_selection_get_selected(sel,&m,&it))return -1;
    int id;gtk_tree_model_get(m,&it,COL_F_ID,&id,-1);return id;
}
static void frow_f(GtkWidget *g,int row,const char *l,GtkWidget **o){
    GtkWidget *lw=gtk_label_new(l);gtk_widget_add_css_class(lw,"field-label");
    gtk_widget_set_halign(lw,GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(g),lw,0,row,1,1);
    GtkWidget *e=gtk_entry_new();gtk_widget_set_hexpand(e,TRUE);
    gtk_grid_attach(GTK_GRID(g),e,1,row,1,1);if(o)*o=e;
}
typedef struct{GtkWidget*e[7];GtkWidget*err,*dlg;}AddFeeCtx;
static void cb_save_fee(GtkButton *b,gpointer ud){
    (void)b;AddFeeCtx *c=ud;
    const char *rs=gtk_editable_get_text(GTK_EDITABLE(c->e[0]));
    const char *ty=gtk_editable_get_text(GTK_EDITABLE(c->e[1]));
    const char *am=gtk_editable_get_text(GTK_EDITABLE(c->e[2]));
    const char *pa=gtk_editable_get_text(GTK_EDITABLE(c->e[3]));
    const char *du=gtk_editable_get_text(GTK_EDITABLE(c->e[4]));
    const char *pd=gtk_editable_get_text(GTK_EDITABLE(c->e[5]));
    const char *mo=gtk_editable_get_text(GTK_EDITABLE(c->e[6]));
    if(!*rs||!*ty||!*am||!*pa){
        gtk_label_set_text(GTK_LABEL(c->err),"⚠ Fill all required fields.");return;}
    Student s=findStudentByRoll(atoi(rs));
    if(s.student_id==0){gtk_label_set_text(GTK_LABEL(c->err),"⚠ Student not found.");return;}
    StudentFee f;memset(&f,0,sizeof f);
    f.fee_id=getNextFeeId();f.student_id=s.student_id;
    f.amount=atof(am);f.paid_amount=atof(pa);f.balance=f.amount-f.paid_amount;
    if(f.balance<=0){f.balance=0;strcpy(f.status,"Paid");}
    else if(f.paid_amount>0)strcpy(f.status,"Partial");
    else strcpy(f.status,"Unpaid");
    strncpy(f.fee_type,ty,29);strncpy(f.due_date,du,11);
    strncpy(f.pay_date,pd,11);strncpy(f.payment_mode,mo,19);
    FILE *fp=fopen(FILE_FEES,"ab");if(fp){fwrite(&f,sizeof f,1,fp);fclose(fp);}
    reload_fees();gtk_window_destroy(GTK_WINDOW(c->dlg));g_free(c);
}
static void on_add_fee(GtkButton *btn,gpointer tv){
    (void)btn;(void)tv;
    GtkWidget *dlg=gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dlg),"Add Fee Record");
    gtk_window_set_modal(GTK_WINDOW(dlg),TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
    gtk_window_set_default_size(GTK_WINDOW(dlg),380,-1);
    GtkWidget *vb=gtk_box_new(GTK_ORIENTATION_VERTICAL,10);
    gtk_widget_set_margin_start(vb,20);gtk_widget_set_margin_end(vb,20);
    gtk_widget_set_margin_top(vb,16);gtk_widget_set_margin_bottom(vb,16);
    gtk_window_set_child(GTK_WINDOW(dlg),vb);
    GtkWidget *h=gtk_label_new("Add Fee Record");
    gtk_widget_add_css_class(h,"page-title");gtk_widget_set_halign(h,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vb),h);
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid),8);gtk_grid_set_column_spacing(GTK_GRID(grid),10);
    gtk_box_append(GTK_BOX(vb),grid);
    AddFeeCtx *ctx=g_new0(AddFeeCtx,1);
    frow_f(grid,0,"Student Roll No *",&ctx->e[0]);
    frow_f(grid,1,"Fee Type *",&ctx->e[1]);
    frow_f(grid,2,"Total Amount (Rs.) *",&ctx->e[2]);
    frow_f(grid,3,"Amount Paid (Rs.) *",&ctx->e[3]);
    frow_f(grid,4,"Due Date (DD-MM-YYYY)",&ctx->e[4]);
    frow_f(grid,5,"Pay Date (DD-MM-YYYY)",&ctx->e[5]);
    frow_f(grid,6,"Payment Mode",&ctx->e[6]);
    gtk_entry_set_placeholder_text(GTK_ENTRY(ctx->e[1]),"Tuition/Library/Transport");
    gtk_entry_set_placeholder_text(GTK_ENTRY(ctx->e[6]),"Cash/Bank/Online");
    ctx->dlg=dlg;ctx->err=gtk_label_new("");
    gtk_widget_add_css_class(ctx->err,"login-err");gtk_box_append(GTK_BOX(vb),ctx->err);
    GtkWidget *br=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8);
    gtk_widget_set_halign(br,GTK_ALIGN_END);gtk_box_append(GTK_BOX(vb),br);
    GtkWidget *cancel=gtk_button_new_with_label("Cancel");gtk_box_append(GTK_BOX(br),cancel);
    g_signal_connect_swapped(cancel,"clicked",G_CALLBACK(gtk_window_destroy),dlg);
    GtkWidget *save=gtk_button_new_with_label("Save");
    gtk_widget_add_css_class(save,"btn-success");gtk_box_append(GTK_BOX(br),save);
    g_signal_connect(save,"clicked",G_CALLBACK(cb_save_fee),ctx);
    gtk_window_present(GTK_WINDOW(dlg));
}
typedef struct{GtkWidget*e_extra,*e_mode,*dlg;int fid;}PayCtx;
static void cb_pay(GtkButton *b,gpointer ud){
    (void)b;PayCtx *c=ud;
    float extra=atof(gtk_editable_get_text(GTK_EDITABLE(c->e_extra)));
    const char *mode=gtk_editable_get_text(GTK_EDITABLE(c->e_mode));
    FILE *fp=fopen(FILE_FEES,"rb");FILE *tmp=fopen("tmp_f.dat","wb");
    if(fp&&tmp){StudentFee f;
        while(fread(&f,sizeof f,1,fp)){
            if(f.fee_id==c->fid){
                f.paid_amount+=extra;f.balance=f.amount-f.paid_amount;
                if(f.balance<=0){f.balance=0;strcpy(f.status,"Paid");}
                else if(f.paid_amount>0)strcpy(f.status,"Partial");
                if(*mode)strncpy(f.payment_mode,mode,19);
            }
            fwrite(&f,sizeof f,1,tmp);
        }
        fclose(fp);fclose(tmp);remove(FILE_FEES);rename("tmp_f.dat",FILE_FEES);}
    reload_fees();gtk_window_destroy(GTK_WINDOW(c->dlg));g_free(c);
}
static void on_add_payment(GtkButton *btn,gpointer tv_ptr){
    (void)btn;int fid=get_sel_fid(GTK_TREE_VIEW(tv_ptr));
    if(fid<0){show_message_dialog(GTK_WINDOW(main_window),"","Select a fee record first.",GTK_MESSAGE_WARNING);return;}
    StudentFee ff;memset(&ff,0,sizeof ff);
    FILE *fp=fopen(FILE_FEES,"rb");
    if(fp){StudentFee f;while(fread(&f,sizeof f,1,fp))if(f.fee_id==fid){ff=f;break;}fclose(fp);}
    GtkWidget *dlg=gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dlg),"Add Payment");
    gtk_window_set_modal(GTK_WINDOW(dlg),TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
    gtk_window_set_default_size(GTK_WINDOW(dlg),340,-1);
    GtkWidget *vb=gtk_box_new(GTK_ORIENTATION_VERTICAL,10);
    gtk_widget_set_margin_start(vb,20);gtk_widget_set_margin_end(vb,20);
    gtk_widget_set_margin_top(vb,16);gtk_widget_set_margin_bottom(vb,16);
    gtk_window_set_child(GTK_WINDOW(dlg),vb);
    GtkWidget *h=gtk_label_new("Add Payment");
    gtk_widget_add_css_class(h,"page-title");gtk_widget_set_halign(h,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vb),h);
    char info[100];snprintf(info,sizeof info,"Fee: %s  |  Balance: Rs.%.0f",ff.fee_type,ff.balance);
    GtkWidget *il=gtk_label_new(info);gtk_widget_add_css_class(il,"page-subtitle");
    gtk_widget_set_halign(il,GTK_ALIGN_START);gtk_box_append(GTK_BOX(vb),il);
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid),8);gtk_grid_set_column_spacing(GTK_GRID(grid),10);
    gtk_box_append(GTK_BOX(vb),grid);
    PayCtx *ctx=g_new0(PayCtx,1);
    frow_f(grid,0,"Amount (Rs.)",&ctx->e_extra);
    frow_f(grid,1,"Payment Mode",&ctx->e_mode);
    if(*ff.payment_mode)gtk_editable_set_text(GTK_EDITABLE(ctx->e_mode),ff.payment_mode);
    ctx->dlg=dlg;ctx->fid=fid;
    GtkWidget *br=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8);
    gtk_widget_set_halign(br,GTK_ALIGN_END);gtk_box_append(GTK_BOX(vb),br);
    GtkWidget *cancel=gtk_button_new_with_label("Cancel");gtk_box_append(GTK_BOX(br),cancel);
    g_signal_connect_swapped(cancel,"clicked",G_CALLBACK(gtk_window_destroy),dlg);
    GtkWidget *save=gtk_button_new_with_label("Add Payment");
    gtk_widget_add_css_class(save,"btn-primary");gtk_box_append(GTK_BOX(br),save);
    g_signal_connect(save,"clicked",G_CALLBACK(cb_pay),ctx);
    gtk_window_present(GTK_WINDOW(dlg));
}
static void on_delete_fee(GtkButton *btn,gpointer tv_ptr){
    (void)btn;int fid=get_sel_fid(GTK_TREE_VIEW(tv_ptr));
    if(fid<0){show_message_dialog(GTK_WINDOW(main_window),"","Select a fee record first.",GTK_MESSAGE_WARNING);return;}
    FILE *fp=fopen(FILE_FEES,"rb");FILE *tmp=fopen("tmp_f.dat","wb");
    if(fp&&tmp){StudentFee f;
        while(fread(&f,sizeof f,1,fp))if(f.fee_id!=fid)fwrite(&f,sizeof f,1,tmp);
        fclose(fp);fclose(tmp);remove(FILE_FEES);rename("tmp_f.dat",FILE_FEES);}
    reload_fees();
}
static void on_refresh_f(GtkButton *b,gpointer d){(void)b;(void)d;reload_fees();}
GtkWidget *build_fees_page(void){
    GtkWidget *vbox=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    GtkWidget *hdr=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,10);
    gtk_widget_add_css_class(hdr,"page-header");gtk_box_append(GTK_BOX(vbox),hdr);
    GtkWidget *tb=gtk_box_new(GTK_ORIENTATION_VERTICAL,2);
    gtk_widget_set_hexpand(tb,TRUE);gtk_box_append(GTK_BOX(hdr),tb);
    GtkWidget *title=gtk_label_new("💰  Fees");
    gtk_widget_add_css_class(title,"page-title");gtk_widget_set_halign(title,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(tb),title);
    GtkWidget *sub=gtk_label_new("Manage fee records and payments");
    gtk_widget_add_css_class(sub,"page-subtitle");gtk_widget_set_halign(sub,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(tb),sub);
    GtkWidget *add_btn=gtk_button_new_with_label("＋ Add Fee");
    gtk_widget_add_css_class(add_btn,"btn-primary");gtk_box_append(GTK_BOX(hdr),add_btn);
    GtkWidget *toolbar=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8);
    gtk_widget_add_css_class(toolbar,"toolbar");gtk_box_append(GTK_BOX(vbox),toolbar);
    GtkWidget *ref=gtk_button_new_with_label("↺ Refresh");
    g_signal_connect(ref,"clicked",G_CALLBACK(on_refresh_f),NULL);gtk_box_append(GTK_BOX(toolbar),ref);
    f_store=gtk_list_store_new(N_F_COLS,G_TYPE_INT,G_TYPE_INT,G_TYPE_STRING,
        G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,
        G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING);
    GtkWidget *tv=gtk_tree_view_new_with_model(GTK_TREE_MODEL(f_store));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tv),TRUE);
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(tv),GTK_TREE_VIEW_GRID_LINES_HORIZONTAL);
    struct{const char*t;int c;}cols[]={
        {"Roll",COL_F_ROLL},{"Student",COL_F_SNAME},{"Fee Type",COL_F_TYPE},
        {"Amount",COL_F_AMOUNT},{"Paid",COL_F_PAID},{"Balance",COL_F_BALANCE},
        {"Due Date",COL_F_DUE},{"Mode",COL_F_MODE},{"Status",COL_F_STATUS}};
    for(int i=0;i<9;i++){
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
    GtkWidget *pay_btn=gtk_button_new_with_label("💳 Add Payment");
    gtk_widget_add_css_class(pay_btn,"btn-success");gtk_box_append(GTK_BOX(bot),pay_btn);
    GtkWidget *del_btn=gtk_button_new_with_label("🗑 Delete");
    gtk_widget_add_css_class(del_btn,"btn-danger");gtk_box_append(GTK_BOX(bot),del_btn);
    g_signal_connect(add_btn,"clicked",G_CALLBACK(on_add_fee),tv);
    g_signal_connect(pay_btn,"clicked",G_CALLBACK(on_add_payment),tv);
    g_signal_connect(del_btn,"clicked",G_CALLBACK(on_delete_fee),tv);
    reload_fees();return vbox;
}

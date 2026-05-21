#include "sms.h"

static GtkWidget *active_nav_btn=NULL;

/* ── Forward declarations for live-refresh ── */
static GtkWidget *lbl_total_students=NULL;
static GtkWidget *lbl_total_boys    =NULL;
static GtkWidget *lbl_total_girls   =NULL;
static GtkWidget *lbl_att_pct       =NULL;

static int count_records(const char *path,size_t sz){
    FILE *f=fopen(path,"rb");if(!f)return 0;
    fseek(f,0,SEEK_END);long b=ftell(f);fclose(f);
    return sz>0?(int)(b/sz):0;
}

/* Count students by gender (first letter match, case-insensitive) */
static int count_gender(char letter){
    FILE *f=fopen(FILE_STUDENTS,"rb");if(!f)return 0;
    Student s; int c=0;
    while(fread(&s,sizeof s,1,f)){
        char g=s.gender[0];
        if(g>='a'&&g<='z') g=g-'a'+'A';
        if(g==letter) c++;
    }
    fclose(f);
    return c;
}

/* Attendance percentage = Present / Total * 100 */
static float calc_att_pct(void){
    FILE *f=fopen(FILE_ATTENDANCE,"rb");if(!f)return 0.0f;
    Attendance a; int total=0,present=0;
    while(fread(&a,sizeof a,1,f)){
        total++;
        if(strcmp(a.status,"Present")==0) present++;
    }
    fclose(f);
    return total>0?(float)present/total*100.0f:0.0f;
}

/* ── Called every time the Home page is shown ── */
void refresh_dashboard(void){
    if(!lbl_total_students) return;
    int ns=count_records(FILE_STUDENTS,sizeof(Student));
    int nb=count_gender('M');
    int ng=count_gender('F');
    float ap=calc_att_pct();
    char buf[32];
    snprintf(buf,sizeof buf,"%d",ns);
    gtk_label_set_text(GTK_LABEL(lbl_total_students),buf);
    snprintf(buf,sizeof buf,"%d",nb);
    gtk_label_set_text(GTK_LABEL(lbl_total_boys),buf);
    snprintf(buf,sizeof buf,"%d",ng);
    gtk_label_set_text(GTK_LABEL(lbl_total_girls),buf);
    snprintf(buf,sizeof buf,"%.1f%%",ap);
    gtk_label_set_text(GTK_LABEL(lbl_att_pct),buf);
}

/* ── Nav helpers ── */
static void set_active(GtkWidget *btn){
    if(active_nav_btn) gtk_widget_remove_css_class(active_nav_btn,"active");
    active_nav_btn=btn;
    gtk_widget_add_css_class(btn,"active");
}

typedef struct{const char *page;GtkWidget *btn;}NavData;
static void on_nav(GtkButton *b,gpointer d){
    (void)b; NavData *nd=d;
    set_active(nd->btn);
    switch_to_content(nd->page);
    if(strcmp(nd->page,"home")==0) refresh_dashboard();
}
static void on_logout(GtkButton *b,gpointer d){(void)b;(void)d;switch_to_login();}

static GtkWidget *nav_btn(const char *icon_path,const char *label,const char *page){
    GtkWidget *btn=gtk_button_new();
    gtk_widget_add_css_class(btn,"nav-btn");
    gtk_button_set_has_frame(GTK_BUTTON(btn),FALSE);
    gtk_widget_set_halign(btn,GTK_ALIGN_FILL);

    GtkWidget *hbox=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,10);
    gtk_widget_set_halign(hbox,GTK_ALIGN_START);
    gtk_widget_set_margin_start(hbox,4);

    /* Icon — loads PNG from icons/ folder */
    GtkWidget *img=gtk_image_new_from_file(icon_path);
    gtk_image_set_pixel_size(GTK_IMAGE(img),24);
    gtk_widget_set_opacity(img,0.75);
    gtk_box_append(GTK_BOX(hbox),img);

    GtkWidget *lbl=gtk_label_new(label);
    gtk_widget_set_halign(lbl,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(hbox),lbl);

    gtk_button_set_child(GTK_BUTTON(btn),hbox);

    NavData *nd=g_new(NavData,1); nd->page=page; nd->btn=btn;
    g_signal_connect(btn,"clicked",G_CALLBACK(on_nav),nd);
    return btn;
}

GtkWidget *build_sidebar(void){
    GtkWidget *sb=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    gtk_widget_add_css_class(sb,"sidebar");
    gtk_widget_set_size_request(sb,175,-1);
    gtk_widget_set_vexpand(sb,TRUE);

    /* Logo area */
    GtkWidget *la=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8);
    gtk_widget_add_css_class(la,"sidebar-logo-area");
    gtk_box_append(GTK_BOX(sb),la);

    GtkWidget *ico=gtk_label_new("🎓");
    GtkCssProvider *ic=gtk_css_provider_new();
    gtk_css_provider_load_from_string(ic,".sb-ico{font-size:24px;}");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
        GTK_STYLE_PROVIDER(ic),GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(ic);
    gtk_widget_add_css_class(ico,"sb-ico");
    gtk_box_append(GTK_BOX(la),ico);

    GtkWidget *nm=gtk_label_new("SMS");
    gtk_widget_add_css_class(nm,"sidebar-app-name");
    gtk_box_append(GTK_BOX(la),nm);

    gtk_box_append(GTK_BOX(sb),gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    GtkWidget *sec=gtk_label_new("NAVIGATION");
    gtk_widget_add_css_class(sec,"sidebar-section");
    gtk_widget_set_halign(sec,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(sb),sec);

    GtkWidget *b_home=nav_btn("icons/home.png","Dashboard","home");
    GtkWidget *b_stu =nav_btn("icons/students.png","Students","students");
    GtkWidget *b_exam=nav_btn("icons/exams.png","Exams","exams");
    GtkWidget *b_test=nav_btn("icons/tests.png","Tests","tests");
    GtkWidget *b_fee =nav_btn("icons/fees.png","Fees","fees");
    GtkWidget *b_att =nav_btn("icons/attendance.png","Attendance","attendance");

    gtk_box_append(GTK_BOX(sb),b_home);
    gtk_box_append(GTK_BOX(sb),b_stu);
    gtk_box_append(GTK_BOX(sb),b_exam);
    gtk_box_append(GTK_BOX(sb),b_test);
    gtk_box_append(GTK_BOX(sb),b_fee);
    gtk_box_append(GTK_BOX(sb),b_att);

    set_active(b_home);

    GtkWidget *sp=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    gtk_widget_set_vexpand(sp,TRUE);
    gtk_box_append(GTK_BOX(sb),sp);

    gtk_box_append(GTK_BOX(sb),gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    GtkWidget *lo=gtk_button_new();
    {
        GtkWidget *lhbox=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,10);
        gtk_widget_set_halign(lhbox,GTK_ALIGN_START);
        gtk_widget_set_margin_start(lhbox,4);
        GtkWidget *lico=gtk_image_new_from_file("icons/logout.png");
        gtk_image_set_pixel_size(GTK_IMAGE(lico),18);
        gtk_widget_set_opacity(lico,0.6);
        gtk_box_append(GTK_BOX(lhbox),lico);
        GtkWidget *llbl=gtk_label_new("Logout");
        gtk_widget_set_halign(llbl,GTK_ALIGN_START);
        gtk_box_append(GTK_BOX(lhbox),llbl);
        gtk_button_set_child(GTK_BUTTON(lo),lhbox);
    }
    gtk_widget_add_css_class(lo,"btn-logout");

    g_signal_connect(lo,"clicked",G_CALLBACK(on_logout),NULL);
    gtk_box_append(GTK_BOX(sb),lo);

    return sb;
}

/* ── Stat card — returns card; stores number label in *out_lbl ── */
static GtkWidget *stat_card(const char *icon_path,const char *label,
                             const char *init_val,const char *color,
                             GtkWidget **out_lbl){
    GtkWidget *card=gtk_box_new(GTK_ORIENTATION_VERTICAL,4);
    gtk_widget_add_css_class(card,"card");
    gtk_widget_set_hexpand(card,TRUE);

    char css[200],cls[40];
    snprintf(cls,sizeof cls,"sc%s",label);
    for(int i=0;cls[i];i++) if(cls[i]==' '||cls[i]=='%') cls[i]='_';
    snprintf(css,sizeof css,".%s{border-top:3px solid %s;}",cls,color);
    GtkCssProvider *cp=gtk_css_provider_new();
    gtk_css_provider_load_from_string(cp,css);
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
        GTK_STYLE_PROVIDER(cp),GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(cp);
    gtk_widget_add_css_class(card,cls);

    GtkWidget *row=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,10);
    gtk_box_append(GTK_BOX(card),row);

/* Icon — loads PNG from icons/ folder */
    GtkWidget *img=gtk_image_new_from_file(icon_path);
    gtk_image_set_pixel_size(GTK_IMAGE(img),32);
    gtk_widget_set_opacity(img,0.75);
    gtk_box_append(GTK_BOX(row),img);

    GtkWidget *vb=gtk_box_new(GTK_ORIENTATION_VERTICAL,1);
    gtk_box_append(GTK_BOX(row),vb);

    GtkWidget *nl=gtk_label_new(init_val);
    gtk_widget_add_css_class(nl,"stat-number");
    gtk_widget_set_halign(nl,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vb),nl);
    if(out_lbl) *out_lbl=nl;

    GtkWidget *tl=gtk_label_new(label);
    gtk_widget_add_css_class(tl,"stat-label");
    gtk_widget_set_halign(tl,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vb),tl);

    return card;
}

GtkWidget *build_home_page(void){
    GtkWidget *scroll=gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC);

    GtkWidget *vb=gtk_box_new(GTK_ORIENTATION_VERTICAL,12);
    gtk_widget_set_margin_start(vb,18);gtk_widget_set_margin_end(vb,18);
    gtk_widget_set_margin_top(vb,16);gtk_widget_set_margin_bottom(vb,16);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll),vb);

    /* banner */
    GtkWidget *banner=gtk_box_new(GTK_ORIENTATION_VERTICAL,3);
    gtk_widget_add_css_class(banner,"card");
    GtkCssProvider *bc=gtk_css_provider_new();
    gtk_css_provider_load_from_string(bc,
        ".banner{background:linear-gradient(135deg,#1e3a5f,#2563eb);"
        "border-radius:8px;padding:16px;}");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
        GTK_STYLE_PROVIDER(bc),GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(bc);
    gtk_widget_add_css_class(banner,"banner");

    GtkWidget *wt=gtk_label_new("Welcome to Student Management System");
    GtkCssProvider *wc=gtk_css_provider_new();
    gtk_css_provider_load_from_string(wc,".wt{color:white;font-size:14px;font-weight:bold;}");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
        GTK_STYLE_PROVIDER(wc),GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(wc);
    gtk_widget_add_css_class(wt,"wt");
    gtk_widget_set_halign(wt,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(banner),wt);

    GtkWidget *ws=gtk_label_new("Manage students, exams, tests and fees from one place.");
    GtkCssProvider *wsc=gtk_css_provider_new();
    gtk_css_provider_load_from_string(wsc,".ws{color:rgba(255,255,255,0.8);font-size:11px;}");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
        GTK_STYLE_PROVIDER(wsc),GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(wsc);
    gtk_widget_add_css_class(ws,"ws");
    gtk_widget_set_halign(ws,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(banner),ws);
    gtk_box_append(GTK_BOX(vb),banner);

    GtkWidget *ov=gtk_label_new("System Overview");
    GtkCssProvider *ovc=gtk_css_provider_new();
    gtk_css_provider_load_from_string(ovc,".ov{font-size:12px;font-weight:bold;color:#1e3a5f;}");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
        GTK_STYLE_PROVIDER(ovc),GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(ovc);
    gtk_widget_add_css_class(ov,"ov");
    gtk_widget_set_halign(ov,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vb),ov);

    /* ── 4 stat cards in a 2x2 grid ── */
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid),8);
    gtk_grid_set_row_spacing(GTK_GRID(grid),8);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid),TRUE);

    gtk_grid_attach(GTK_GRID(grid),
        stat_card("icons/students.png","Total Students","0","#2563eb",&lbl_total_students),0,0,1,1);
    gtk_grid_attach(GTK_GRID(grid),
        stat_card("icons/boy.png","Total Boys",    "0","#16a34a",&lbl_total_boys),    1,0,1,1);
    gtk_grid_attach(GTK_GRID(grid),
        stat_card("icons/girl.png","Total Girls",   "0","#d97706",&lbl_total_girls),   0,1,1,1);
    gtk_grid_attach(GTK_GRID(grid),
        stat_card("icons/attendance.png","Attendance %",  "0.0%","#0891b2",&lbl_att_pct),   1,1,1,1);

    gtk_box_append(GTK_BOX(vb),grid);

    /* Populate on first build */
    refresh_dashboard();

    return scroll;
}

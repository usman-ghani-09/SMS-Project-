#include "sms.h"
static GtkWidget *entry_user,*entry_pass,*label_error;

static void seedAdmin(void){
    FILE *f=fopen(FILE_USERS,"rb");if(f){fclose(f);return;}
    f=fopen(FILE_USERS,"wb");if(!f)return;
    User a;memset(&a,0,sizeof a);
    a.user_id=1;a.is_active=1;
    strcpy(a.username,"admin");strcpy(a.password,"admin123");strcpy(a.role,"Admin");
    fwrite(&a,sizeof a,1,f);fclose(f);
}
static void do_login(void){
    const char *u=gtk_editable_get_text(GTK_EDITABLE(entry_user));
    const char *p=gtk_editable_get_text(GTK_EDITABLE(entry_pass));
    FILE *f=fopen(FILE_USERS,"rb");
    if(!f){gtk_label_set_text(GTK_LABEL(label_error),"Cannot open user file.");return;}
    User usr;
    while(fread(&usr,sizeof usr,1,f)){
        if(strcmp(usr.username,u)==0&&strcmp(usr.password,p)==0&&usr.is_active){
            fclose(f);
            gtk_label_set_text(GTK_LABEL(label_error),"");
            gtk_editable_set_text(GTK_EDITABLE(entry_pass),"");
            gtk_stack_set_visible_child_name(GTK_STACK(main_stack),"shell");
            return;
        }
    }
    fclose(f);
    gtk_label_set_text(GTK_LABEL(label_error),"⚠  Invalid username or password.");
}
static void on_login_clicked(GtkButton *b,gpointer d){(void)b;(void)d;do_login();}
static void on_entry_activate(GtkEntry *e,gpointer d){(void)e;(void)d;do_login();}

GtkWidget *build_login_page(void){
    seedAdmin();
    GtkWidget *page=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    gtk_widget_set_vexpand(page,TRUE);
    GtkCssProvider *bc=gtk_css_provider_new();
    gtk_css_provider_load_from_string(bc,".login-bg{background:linear-gradient(135deg,#1e3a5f,#2563eb);}");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
        GTK_STYLE_PROVIDER(bc),GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(bc);
    gtk_widget_add_css_class(page,"login-bg");

    GtkWidget *center=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    gtk_widget_set_valign(center,GTK_ALIGN_CENTER);
    gtk_widget_set_halign(center,GTK_ALIGN_CENTER);
    gtk_widget_set_vexpand(center,TRUE);
    gtk_box_append(GTK_BOX(page),center);

    GtkWidget *card=gtk_box_new(GTK_ORIENTATION_VERTICAL,12);
    gtk_widget_add_css_class(card,"login-box");
    gtk_box_append(GTK_BOX(center),card);

    GtkWidget *ico=gtk_label_new("🎓");
    GtkCssProvider *ic=gtk_css_provider_new();
    gtk_css_provider_load_from_string(ic,".l-ico{font-size:40px;}");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
        GTK_STYLE_PROVIDER(ic),GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(ic);
    gtk_widget_add_css_class(ico,"l-ico");
    gtk_widget_set_halign(ico,GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(card),ico);

    GtkWidget *title=gtk_label_new("Student Management System");
    gtk_widget_add_css_class(title,"login-title");
    gtk_widget_set_halign(title,GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(card),title);

    GtkWidget *sub=gtk_label_new("Sign in to continue");
    gtk_widget_add_css_class(sub,"login-subtitle");
    gtk_widget_set_halign(sub,GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(card),sub);

    gtk_box_append(GTK_BOX(card),gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    GtkWidget *lu=gtk_label_new("Username");
    gtk_widget_add_css_class(lu,"field-label");
    gtk_widget_set_halign(lu,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(card),lu);
    entry_user=gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_user),"Enter username");
    g_signal_connect(entry_user,"activate",G_CALLBACK(on_entry_activate),NULL);
    gtk_box_append(GTK_BOX(card),entry_user);

    GtkWidget *lp=gtk_label_new("Password");
    gtk_widget_add_css_class(lp,"field-label");
    gtk_widget_set_halign(lp,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(card),lp);
    entry_pass=gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(entry_pass),FALSE);
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_pass),"Enter password");
    g_signal_connect(entry_pass,"activate",G_CALLBACK(on_entry_activate),NULL);
    gtk_box_append(GTK_BOX(card),entry_pass);

    label_error=gtk_label_new("");
    gtk_widget_add_css_class(label_error,"login-err");
    gtk_widget_set_halign(label_error,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(card),label_error);

    GtkWidget *btn=gtk_button_new_with_label("Sign In  →");
    gtk_widget_add_css_class(btn,"login-btn");
    g_signal_connect(btn,"clicked",G_CALLBACK(on_login_clicked),NULL);
    gtk_box_append(GTK_BOX(card),btn);

    GtkWidget *hint=gtk_label_new("Default: admin / admin123");
    gtk_widget_add_css_class(hint,"login-hint");
    gtk_widget_set_halign(hint,GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(card),hint);

    return page;
}

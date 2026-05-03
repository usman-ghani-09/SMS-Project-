#include "sms.h"

GtkWidget *main_stack   = NULL;
GtkWidget *main_window  = NULL;
GtkWidget *content_stack= NULL;

static void apply_css(void){
    GtkCssProvider *p=gtk_css_provider_new();
    gtk_css_provider_load_from_string(p,
      
        "window{background:#f1f5f9;}"
        "*{font-family:'Segoe UI',Sans-Serif;font-size:13px;}"
        ".sidebar{background:#1e3a5f;min-width:175px;}"
        ".sidebar-logo-area{padding:14px 12px 10px 12px;}"
        ".sidebar-app-name{color:white;font-size:15px;font-weight:bold;}"
        ".sidebar-section{color:rgba(255,255,255,0.5);font-size:10px;"
        "  font-weight:bold;padding:10px 12px 4px 12px;letter-spacing:1px;}"
        ".nav-btn{background:none;border:none;border-radius:0;color:#94a3b8;"
        "  font-size:12px;padding:9px 12px;min-width:0;}"
        ".nav-btn:hover{background:rgba(255,255,255,0.07);color:#e2e8f0;}"
        ".nav-btn.active{background:rgba(255,255,255,0.13);color:#fff;"
        "  border-left:3px solid #60a5fa;}"
        ".btn-logout{background:rgba(255,255,255,0.06);border:1px solid rgba(255,255,255,0.2);"
        "  border-radius:5px;color:#94a3b8;font-size:11px;padding:6px 10px;margin:8px 10px;}"
        ".btn-logout:hover{background:rgba(255,255,255,0.12);color:#fff;}"
        ".page-header{background:white;border-bottom:1px solid #e2e8f0;padding:10px 18px;}"
        ".page-title{font-size:16px;font-weight:bold;color:#1e3a5f;}"
        ".page-subtitle{font-size:11px;color:#64748b;}"
        ".card{background:white;border-radius:8px;"
        "  box-shadow:0 1px 3px rgba(0,0,0,0.07);padding:14px;margin:4px;}"
        ".stat-number{font-size:24px;font-weight:bold;color:#1e3a5f;}"
        ".stat-label{font-size:11px;color:#64748b;}"
        ".btn-primary{background:#2563eb;color:#ffffff;border:none;"
        "  border-radius:5px;padding:6px 14px;font-size:12px;font-weight:600;}"
        ".btn-primary:hover{background:#1d4ed8;color:#ffffff;}"
        ".btn-danger{background:#dc2626;color:#ffffff;border:none;"
        "  border-radius:5px;padding:6px 14px;font-size:12px;font-weight:600;}"
        ".btn-danger:hover{background:#b91c1c;color:#ffffff;}"
        ".btn-success{background:#16a34a;color:#ffffff;border:none;"
        "  border-radius:5px;padding:6px 14px;font-size:12px;font-weight:600;}"
        ".btn-success:hover{background:#15803d;color:#ffffff;}"
        ".btn-warning{background:#d97706;color:#ffffff;border:none;"
        "  border-radius:5px;padding:6px 14px;font-size:12px;font-weight:600;}"
        ".btn-warning:hover{background:#b45309;color:#ffffff;}"
        ".btn-info{background:#0891b2;color:#ffffff;border:none;"
        "  border-radius:5px;padding:6px 14px;font-size:12px;font-weight:600;}"
        ".btn-info:hover{background:#0e7490;color:#ffffff;}"
        ".login-bg{background:linear-gradient(135deg,#1e3a5f,#2563eb);}"
        ".login-box{background:white;border-radius:10px;"
        "  box-shadow:0 8px 28px rgba(0,0,0,0.15);padding:32px;min-width:320px;}"
        ".login-title{font-size:18px;font-weight:bold;color:#1e3a5f;}"
        ".login-subtitle{font-size:11px;color:#64748b;}"
        ".login-btn{background:#2563eb;color:#ffffff;border:none;"
        "  border-radius:5px;padding:10px;font-size:13px;font-weight:bold;}"
        ".login-btn:hover{background:#1d4ed8;color:#ffffff;}"
        ".login-err{color:#dc2626;font-size:11px;}"
        ".login-hint{color:#94a3b8;font-size:10px;}"
        "treeview{background:white;font-size:12px;color:#1e293b;}"
        "treeview header button{background:#f8fafc;font-size:11px;font-weight:600;"
        "  color:#475569;border-bottom:1px solid #e2e8f0;padding:4px 6px;}"
        "treeview row{color:#1e293b;background:white;}"
        "treeview row:nth-child(even){background:#f8fafc;}"
        "treeview row:selected{background:#2563eb;color:#ffffff;}"
        "treeview row:hover:not(:selected){background:#eff6ff;color:#1e293b;}"
        "entry{border:1px solid #cbd5e1;border-radius:5px;"
        "  padding:6px 8px;font-size:12px;background:white;color:#1e293b;}"
        "entry:focus{border-color:#2563eb;}"
        ".field-label{font-size:11px;font-weight:600;color:#374151;}"
        ".toolbar{background:white;border-bottom:1px solid #e2e8f0;padding:6px 14px;}"

        ".profile-card{background:white;border-radius:8px;"
        "  box-shadow:0 1px 3px rgba(0,0,0,0.07);padding:16px;}"
        ".profile-name{font-size:16px;font-weight:bold;color:#1e3a5f;}"
        ".profile-sub{font-size:11px;color:#64748b;}"
        ".profile-field-label{font-size:10px;color:#94a3b8;font-weight:600;}"
        ".profile-field-value{font-size:12px;color:#1e293b;font-weight:500;}"
        ".section-heading{font-size:13px;font-weight:bold;color:#1e3a5f;"
        "  border-bottom:2px solid #2563eb;padding-bottom:4px;margin-bottom:8px;}"
        ".grade-A{color:#16a34a;font-weight:bold;}"
        ".grade-B{color:#2563eb;font-weight:bold;}"
        ".grade-C{color:#d97706;font-weight:bold;}"
        ".grade-F{color:#dc2626;font-weight:bold;}"
        ".pass-badge{color:#16a34a;font-weight:bold;}"
        ".fail-badge{color:#dc2626;font-weight:bold;}"
    );
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),GTK_STYLE_PROVIDER(p),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(p);
}

static void activate(GtkApplication *app,gpointer ud){
    (void)ud;
    apply_css();
    main_window=gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(main_window),"Student Management System");
    gtk_window_set_default_size(GTK_WINDOW(main_window),1050,650);

    main_stack=gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(main_stack),GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_set_transition_duration(GTK_STACK(main_stack),160);

    GtkWidget *shell=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
    gtk_box_append(GTK_BOX(shell),build_sidebar());

    content_stack=gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(content_stack),GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_set_transition_duration(GTK_STACK(content_stack),120);
    gtk_widget_set_hexpand(content_stack,TRUE);
    gtk_widget_set_vexpand(content_stack,TRUE);
    gtk_box_append(GTK_BOX(shell),content_stack);

    gtk_stack_add_named(GTK_STACK(content_stack),build_home_page(),    "home");
    gtk_stack_add_named(GTK_STACK(content_stack),build_students_page(),"students");
    gtk_stack_add_named(GTK_STACK(content_stack),build_exams_page(),   "exams");
    gtk_stack_add_named(GTK_STACK(content_stack),build_tests_page(),   "tests");
    gtk_stack_add_named(GTK_STACK(content_stack),build_fees_page(),    "fees");
    gtk_stack_add_named(GTK_STACK(content_stack),build_profile_page(),    "profile");
    gtk_stack_add_named(GTK_STACK(content_stack),build_attendance_page(),"attendance");
    gtk_stack_set_visible_child_name(GTK_STACK(content_stack),"home");

    gtk_stack_add_named(GTK_STACK(main_stack),build_login_page(),"login");
    gtk_stack_add_named(GTK_STACK(main_stack),shell,"shell");
    gtk_stack_set_visible_child_name(GTK_STACK(main_stack),"login");

    gtk_window_set_child(GTK_WINDOW(main_window),main_stack);
    gtk_window_present(GTK_WINDOW(main_window));
}

int main(int argc,char *argv[]){
    GtkApplication *app=gtk_application_new("com.sms.gtk4",G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app,"activate",G_CALLBACK(activate),NULL);
    int s=g_application_run(G_APPLICATION(app),argc,argv);
    g_object_unref(app);
    return s;
}

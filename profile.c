#include "sms.h"

/* Dynamic label pairs updated on refresh */
static GtkWidget *p_name_lbl,*p_roll_lbl,*p_class_lbl,*p_age_lbl;
static GtkWidget *p_gender_lbl,*p_phone_lbl,*p_email_lbl,*p_dob_lbl,*p_enroll_lbl;
static GtkWidget *p_exam_box,*p_test_box,*p_fee_box;
static GtkWidget *p_avg_exam_lbl,*p_avg_test_lbl,*p_fee_status_lbl;
static GtkWidget *p_avatar_img;

static GtkWidget *info_row(GtkWidget *grid,int row,const char *field,GtkWidget **val_out){
    GtkWidget *lbl=gtk_label_new(field);
    gtk_widget_add_css_class(lbl,"profile-field-label");
    gtk_widget_set_halign(lbl,GTK_ALIGN_END);
    gtk_widget_set_valign(lbl,GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid),lbl,0,row,1,1);
    GtkWidget *val=gtk_label_new("—");
    gtk_widget_add_css_class(val,"profile-field-value");
    gtk_widget_set_halign(val,GTK_ALIGN_START);
    gtk_label_set_wrap(GTK_LABEL(val),TRUE);
    gtk_grid_attach(GTK_GRID(grid),val,1,row,1,1);
    if(val_out)*val_out=val;
    return val;
}

static void section_heading(GtkWidget *box,const char *txt){
    GtkWidget *lbl=gtk_label_new(txt);
    gtk_widget_add_css_class(lbl,"section-heading");
    gtk_widget_set_halign(lbl,GTK_ALIGN_START);
    gtk_widget_set_margin_top(lbl,10);
    gtk_box_append(GTK_BOX(box),lbl);
}

void refresh_profile(int student_id){
    Student s=findStudentById(student_id);
    if(s.student_id==0)return;

    /* Update avatar based on gender */
    if(p_avatar_img){
        const char *icon_file=(s.gender[0]=='M' || s.gender[0]=='m') ? "icons/boy.png" : "icons/girl.png";
        gtk_image_set_from_file(GTK_IMAGE(p_avatar_img),icon_file);
    }

    /* Basic info */
    gtk_label_set_text(GTK_LABEL(p_name_lbl),  s.name);
    char buf[64];
    snprintf(buf,sizeof buf,"Roll: %d",s.roll_no);
    gtk_label_set_text(GTK_LABEL(p_roll_lbl),  buf);
    snprintf(buf,sizeof buf,"%s  —  %s",s.class_name,s.section);
    gtk_label_set_text(GTK_LABEL(p_class_lbl), buf);
    snprintf(buf,sizeof buf,"%d years",s.age);
    gtk_label_set_text(GTK_LABEL(p_age_lbl),   buf);
    gtk_label_set_text(GTK_LABEL(p_gender_lbl),s.gender);
    gtk_label_set_text(GTK_LABEL(p_phone_lbl), s.phone[0]?s.phone:"—");
    gtk_label_set_text(GTK_LABEL(p_email_lbl), s.email[0]?s.email:"—");
    gtk_label_set_text(GTK_LABEL(p_dob_lbl),   s.dob[0]?s.dob:"—");
    gtk_label_set_text(GTK_LABEL(p_enroll_lbl),s.enroll_date[0]?s.enroll_date:"—");

    /* Clear dynamic sections */
    GtkWidget *child;
    while((child=gtk_widget_get_first_child(p_exam_box)))gtk_box_remove(GTK_BOX(p_exam_box),child);
    while((child=gtk_widget_get_first_child(p_test_box)))gtk_box_remove(GTK_BOX(p_test_box),child);
    while((child=gtk_widget_get_first_child(p_fee_box))) gtk_box_remove(GTK_BOX(p_fee_box), child);

    /* ── Exams ── */
    FILE *fp=fopen(FILE_EXAMS,"rb");
    float total_pct=0; int exam_count=0;
    if(fp){
        StudentExam e;
        while(fread(&e,sizeof e,1,fp)){
            if(e.student_id!=student_id)continue;
            exam_count++;
            total_pct+=e.percentage;

            GtkWidget *row=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
            GtkCssProvider *rc=gtk_css_provider_new();
            gtk_css_provider_load_from_string(rc,
                ".exam-row{background:white;border-radius:6px;padding:8px 12px;"
                "border-left:3px solid #2563eb;margin-bottom:4px;}");
            gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                GTK_STYLE_PROVIDER(rc),GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
            g_object_unref(rc);
            gtk_widget_add_css_class(row,"exam-row");

            /* Subject */
            GtkWidget *subj=gtk_label_new(e.subject);
            GtkCssProvider *sc2=gtk_css_provider_new();
            gtk_css_provider_load_from_string(sc2,".e-subj{font-weight:600;font-size:12px;color:#1e3a5f;}");
            gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                GTK_STYLE_PROVIDER(sc2),GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
            g_object_unref(sc2);
            gtk_widget_add_css_class(subj,"e-subj");
            gtk_widget_set_hexpand(subj,TRUE);
            gtk_widget_set_halign(subj,GTK_ALIGN_START);
            gtk_box_append(GTK_BOX(row),subj);

            char marks[32];
            snprintf(marks,sizeof marks,"%d/%d",e.obtained,e.total_marks);
            GtkWidget *ml=gtk_label_new(marks);
            gtk_widget_add_css_class(ml,"profile-field-value");
            gtk_widget_set_margin_end(ml,12);
            gtk_box_append(GTK_BOX(row),ml);

            char pct[16]; snprintf(pct,sizeof pct,"%.1f%%",e.percentage);
            GtkWidget *pl=gtk_label_new(pct);
            gtk_widget_set_margin_end(pl,12);
            gtk_widget_add_css_class(pl,"profile-field-value");
            gtk_box_append(GTK_BOX(row),pl);

            GtkWidget *gl=gtk_label_new(e.grade);
            const char *gcls=e.percentage>=70?"grade-A":e.percentage>=50?"grade-B":"grade-F";
            gtk_widget_add_css_class(gl,gcls);
            gtk_box_append(GTK_BOX(row),gl);

            gtk_box_append(GTK_BOX(p_exam_box),row);
        }
        fclose(fp);
    }
    if(exam_count==0){
        GtkWidget *nl=gtk_label_new("No exam records found.");
        gtk_widget_add_css_class(nl,"profile-field-label");
        gtk_widget_set_halign(nl,GTK_ALIGN_START);
        gtk_box_append(GTK_BOX(p_exam_box),nl);
        gtk_label_set_text(GTK_LABEL(p_avg_exam_lbl),"Average: —");
    } else {
        char avg[64];
        snprintf(avg,sizeof avg,"Average: %.1f%%  |  Grade: %s",
                 total_pct/exam_count, gradeFromPercent(total_pct/exam_count));
        gtk_label_set_text(GTK_LABEL(p_avg_exam_lbl),avg);
    }

    /* ── Tests ── */
    fp=fopen(FILE_TESTS,"rb");
    float t_pct=0; int t_count=0,t_pass=0;
    if(fp){
        StudentTest t;
        while(fread(&t,sizeof t,1,fp)){
            if(t.student_id!=student_id)continue;
            t_count++; t_pct+=t.percentage;
            if(strcmp(t.status,"Pass")==0)t_pass++;

            GtkWidget *row=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
            GtkCssProvider *rc=gtk_css_provider_new();
            gtk_css_provider_load_from_string(rc,
                ".test-row{background:white;border-radius:6px;padding:8px 12px;"
                "border-left:3px solid #d97706;margin-bottom:4px;}");
            gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                GTK_STYLE_PROVIDER(rc),GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
            g_object_unref(rc);
            gtk_widget_add_css_class(row,"test-row");

            char tname[80]; snprintf(tname,sizeof tname,"%s — %s",t.test_name,t.subject);
            GtkWidget *tn=gtk_label_new(tname);
            GtkCssProvider *tc=gtk_css_provider_new();
            gtk_css_provider_load_from_string(tc,".t-name{font-weight:600;font-size:12px;color:#1e3a5f;}");
            gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                GTK_STYLE_PROVIDER(tc),GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
            g_object_unref(tc);
            gtk_widget_add_css_class(tn,"t-name");
            gtk_widget_set_hexpand(tn,TRUE);
            gtk_widget_set_halign(tn,GTK_ALIGN_START);
            gtk_box_append(GTK_BOX(row),tn);

            char marks[32]; snprintf(marks,sizeof marks,"%d/%d",t.obtained,t.total_marks);
            GtkWidget *ml=gtk_label_new(marks);
            gtk_widget_add_css_class(ml,"profile-field-value");
            gtk_widget_set_margin_end(ml,12);
            gtk_box_append(GTK_BOX(row),ml);

            char ps[16]; snprintf(ps,sizeof ps,"%.1f%%",t.percentage);
            GtkWidget *pl=gtk_label_new(ps);
            gtk_widget_add_css_class(pl,"profile-field-value");
            gtk_widget_set_margin_end(pl,12);
            gtk_box_append(GTK_BOX(row),pl);

            GtkWidget *sl=gtk_label_new(t.status);
            gtk_widget_add_css_class(sl,strcmp(t.status,"Pass")==0?"pass-badge":"fail-badge");
            gtk_box_append(GTK_BOX(row),sl);

            gtk_box_append(GTK_BOX(p_test_box),row);
        }
        fclose(fp);
    }
    if(t_count==0){
        GtkWidget *nl=gtk_label_new("No test records found.");
        gtk_widget_add_css_class(nl,"profile-field-label");
        gtk_widget_set_halign(nl,GTK_ALIGN_START);
        gtk_box_append(GTK_BOX(p_test_box),nl);
        gtk_label_set_text(GTK_LABEL(p_avg_test_lbl),"Average: —");
    } else {
        char avg[80];
        snprintf(avg,sizeof avg,"Average: %.1f%%  |  Passed: %d/%d",
                 t_pct/t_count, t_pass, t_count);
        gtk_label_set_text(GTK_LABEL(p_avg_test_lbl),avg);
    }

    /* ── Fees ── */
    fp=fopen(FILE_FEES,"rb");
    float f_total=0,f_paid=0,f_bal=0; int f_count=0;
    if(fp){
        StudentFee f;
        while(fread(&f,sizeof f,1,fp)){
            if(f.student_id!=student_id)continue;
            f_count++; f_total+=f.amount; f_paid+=f.paid_amount; f_bal+=f.balance;

            GtkWidget *row=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
            GtkCssProvider *rc=gtk_css_provider_new();
            gtk_css_provider_load_from_string(rc,
                ".fee-row{background:white;border-radius:6px;padding:8px 12px;"
                "border-left:3px solid #16a34a;margin-bottom:4px;}");
            gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                GTK_STYLE_PROVIDER(rc),GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
            g_object_unref(rc);
            gtk_widget_add_css_class(row,"fee-row");

            GtkWidget *tn=gtk_label_new(f.fee_type);
            GtkCssProvider *tc=gtk_css_provider_new();
            gtk_css_provider_load_from_string(tc,".f-type{font-weight:600;font-size:12px;color:#1e3a5f;}");
            gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                GTK_STYLE_PROVIDER(tc),GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
            g_object_unref(tc);
            gtk_widget_add_css_class(tn,"f-type");
            gtk_widget_set_hexpand(tn,TRUE);
            gtk_widget_set_halign(tn,GTK_ALIGN_START);
            gtk_box_append(GTK_BOX(row),tn);

            char amt[40]; snprintf(amt,sizeof amt,"Rs.%.0f / Paid:%.0f",f.amount,f.paid_amount);
            GtkWidget *al=gtk_label_new(amt);
            gtk_widget_add_css_class(al,"profile-field-value");
            gtk_widget_set_margin_end(al,12);
            gtk_box_append(GTK_BOX(row),al);

            GtkWidget *sl=gtk_label_new(f.status);
            const char *scls=strcmp(f.status,"Paid")==0?"pass-badge":
                             strcmp(f.status,"Partial")==0?"grade-C":"fail-badge";
            gtk_widget_add_css_class(sl,scls);
            gtk_box_append(GTK_BOX(row),sl);

            gtk_box_append(GTK_BOX(p_fee_box),row);
        }
        fclose(fp);
    }
    if(f_count==0){
        GtkWidget *nl=gtk_label_new("No fee records found.");
        gtk_widget_add_css_class(nl,"profile-field-label");
        gtk_widget_set_halign(nl,GTK_ALIGN_START);
        gtk_box_append(GTK_BOX(p_fee_box),nl);
        gtk_label_set_text(GTK_LABEL(p_fee_status_lbl),"Total: —");
    } else {
        char fs[100];
        snprintf(fs,sizeof fs,"Total: Rs.%.0f  |  Paid: Rs.%.0f  |  Balance: Rs.%.0f",
                 f_total,f_paid,f_bal);
        gtk_label_set_text(GTK_LABEL(p_fee_status_lbl),fs);
    }
}

static void on_back_to_students(GtkButton *b,gpointer d){
    (void)b;(void)d; switch_to_content("students");
}

GtkWidget *build_profile_page(void){
    GtkWidget *outer=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);

    /* Header */
    GtkWidget *hdr=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,10);
    gtk_widget_add_css_class(hdr,"page-header");
    gtk_box_append(GTK_BOX(outer),hdr);

    GtkWidget *back=gtk_button_new_with_label("Back");
    gtk_widget_add_css_class(back,"btn-info");
    g_signal_connect(back,"clicked",G_CALLBACK(on_back_to_students),NULL);
    gtk_box_append(GTK_BOX(hdr),back);

    GtkWidget *tb=gtk_box_new(GTK_ORIENTATION_VERTICAL,2);
    gtk_widget_set_hexpand(tb,TRUE);gtk_box_append(GTK_BOX(hdr),tb);
    GtkWidget *title=gtk_label_new("Student Profile");
    gtk_widget_add_css_class(title,"page-title");gtk_widget_set_halign(title,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(tb),title);
    GtkWidget *sub=gtk_label_new("Full information and academic progress");
    gtk_widget_add_css_class(sub,"page-subtitle");gtk_widget_set_halign(sub,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(tb),sub);

    /* Scrollable body */
    GtkWidget *scroll=gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll,TRUE);
    gtk_box_append(GTK_BOX(outer),scroll);

    GtkWidget *body=gtk_box_new(GTK_ORIENTATION_VERTICAL,10);
    gtk_widget_set_margin_start(body,16);gtk_widget_set_margin_end(body,16);
    gtk_widget_set_margin_top(body,14);gtk_widget_set_margin_bottom(body,14);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll),body);

    /* ── Top: avatar + basic info side by side ── */
    GtkWidget *top=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,12);
    gtk_box_append(GTK_BOX(body),top);

    /* Avatar card */
    GtkWidget *av_card=gtk_box_new(GTK_ORIENTATION_VERTICAL,6);
    gtk_widget_add_css_class(av_card,"profile-card");
    gtk_widget_set_valign(av_card,GTK_ALIGN_START);
    gtk_widget_set_size_request(av_card,140,-1);
    gtk_box_append(GTK_BOX(top),av_card);

    GtkWidget *hbox=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,6);
    gtk_widget_set_halign(hbox,GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(av_card),hbox);

    p_avatar_img=gtk_image_new_from_file("icons/girl.png");
    gtk_image_set_pixel_size(GTK_IMAGE(p_avatar_img),64);
    gtk_widget_set_opacity(p_avatar_img,0.75);
    gtk_box_append(GTK_BOX(hbox),p_avatar_img);

    p_name_lbl=gtk_label_new("—");
    gtk_widget_add_css_class(p_name_lbl,"profile-name");
    gtk_label_set_wrap(GTK_LABEL(p_name_lbl),TRUE);
    gtk_widget_set_halign(p_name_lbl,GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(av_card),p_name_lbl);

    p_roll_lbl=gtk_label_new("Roll: —");
    gtk_widget_add_css_class(p_roll_lbl,"profile-sub");
    gtk_widget_set_halign(p_roll_lbl,GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(av_card),p_roll_lbl);

    p_class_lbl=gtk_label_new("—");
    gtk_widget_add_css_class(p_class_lbl,"profile-sub");
    gtk_widget_set_halign(p_class_lbl,GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(av_card),p_class_lbl);

    /* Info grid card */
    GtkWidget *info_card=gtk_box_new(GTK_ORIENTATION_VERTICAL,8);
    gtk_widget_add_css_class(info_card,"profile-card");
    gtk_widget_set_hexpand(info_card,TRUE);
    gtk_widget_set_valign(info_card,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(top),info_card);

    GtkWidget *ih=gtk_label_new("Personal Information");
    gtk_widget_add_css_class(ih,"section-heading");
    gtk_widget_set_halign(ih,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(info_card),ih);

    GtkWidget *igrid=gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(igrid),6);
    gtk_grid_set_column_spacing(GTK_GRID(igrid),16);
    gtk_box_append(GTK_BOX(info_card),igrid);

    info_row(igrid,0,"AGE",      &p_age_lbl);
    info_row(igrid,1,"GENDER",   &p_gender_lbl);
    info_row(igrid,2,"PHONE",    &p_phone_lbl);
    info_row(igrid,3,"EMAIL",    &p_email_lbl);
    info_row(igrid,4,"DOB",      &p_dob_lbl);
    info_row(igrid,5,"ENROLLED", &p_enroll_lbl);

    /* ── Exams section ── */
    GtkWidget *exam_card=gtk_box_new(GTK_ORIENTATION_VERTICAL,6);
    gtk_widget_add_css_class(exam_card,"profile-card");
    gtk_box_append(GTK_BOX(body),exam_card);

    section_heading(exam_card,"Exam Results");

    p_exam_box=gtk_box_new(GTK_ORIENTATION_VERTICAL,4);
    gtk_box_append(GTK_BOX(exam_card),p_exam_box);

    p_avg_exam_lbl=gtk_label_new("Average: —");
    GtkCssProvider *avgc=gtk_css_provider_new();
    gtk_css_provider_load_from_string(avgc,".avg-lbl{font-size:11px;font-weight:600;color:#2563eb;margin-top:4px;}");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
        GTK_STYLE_PROVIDER(avgc),GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(avgc);
    gtk_widget_add_css_class(p_avg_exam_lbl,"avg-lbl");
    gtk_widget_set_halign(p_avg_exam_lbl,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(exam_card),p_avg_exam_lbl);

    /* ── Tests section ── */
    GtkWidget *test_card=gtk_box_new(GTK_ORIENTATION_VERTICAL,6);
    gtk_widget_add_css_class(test_card,"profile-card");
    gtk_box_append(GTK_BOX(body),test_card);

    section_heading(test_card,"Test Results");

    p_test_box=gtk_box_new(GTK_ORIENTATION_VERTICAL,4);
    gtk_box_append(GTK_BOX(test_card),p_test_box);

    p_avg_test_lbl=gtk_label_new("Average: —");
    gtk_widget_add_css_class(p_avg_test_lbl,"avg-lbl");
    gtk_widget_set_halign(p_avg_test_lbl,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(test_card),p_avg_test_lbl);

    /* ── Fees section ── */
    GtkWidget *fee_card=gtk_box_new(GTK_ORIENTATION_VERTICAL,6);
    gtk_widget_add_css_class(fee_card,"profile-card");
    gtk_box_append(GTK_BOX(body),fee_card);

    section_heading(fee_card,"Fee Summary");

    p_fee_box=gtk_box_new(GTK_ORIENTATION_VERTICAL,4);
    gtk_box_append(GTK_BOX(fee_card),p_fee_box);

    p_fee_status_lbl=gtk_label_new("Total: —");
    gtk_widget_add_css_class(p_fee_status_lbl,"avg-lbl");
    gtk_widget_set_halign(p_fee_status_lbl,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(fee_card),p_fee_status_lbl);

    return outer;
}

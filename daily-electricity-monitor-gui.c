#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COST_PER_KWH 6.0 
#define MAX_APP 50
#define MONTHLY_FREE_UNITS 200.0

typedef struct {
    char name[100];
    float watts;
    float duration;
} ApplianceData;

typedef struct {
    GtkWidget *count_entry;
    GtkWidget *name_entry;
    GtkWidget *power_entry;
    GtkWidget *duration_entry;
    GtkWidget *month_entry;
    GtkWidget *status_label;
    GtkWidget *result_label;
    GtkWidget *setup_grid;
    GtkWidget *usage_grid;
    GtkListStore *list_store;
} AppData;

ApplianceData app_list[MAX_APP];
int total_apps = 0;
int current_idx = 0;
int phase = 1; 

int get_days_in_month(int month) {
    if (month == 4 || month == 6 || month == 9 || month == 11) return 30; 
    if (month == 2) return 28; 
    return (month >= 1 && month <= 12) ? 31 : 30;
}

static void set_count_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    total_apps = atoi(gtk_entry_get_text(GTK_ENTRY(app_data->count_entry)));
    
    if (total_apps <= 0 || total_apps > MAX_APP) {
        gtk_label_set_text(GTK_LABEL(app_data->status_label), "Enter valid count (1-50)");
        return;
    }
    
    current_idx = 0;
    phase = 1;
    gtk_widget_set_sensitive(app_data->setup_grid, TRUE);
    gtk_widget_set_sensitive(app_data->count_entry, FALSE);
    gtk_widget_set_sensitive(widget, FALSE);
    gtk_label_set_text(GTK_LABEL(app_data->status_label), "Phase 1: Enter Name & Watts for App 1");
}

static void log_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;

    if (phase == 1) { 
        const char *n = gtk_entry_get_text(GTK_ENTRY(app_data->name_entry));
        float w = atof(gtk_entry_get_text(GTK_ENTRY(app_data->power_entry)));
        
        if (strlen(n) == 0 || w <= 0) return;

        strcpy(app_list[current_idx].name, n);
        app_list[current_idx].watts = w;
        
        current_idx++;
        if (current_idx < total_apps) {
            char buf[100];
            snprintf(buf, sizeof(buf), "Phase 1: Enter Name & Watts for App %d", current_idx + 1);
            gtk_label_set_text(GTK_LABEL(app_data->status_label), buf);
            gtk_entry_set_text(GTK_ENTRY(app_data->name_entry), "");
            gtk_entry_set_text(GTK_ENTRY(app_data->power_entry), "");
        } else {
            phase = 2;
            current_idx = 0;
            gtk_widget_set_sensitive(app_data->setup_grid, FALSE);
            gtk_widget_set_sensitive(app_data->usage_grid, TRUE);
            char buf[100];
            snprintf(buf, sizeof(buf), "Phase 2: Enter Duration for %s", app_list[0].name);
            gtk_label_set_text(GTK_LABEL(app_data->status_label), buf);
        }
    } else if (phase == 2) { 
        float d = atof(gtk_entry_get_text(GTK_ENTRY(app_data->duration_entry)));
        if (d < 0) return;

        app_list[current_idx].duration = d;
        float u = (app_list[current_idx].watts / 1000.0) * d;
        float c = u * COST_PER_KWH;

        GtkTreeIter iter;
        gtk_list_store_append(app_data->list_store, &iter);
        gtk_list_store_set(app_data->list_store, &iter, 0, app_list[current_idx].name, 1, u, 2, c, -1);

        current_idx++;
        if (current_idx < total_apps) {
            char buf[100];
            snprintf(buf, sizeof(buf), "Phase 2: Enter Duration for %s", app_list[current_idx].name);
            gtk_label_set_text(GTK_LABEL(app_data->status_label), buf);
            gtk_entry_set_text(GTK_ENTRY(app_data->duration_entry), "");
        } else {
            gtk_label_set_text(GTK_LABEL(app_data->status_label), "Data collected! Generate Bill.");
            gtk_widget_set_sensitive(app_data->usage_grid, FALSE);
        }
    }
}

static void view_bill_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    int month = atoi(gtk_entry_get_text(GTK_ENTRY(app_data->month_entry)));
    if (month < 1 || month > 12) {
        gtk_label_set_text(GTK_LABEL(app_data->result_label), "Error: Invalid Month (1-12)");
        return;
    }

    float total_u = 0;
    for(int i=0; i<total_apps; i++) {
        total_u += (app_list[i].watts / 1000.0) * app_list[i].duration;
    }

    int days = get_days_in_month(month);
    float free_u_day = MONTHLY_FREE_UNITS / (float)days;
    float daily_subsidy_val = free_u_day * COST_PER_KWH;

    char res[2500];
    const char *tips = "\n\n💡 SUSTAINABLE TIPS:\n"
                       "1. Switch to LED bulbs (save up to 75% energy).\n"
                       "2. Unplug 'Vampire' devices when not in use.\n"
                       "3. Clean AC filters monthly to improve efficiency.";

    snprintf(res, sizeof(res), 
             "--- DAILY SUBSIDY INFO (Month %02d) ---\n"
             "Free Units for Today: %.2f kWh\n"
             "Daily Subsidy Value: ₹%.2f\n\n"
             "--- TODAY'S CONSUMPTION ---\n"
             "Total Usage: %.2f kWh\n", 
             month, free_u_day, daily_subsidy_val, total_u);

    if (total_u <= free_u_day) {
        char buffer[500];
        snprintf(buffer, sizeof(buffer), 
                 "Wow, consumption is within the daily limit.\n"
                 "Congratulations, you are exempt from charges!\n"
                 "KEEP IT UP!!\n"
                 "Net Payable Today: ₹0.00%s", 
                 tips);
        strcat(res, buffer);
    } else {
        float excess = total_u - free_u_day;
        float net = excess * COST_PER_KWH;
        char buffer[500];
        snprintf(buffer, sizeof(buffer), 
                 "Oops!, you have surpassed daily limit by %.2f units.\n"
                 "Net Payable Today: ₹%.2f%s", 
                 excess, net, tips);
        strcat(res, buffer);
    }
    gtk_label_set_text(GTK_LABEL(app_data->result_label), res);
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window, *vbox, *grid_count, *btn_set, *btn_log_1, *btn_log_2, *btn_bill, *scroll, *tree;
    AppData *app_data = g_new(AppData, 1);

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Smart Electricity Manager");
    gtk_container_set_border_width(GTK_CONTAINER(window), 15);
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 750);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    grid_count = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid_count), 10);
    app_data->count_entry = gtk_entry_new();
    btn_set = gtk_button_new_with_label("Start Setup");
    gtk_grid_attach(GTK_GRID(grid_count), gtk_label_new("Appliances:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid_count), app_data->count_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid_count), btn_set, 2, 0, 1, 1);
    gtk_box_pack_start(GTK_BOX(vbox), grid_count, FALSE, FALSE, 0);

    app_data->setup_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(app_data->setup_grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(app_data->setup_grid), 10);
    app_data->name_entry = gtk_entry_new();
    app_data->power_entry = gtk_entry_new();
    btn_log_1 = gtk_button_new_with_label("Log Specs");
    gtk_grid_attach(GTK_GRID(app_data->setup_grid), gtk_label_new("Name (acronym):"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(app_data->setup_grid), app_data->name_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(app_data->setup_grid), gtk_label_new("Watts:"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(app_data->setup_grid), app_data->power_entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(app_data->setup_grid), btn_log_1, 0, 2, 2, 1);
    gtk_widget_set_sensitive(app_data->setup_grid, FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), app_data->setup_grid, FALSE, FALSE, 0);

    app_data->usage_grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(app_data->usage_grid), 10);
    app_data->duration_entry = gtk_entry_new();
    btn_log_2 = gtk_button_new_with_label("Log Hours");
    gtk_grid_attach(GTK_GRID(app_data->usage_grid), gtk_label_new("Duration:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(app_data->usage_grid), app_data->duration_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(app_data->usage_grid), btn_log_2, 2, 0, 1, 1);
    gtk_widget_set_sensitive(app_data->usage_grid, FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), app_data->usage_grid, FALSE, FALSE, 0);

    app_data->status_label = gtk_label_new("Waiting for setup...");
    gtk_box_pack_start(GTK_BOX(vbox), app_data->status_label, FALSE, FALSE, 0);

    app_data->list_store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_FLOAT, G_TYPE_FLOAT);
    tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(app_data->list_store));
    GtkCellRenderer *render = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree), -1, "Appliance", render, "text", 0, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree), -1, "kWh", render, "text", 1, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree), -1, "Cost (₹)", render, "text", 2, NULL);
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroll), 120);
    gtk_container_add(GTK_CONTAINER(scroll), tree);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    app_data->month_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app_data->month_entry), "Month (1-12)");
    btn_bill = gtk_button_new_with_label("Generate Final Bill");
    gtk_box_pack_start(GTK_BOX(vbox), app_data->month_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), btn_bill, FALSE, FALSE, 0);
    app_data->result_label = gtk_label_new("");
    gtk_label_set_line_wrap(GTK_LABEL(app_data->result_label), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), app_data->result_label, FALSE, FALSE, 0);

    g_signal_connect(btn_set, "clicked", G_CALLBACK(set_count_clicked), app_data);
    g_signal_connect(btn_log_1, "clicked", G_CALLBACK(log_clicked), app_data);
    g_signal_connect(btn_log_2, "clicked", G_CALLBACK(log_clicked), app_data);
    g_signal_connect(btn_bill, "clicked", G_CALLBACK(view_bill_clicked), app_data);

    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("com.smart.ele2", 0);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}

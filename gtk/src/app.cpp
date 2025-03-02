#include <stdio.h>
#include <gtk/gtk.h>

#include "../include/app.h"

// some shit with including css
static void load_css(void) {
    GtkCssProvider *provider;
    GdkDisplay *display;

    provider = gtk_css_provider_new();

    gtk_css_provider_load_from_path(provider, "style.css");

    display = gdk_display_get_default();

    gtk_style_context_add_provider_for_display(
        display,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

static void runWorkingWindow(GtkButton* main_button, gpointer* user_data){
    GtkApplication* app = GTK_APPLICATION(user_data);

    GtkWindow* current_window = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(main_button)));
    gtk_window_close(current_window);

    GtkWidget* working_window = gtk_application_window_new(GTK_APPLICATION(app));

    gtk_window_set_title        (GTK_WINDOW (working_window), "Crack World");
    gtk_window_set_default_size (GTK_WINDOW (working_window), DEFAULT_WIDTH, DEFAULT_HEIGHT);
    gtk_window_set_resizable    (GTK_WINDOW(working_window), FALSE);

    // do some magic css stuff
    load_css();

    gtk_window_present          (GTK_WINDOW(working_window));
}

static void runMainWindow(GtkApplication* app, gpointer* user_data){
    // create main window
    GtkWidget* main_window = gtk_application_window_new(GTK_APPLICATION(app));

    gtk_window_set_title        (GTK_WINDOW (main_window), "Crack World");
    gtk_window_set_default_size (GTK_WINDOW (main_window), DEFAULT_WIDTH, DEFAULT_HEIGHT);
    gtk_window_set_resizable    (GTK_WINDOW(main_window), FALSE);

    // do some magic css stuff
    load_css();

    // set overlay for background and box
    GtkWidget* main_overlay = gtk_overlay_new();
    gtk_window_set_child(GTK_WINDOW (main_window), main_overlay);

    // create picture for background
    GtkWidget* main_background = gtk_picture_new_for_filename("img/main_background.jpg");

    // set background to overlay as main child
    gtk_overlay_set_child       (GTK_OVERLAY(main_overlay), main_background);
    gtk_picture_set_content_fit (GTK_PICTURE(main_background), GTK_CONTENT_FIT_COVER);

    // create box for main label and button
    GtkWidget* main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(main_box, 20);
    gtk_widget_set_halign(main_box, GTK_ALIGN_START);
    gtk_widget_set_valign(main_box, GTK_ALIGN_CENTER);
    gtk_overlay_add_overlay(GTK_OVERLAY(main_overlay), main_box);

    // create main label
    GtkWidget* main_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(main_label), "<span color=\"white\" weight=\"bold\" font=\"Verdana 70\">Crack World</span>");
    gtk_box_append(GTK_BOX(main_box), main_label);

    // crate main button
    GtkWidget* main_button = gtk_button_new_with_label("Start your journey");
    gtk_widget_set_name(main_button, "main_button");
    gtk_box_append(GTK_BOX(main_box), main_button);
    g_signal_connect(main_button, "clicked", G_CALLBACK(runWorkingWindow), app);

    gtk_window_present          (GTK_WINDOW(main_window));
}

int main(int argc, char* argv[]){
    GtkApplication* app = gtk_application_new("COM.CRACK", G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app, "activate", G_CALLBACK(runMainWindow), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);

    return status;
}

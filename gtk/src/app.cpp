#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "../include/app.h"

typedef struct {
    GtkPicture* picture;
    GdkPixbufAnimation* animation;
    GdkPixbufAnimationIter* iter;
    guint timeout_id;
} AnimationData;

static gboolean update_animation(gpointer user_data) {
    AnimationData* data = (AnimationData*)user_data;

    gdk_pixbuf_animation_iter_advance(data->iter, NULL);

    GdkPixbuf* frame = gdk_pixbuf_animation_iter_get_pixbuf(data->iter);
    GdkPaintable* paintable = GDK_PAINTABLE(gdk_texture_new_for_pixbuf(frame));

    gtk_picture_set_paintable(data->picture, paintable);

    int delay = gdk_pixbuf_animation_iter_get_delay_time(data->iter);

    g_source_remove(data->timeout_id);
    data->timeout_id = g_timeout_add(delay, update_animation, data);

    return G_SOURCE_REMOVE;
}

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

    // create working box
    GtkWidget* working_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_name(working_box, "working_box");
    gtk_window_set_child(GTK_WINDOW(working_window), working_box);

    // create gif
    GtkWidget* picture = gtk_picture_new();
    gtk_widget_set_hexpand(picture, TRUE);

    // set gif and animation
    GError* error = NULL;
    GdkPixbufAnimation* animation = gdk_pixbuf_animation_new_from_file("img/gif.gif", &error);

    GdkPixbufAnimationIter* iter = gdk_pixbuf_animation_get_iter(animation, NULL);

    AnimationData* data = g_new0(AnimationData, 1);
    data->picture = GTK_PICTURE(picture);
    data->animation = animation;
    data->iter = iter;

    data->timeout_id = g_timeout_add(100, update_animation, data);

    gtk_widget_set_halign(picture, GTK_ALIGN_END);
    gtk_widget_set_valign(picture, GTK_ALIGN_FILL);

    // create overlay for left side of working window
    GtkWidget* left_side_overlay = gtk_overlay_new();
    gtk_widget_set_name(left_side_overlay, "left_side_overlay");
    gtk_box_append(GTK_BOX(working_box), left_side_overlay);

    // set gif as box child
    gtk_box_append(GTK_BOX(working_box), picture);

    // create explanatory label
    GtkWidget* explanatory_label = gtk_label_new(NULL);
    gtk_widget_set_name(explanatory_label, "explanatory_label");
    gtk_label_set_markup(GTK_LABEL(explanatory_label), "Welcome! To start the job write down the file, you "
                                                       "want to crack.");
    gtk_label_set_wrap(GTK_LABEL(explanatory_label), TRUE);
    // gtk_widget_set_halign(explanatory_label, GTK_ALIGN_START);
    // gtk_widget_set_valign(explanatory_label, GTK_ALIGN_CENTER);

    gtk_overlay_set_child(GTK_OVERLAY(left_side_overlay), explanatory_label);

    // create entry
    GtkWidget* file_path_entry= gtk_entry_new();
    gtk_widget_set_name(file_path_entry, "file_path_entry");
    gtk_entry_set_placeholder_text(GTK_ENTRY(file_path_entry), "Input path to file you want to crack");

    gtk_overlay_add_overlay(GTK_OVERLAY(left_side_overlay), file_path_entry);

    gtk_window_present(GTK_WINDOW(working_window));
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

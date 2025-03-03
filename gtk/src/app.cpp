#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gst/gst.h>

#include "../include/app.h"
#include "../include/patch.h"

// Main window
static void runMainWindow(GtkApplication* app, gpointer* user_data);

static GtkWidget* createMainWindow(GtkApplication* app);
static GtkWidget* createMainOverlay(GtkWidget* main_window);
static GtkWidget* createMainBackground(GtkWidget* main_overlay);
static GtkWidget* createMainBox(GtkWidget* main_overlay);
static GtkWidget* createMainLabel(GtkWidget* main_box);
static GtkWidget* createMainButton(GtkWidget* main_box);

static void closeMainWindow(GtkButton* main_button);

// Working window
static void runWorkingWindow(GtkButton* main_button, gpointer* user_data);

static GtkWidget* createWorkingWindow(GtkApplication* app);
static GtkWidget* createWorkingBox(GtkWidget* working_window);
static GtkWidget* createLeftSideOverlay(GtkWidget* working_box);
static GtkWidget* createExplanatoryLabel(GtkWidget* left_side_overlay);
static GtkWidget* createFilePathEntry(GtkWidget* left_side_overlay);
static GtkWidget* createCrackButton(GtkWidget* left_side_overlay);

static GstElement* createAudioPipelineAndPlay();

static GtkWidget* createGif();
static gboolean updateAnimation(gpointer user_data);

// CSS
static void loadCSS();

// Crack file
static void crackFile(GtkButton* crack_button, gpointer* user_data);
static gboolean updateLabel(gpointer user_data);

// Close request
static gboolean closeRequest(GtkWindow *window, gpointer* user_data);

// Struct for gif animation
typedef struct AnimationData{
    GtkPicture* picture;
    GdkPixbufAnimation* animation;
    GdkPixbufAnimationIter* iter;
    guint timeout_id;
} AnimationData;

int main(int argc, char* argv[]){
    // initialize gstream
    gst_init(&argc, &argv);

    // create application
    GtkApplication* app = gtk_application_new(APPLICATION_NAME, G_APPLICATION_DEFAULT_FLAGS);

    // create activate signal to call runMainWindow when opened
    g_signal_connect(app, "activate", G_CALLBACK(runMainWindow), NULL);

    // send activate signal to run application
    int status = g_application_run(G_APPLICATION(app), argc, argv);

    // clear app
    g_object_unref(app);

    // clear gstream
    gst_deinit();

    return status;
}

static void runMainWindow(GtkApplication* app, gpointer* user_data){
    loadCSS();

    GtkWidget* main_window      = createMainWindow(app);

    GtkWidget* main_overlay     = createMainOverlay(main_window);

    GtkWidget* main_background  = createMainBackground(main_overlay);
    GtkWidget* main_box         = createMainBox(main_overlay);

    GtkWidget* main_label       = createMainLabel(main_box);
    GtkWidget* main_button      = createMainButton(main_box);

    // create clicked signal to call runWorkingWindow when main button is clicked
    g_signal_connect(main_button, "clicked", G_CALLBACK(runWorkingWindow), app);

    // show main window
    gtk_window_present(GTK_WINDOW(main_window));
}

static void runWorkingWindow(GtkButton* main_button, gpointer* user_data){
    loadCSS();

    // set user_data to GtkApplication*
    GtkApplication* app             = GTK_APPLICATION(user_data);

    closeMainWindow(main_button);

    GtkWidget* working_window       = createWorkingWindow(app);

    GstElement* pipeline            = createAudioPipelineAndPlay();

    GtkWidget* working_box          = createWorkingBox(working_window);

    GtkWidget* working_picture      = createGif();

    GtkWidget* left_side_overlay    = createLeftSideOverlay(working_box);

    // set gif as box child (after left_side_overlay for gif to be on the right side)
    gtk_box_append(GTK_BOX(working_box), working_picture);

    GtkWidget* explanatory_label    =  createExplanatoryLabel(left_side_overlay);
    GtkWidget* file_path_entry      = createFilePathEntry(left_side_overlay);

    GtkWidget* crack_button         =  createCrackButton(left_side_overlay);

    // create clicked signal to call crackFile when crack button is clicked
    g_signal_connect(crack_button, "clicked", G_CALLBACK(crackFile), left_side_overlay);

    // create close-request signal to call closeRequest when working window is closed
    g_signal_connect(working_window, "close-request", G_CALLBACK (closeRequest), pipeline);

    // show working window
    gtk_window_present(GTK_WINDOW(working_window));
}

static void loadCSS(){
    // creating css provider
    GtkCssProvider* provider = gtk_css_provider_new();

    // parse style.css
    gtk_css_provider_load_from_path(provider, CSS_PATH);

    // get default display
    GdkDisplay* display = gdk_display_get_default();

    // add global style provider (aka style.css) to display
    gtk_style_context_add_provider_for_display(display, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
}

static gboolean closeRequest(GtkWindow *window, gpointer* user_data){
    // set user data to GstElement*
    GstElement* pipeline = GST_ELEMENT(user_data);

    // stop playing audio
    gst_element_set_state(pipeline, GST_STATE_NULL);

    // clear pipeline
    gst_object_unref(pipeline);

    return FALSE;
}

static void crackFile(GtkButton* crack_button, gpointer* user_data){
    // set user_data to GtkOverlay*
    GtkOverlay* left_side_overlay = GTK_OVERLAY(user_data);

    // get file_path_entry as left_side_overlay 2nd child
    GtkWidget* explanatory_label = gtk_overlay_get_child(left_side_overlay);
    GtkWidget* file_path_entry = gtk_widget_get_next_sibling(explanatory_label);

    // get text in file_path_entry
    GtkEntryBuffer* file_path_buffer = gtk_entry_get_buffer(GTK_ENTRY(file_path_entry));
    const char* file_path = gtk_entry_buffer_get_text(file_path_buffer);

    // check if file exists
    FILE* file_to_crack = fopen(file_path, "r+b");
    if (!file_to_crack){
        gtk_label_set_label(GTK_LABEL(explanatory_label), FILE_NOT_FOUND_MSG);

        fclose(file_to_crack);

        return;
    }

    // crack file
    binaryPatch(file_to_crack, file_path);

    // set WORK_IN_PROGRESS_MSG message in explanatory_label
    gtk_label_set_label(GTK_LABEL(explanatory_label), WORK_IN_PROGRESS_MSG);

    // call updateLabel after WORK_TIME_DELAY s
    g_timeout_add_seconds(WORK_TIME_DELAY, updateLabel, explanatory_label);
}

//////////////////////////////////////
//////////////////////////////////////
///// MAIN WINDOW STATICS BELOW //////
//////////////////////////////////////
//////////////////////////////////////

static GtkWidget* createMainWindow(GtkApplication* app){
    // create window bonded to app
    GtkWidget* main_window = gtk_application_window_new(app);

    // set parametres for main window
    gtk_window_set_title        (GTK_WINDOW (main_window), WINDOW_TITLE);
    gtk_window_set_default_size (GTK_WINDOW (main_window), DEFAULT_WIDTH, DEFAULT_HEIGHT);
    gtk_window_set_resizable    (GTK_WINDOW(main_window), FALSE);

    return main_window;
}

static GtkWidget* createMainOverlay(GtkWidget* main_window){
    // create main overlay
    GtkWidget* main_overlay = gtk_overlay_new();

    // set main_overlay as main_window child
    gtk_window_set_child(GTK_WINDOW(main_window), main_overlay);

    return main_overlay;
}

static GtkWidget* createMainBackground(GtkWidget* main_overlay){
    // create main background
    GtkWidget* main_background = gtk_picture_new_for_filename(MAIN_BACKGROUND_PATH);

    // set main_background as main_overlay child (first child)
    gtk_overlay_set_child(GTK_OVERLAY(main_overlay), main_background);

    // set parametres
    gtk_picture_set_content_fit(GTK_PICTURE(main_background), GTK_CONTENT_FIT_COVER);

    return main_background;
}

static GtkWidget* createMainBox(GtkWidget* main_overlay){
    // create main box for main label and main button
    GtkWidget* main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, MAIN_BOX_ELEMENT_SPACING);

    // set parametres
    gtk_widget_set_margin_start (main_box, MAIN_BOX_MARGIN_LEFT_SIDE);
    gtk_widget_set_halign       (main_box, GTK_ALIGN_START);
    gtk_widget_set_valign       (main_box, GTK_ALIGN_CENTER);

    // set main_box as main_overlay child (second child)
    gtk_overlay_add_overlay(GTK_OVERLAY(main_overlay), main_box);

    return main_box;
}

static GtkWidget* createMainLabel(GtkWidget* main_box){
    // create main label
    GtkWidget* main_label = gtk_label_new(NULL);

    // set name for style.css
    gtk_widget_set_name(main_label, "main_label");

    // set label text
    gtk_label_set_label(GTK_LABEL(main_label), MAIN_LABEL_TEXT);

    // set main_label as main_box child
    gtk_box_append(GTK_BOX(main_box), main_label);

    return main_label;
}

static GtkWidget* createMainButton(GtkWidget* main_box){
    // create main button
    GtkWidget* main_button = gtk_button_new_with_label(MAIN_BUTTON_TEXT);

    // set name for style.css
    gtk_widget_set_name(main_button, "main_button");

    // set main_button as main_box child
    gtk_box_append(GTK_BOX(main_box), main_button);

    return main_button;
}

////////////////////////////////////////
////////////////////////////////////////
///// WORKING WINDOW STATICS BELOW /////
////////////////////////////////////////
////////////////////////////////////////

static void closeMainWindow(GtkButton* main_button){
    // get main_window pointer as of main_button's parent
    GtkWindow* main_window = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(main_button)));

    gtk_window_close(main_window);
}

static GtkWidget* createWorkingWindow(GtkApplication* app){
    // create working window bonded to app
    GtkWidget* working_window = gtk_application_window_new(app);

    // set parametres
    gtk_window_set_title        (GTK_WINDOW (working_window), WINDOW_TITLE);
    gtk_window_set_default_size (GTK_WINDOW (working_window), DEFAULT_WIDTH, DEFAULT_HEIGHT);
    gtk_window_set_resizable    (GTK_WINDOW(working_window), FALSE);

    return working_window;
}

static GstElement* createAudioPipelineAndPlay(){
    // concatenate strings
    char launch_arg[MAX_CMD_SIZE];
    strcpy(launch_arg, "playbin uri=file://");
    strcat(launch_arg, MUSIC_PATH);

    // create audio pipeline
    GstElement* pipeline = gst_parse_launch(launch_arg, NULL);

    // play audio
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    return pipeline;
}

static GtkWidget* createWorkingBox(GtkWidget* working_window){
    // create working box
    GtkWidget* working_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, WORKING_BOX_ELEMENT_SPACING);

    // set name for style.css
    gtk_widget_set_name(working_box, "working_box");

    // set working_box as working_window child
    gtk_window_set_child(GTK_WINDOW(working_window), working_box);

    return working_box;
}

static GtkWidget* createGif(){
    // create picture (container for time-changing puctures)
    GtkWidget* working_picture = gtk_picture_new();

    // create animation by loading it from file
    GError* error = NULL;
    GdkPixbufAnimation* animation = gdk_pixbuf_animation_new_from_file(GIF_PATH, &error);

    // get an iterator for displaying an animation
    GdkPixbufAnimationIter* iter = gdk_pixbuf_animation_get_iter(animation, NULL);

    // allocate AnimationData object
    AnimationData* data = g_new0(AnimationData, 1);

    // set fields
    data->picture   = GTK_PICTURE(working_picture);
    data->animation = animation;
    data->iter      = iter;

    // call updateAnimation once in UPDATE_ANIMATION_DELAY ms
    g_timeout_add(UPDATE_ANIMATION_DELAY, updateAnimation, data);

    // set parametres
    gtk_widget_set_halign(working_picture, GTK_ALIGN_END);
    gtk_widget_set_valign(working_picture, GTK_ALIGN_FILL);

    return working_picture;
}

static gboolean updateAnimation(gpointer user_data) {
    // set user_data to AnimationData*
    AnimationData* data = (AnimationData*)user_data;

    // advances an animation to a new frame
    gdk_pixbuf_animation_iter_advance(data->iter, NULL);

    // get frame from data
    GdkPixbuf* frame = gdk_pixbuf_animation_iter_get_pixbuf(data->iter);

    // create paintable with frame
    GdkPaintable* paintable = GDK_PAINTABLE(gdk_texture_new_for_pixbuf(frame));

    // set paintable in picture
    gtk_picture_set_paintable(data->picture, paintable);

    return TRUE;
}

static GtkWidget* createLeftSideOverlay(GtkWidget* working_box){
    // create left side overlay
    GtkWidget* left_side_overlay = gtk_overlay_new();

    // set name for style.css
    gtk_widget_set_name(left_side_overlay, "left_side_overlay");

    // set left_side_overlay as working_box child
    gtk_box_append(GTK_BOX(working_box), left_side_overlay);

    return left_side_overlay;
}

static GtkWidget* createExplanatoryLabel(GtkWidget* left_side_overlay){
    // create explanatory label
    GtkWidget* explanatory_label = gtk_label_new(NULL);

    // set name for style.css
    gtk_widget_set_name(explanatory_label, "explanatory_label");

    // set label text
    gtk_label_set_label(GTK_LABEL(explanatory_label), "Welcome! To start the job write down the file, you want to crack.");

    // enable line break
    gtk_label_set_wrap(GTK_LABEL(explanatory_label), TRUE);

    // set explanatory_label as left_side_overlay child (first child)
    gtk_overlay_set_child(GTK_OVERLAY(left_side_overlay), explanatory_label);

    return explanatory_label;
}

static GtkWidget* createFilePathEntry(GtkWidget* left_side_overlay){
    // create file path entry
    GtkWidget* file_path_entry = gtk_entry_new();

    // set name for style.css
    gtk_widget_set_name(file_path_entry, "file_path_entry");

    // set placeholder text
    gtk_entry_set_placeholder_text(GTK_ENTRY(file_path_entry), "Input path to file you want to crack");

    // set file_path_entry as left_side_overlay child (second child)
    gtk_overlay_add_overlay(GTK_OVERLAY(left_side_overlay), file_path_entry);

    return file_path_entry;
}

static GtkWidget* createCrackButton(GtkWidget* left_side_overlay){
    // create crack button
    GtkWidget* crack_button = gtk_button_new_with_label("Crack this!");

    // set name for style.css
    gtk_widget_set_name(crack_button, "crack_button");

    // set crack_button as left_side_overlay child (third child)
    gtk_overlay_add_overlay(GTK_OVERLAY(left_side_overlay), crack_button);

    return crack_button;
}

static gboolean updateLabel(gpointer user_data){
    // set user_data to GtkWidget*
    GtkWidget* label = (GtkWidget*)user_data;

    // set label's text
    gtk_label_set_label(GTK_LABEL(label), "SUCCESS! Enjoy your superiority");

    return FALSE;
}

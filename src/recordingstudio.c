
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gdk/gdk.h>
#include "recordingstudio.h"
#include "annotation_window.h"
#include "utils.h"
#include "background_window.h"
#include "bar.h"
#include "bar_callbacks.h"
#include "cairo_functions.h"
#include "recorder.h"

G_MODULE_EXPORT void
on_record_click            (GtkToggleButton   *toolbutton,
                                   gpointer         func_data)
{
    g_print("on_recording_click\n");
    gboolean grab_value = bar_data->grab;

    /* Release grab. */
    annotate_release_grab ();


    bar_data->grab = FALSE;

    if (is_started ())
      {
        if (is_paused ())
          {
            resume_recorder ();
            /* Set the stop tool-tip. */
        //    gtk_tool_button_set_tooltip_text ( toolbutton, gettext ("Stop"));
            /* Put the stop icon. */
            GtkWidget* imageWidget = GTK_WIDGET( gtk_builder_get_object( annotation_data->recordingstudio_window_gtk_builder, "media-playback-stop" ) );
            gtk_button_set_image( (GtkButton*) toolbutton, imageWidget );
            gtk_button_set_label( (GtkButton*) toolbutton, "Stop" );
            //gtk_tool_button_set_icon_name (toolbutton, "media-playback-stop");
          }
        else
          {
            pause_recorder ();
            /* Set the stop tool-tip. */
//gtk_tool_button_set_tooltip_text ( toolbutton,
    //                                        gettext ("Record"));

            /* Put the record icon. */
            GtkWidget* imageWidget = GTK_WIDGET( gtk_builder_get_object( annotation_data->recordingstudio_window_gtk_builder, "media-record" ) );
            gtk_button_set_image( (GtkButton*) toolbutton, imageWidget );
            gtk_button_set_label( (GtkButton*) toolbutton, "Record" );

            //gtk_tool_button_set_icon_name (toolbutton, "media-record");
            replace_status_message(gettext("Screen recorder stopped"));
          }
      }
    else
      {

        if (!is_recorder_available ())
          {
              GtkWidget* imageWidget = GTK_WIDGET( gtk_builder_get_object( annotation_data->recordingstudio_window_gtk_builder, "media-recorder-unavailable" ) );
              gtk_button_set_image( (GtkButton*) toolbutton, imageWidget );
              gtk_button_set_label( (GtkButton*) toolbutton, "Unavailable" );

            /* Visualize a dialog that informs the user about the missing recorder tool. */
            // GObject *recorder_obj = gtk_builder_get_object (bar_gtk_builder,
            //                                                 "media-recorder-unavailable");

            gdk_window_set_cursor (gtk_widget_get_window (get_annotation_window ()),
                                   (GdkCursor *) NULL);

            visualize_missing_recorder_program_dialog (GTK_WINDOW (get_bar_widget ()),
                  gettext ("In order to record with Ardesia you must install the vlc program and add it to the PATH environment variable")
              );
            /* Put an icon that remember that the tool is not available. */
            // gtk_tool_button_set_icon_widget (toolbutton, GTK_WIDGET (recorder_obj));
            bar_data->grab = grab_value;
            start_tool (bar_data);
            return;
          }

        gdk_window_set_cursor (gtk_widget_get_window (get_annotation_window ()),
                               (GdkCursor *) NULL);

        replace_status_message(gettext("Starting screen recorder"));
        /* The recording is not active. */
        gboolean status = start_save_video_dialog ( (GtkButton*) toolbutton, GTK_WINDOW (get_bar_widget ()));
        if (status)
          {
            /* Set the stop tool-tip. */
        //    gtk_tool_button_set_tooltip_text (  toolbutton, gettext ("Stop"));
            /* Put the stop icon. */
            //gtk_tool_button_set_icon_name(toolbutton, gettext("media-playback-stop") );
            GtkWidget* imageWidget = GTK_WIDGET( gtk_builder_get_object( annotation_data->recordingstudio_window_gtk_builder, "media-playback-stop" ) );
            gtk_button_set_image( (GtkButton*) toolbutton, imageWidget );
            gtk_button_set_label( (GtkButton*) toolbutton, "Stop" );
          }
      }
    bar_data->grab = grab_value;
    start_tool (bar_data);
}

// Region CreateRegion(int x, int y, int w, int h) {
//     Region region = XCreateRegion();
//     XRectangle rectangle;
//     rectangle.x = x;
//     rectangle.y = y;
//     rectangle.width = w;
//     rectangle.height = h;
//     XUnionRectWithRegion(&rectangle, region, region);
//
//     return region;
// }
void drill_window_in_cursor_area();

void
get_desktop_mouse_location(int* x, int* y) {
    GdkScreen* screen = gdk_screen_get_default();
    GdkWindow* desktop = gdk_screen_get_root_window( screen );
    GdkDisplay* display = gdk_display_get_default();
    GdkSeat* seat = gdk_display_get_default_seat(display);
    GdkDevice* device = gdk_seat_get_pointer( seat );
    gdk_window_get_device_position( desktop, device, x, y, NULL );
}

gboolean
move_cursor_window(gpointer data) {
    if ( annotation_data->is_cursor_visible == FALSE ) {
        return FALSE; // stop timer
    } else {
        //g_printf("Moving cursor");
        gint x, y;
        GdkScreen* screen = gdk_screen_get_default();
        GdkWindow* desktop = gdk_screen_get_root_window( screen );
        GdkDisplay* display = gdk_display_get_default();
        GdkSeat* seat = gdk_display_get_default_seat(display);
        GdkDevice* device = gdk_seat_get_pointer( seat );


        gdk_window_get_device_position( desktop, device, &x, &y, NULL );
        gtk_window_move( GTK_WINDOW(annotation_data->cursor_window) , x-32, y-32 );
        //gtk_window_move( GTK_WINDOW(annotation_data->cursor_window) , 100, 100 );
          gtk_widget_input_shape_combine_region (annotation_data->cursor_window, NULL);
          drill_window_in_cursor_area ();
        return TRUE; // continue timer
    }
}

void drill_window_in_cursor_area() {
    GtkWidget *cursor_window= annotation_data->cursor_window;
    gint x, y, width, height;

    gtk_window_get_position (GTK_WINDOW (cursor_window), &x, &y);
    gtk_window_get_size (GTK_WINDOW (cursor_window), &width, &height);

    // the rectangle we are cutting out is the same shape
    GdkRectangle* rA = g_new( GdkRectangle, 1 );
    rA->x = x;
    rA->y = y;
    rA->width = width;
    rA->height = height;

      const cairo_rectangle_int_t widget_rect = { x+1, y+1, width-1, height-1 };
      cairo_region_t *widget_reg = cairo_region_create_rectangle (&widget_rect);

      // drill with input shape the pointer will go below the window.
      gtk_widget_input_shape_combine_region ( cursor_window, widget_reg);

      // drill with shape; the area will be transparent.
    //  gtk_widget_shape_combine_region (cursor_window, widget_reg);

      cairo_region_destroy (widget_reg);
    g_free( rA );
}

void draw_video_cursor(cairo_t* cr, GtkWidget* widget);

/**
 * Called by Gtk on draw event for cursor window
 * @param  widget    [description]
 * @param  cr        [description]
 * @param  user_data [description]
 * @return           [description]
 */
static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
  draw_video_cursor(cr, widget);
  return FALSE;
}

static void setup_transparency(GtkWidget *win)
{
  GdkScreen *screen;
  GdkVisual *visual;

  gtk_widget_set_app_paintable(win, TRUE);
  screen = gdk_screen_get_default();
  visual = gdk_screen_get_rgba_visual(screen);

  if (visual != NULL && gdk_screen_is_composited(screen)) {
      gtk_widget_set_visual(win, visual);
  }
}


/**
 * Manual creation of cursor window of 32x32 px size
 * @return [description]
 */
GtkWidget* create_cursor_window() {
    g_printf("Creating cursor\n");
    gint size = 64;
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE ); // remove titlebar, resize controls etc
    gtk_window_set_deletable(GTK_WINDOW(window), FALSE); // remove close box
    gtk_window_set_skip_taskbar_hint( GTK_WINDOW(window), TRUE ); // remove from taskbar
    gtk_window_set_skip_pager_hint( GTK_WINDOW(window), TRUE ); // remove from pager
    gtk_window_set_default_size(GTK_WINDOW(window), size, size); // sets initial size
    gtk_widget_set_size_request(window, size, size); // sets minimum size
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE ); // cannot be resized by user

    GtkWidget* drawing_area = gtk_drawing_area_new();
    gtk_container_add( GTK_CONTAINER(window), drawing_area );
    g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(on_draw_event), NULL);
    gtk_widget_set_events (drawing_area, gtk_widget_get_events (drawing_area)
                                     | GDK_BUTTON_PRESS_MASK
                                     | GDK_POINTER_MOTION_MASK);
    setup_transparency(window);
    return window;
}

/**
 * Draw the cursor as per the current step
 * @param cr     [description]
 * @param widget [description]
 */
void draw_video_cursor(cairo_t* cr, GtkWidget* widget) {
    if ( annotation_data->is_cursor_visible == FALSE ) {
        return;
    }

    // take a screen grab around where the mouse is
    // check to see if the pixel is nearer to white than black
    // change color accordingly
    int x, y;
    get_desktop_mouse_location( &x, &y );
    GdkWindow *root_win = gdk_get_default_root_window();
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 32, 32);
    GdkPixbuf *pb = gdk_pixbuf_get_from_window(root_win, x-16, y-16, 32, 32);
    cairo_t *desktop = cairo_create(surface);
    gdk_cairo_set_source_pixbuf(desktop, pb, 0, 0);
    cairo_paint(desktop);

    // average over all pixels
    unsigned char* pixels = cairo_image_surface_get_data( surface );
    int stride = cairo_image_surface_get_stride(surface); // gives back 128 = 32 pixels * 4 bytes each
    gint avg_pixel=0;
    for( int ii=0; ii < 32; ii++ ) {
        for( int jj =0; jj < stride; jj++ ) {
            if ( jj % 4 < 3 ) { // ignore alpha channel
                avg_pixel += pixels[(ii*32)+jj];
            }
        }
    }
    avg_pixel /= (stride * 32);
    gint white = 255;
    gint r=0,g=0,b=0;
    if ( white - avg_pixel > avg_pixel ) { // nearer to black
        r = 1;
        g = 1;
        b = 0;
    } else { // nearer to white
        r = 0;
        g = 1;
        b = 0;
    }

    gint width = gtk_widget_get_allocated_width( widget );
    gint height = gtk_widget_get_allocated_width( widget );
    // cursor step increments to provide user with an animation
    cairo_set_source_rgba(cr, 0, 0, 0, 0);
    cairo_paint(cr);

    cairo_set_line_width(cr, 1 );
    cairo_set_source_rgb(cr, r, g, b);
    cairo_translate(cr, width/2, height/2 );
    cairo_arc(cr, 0, 0, 5, 0, 2 * M_PI );
    cairo_stroke_preserve(cr);
    cairo_fill(cr);

    // now we draw lines depending on the step
    switch( (gint) (annotation_data->cursor_step / 10) ) {
    case 0:
        break;
    case 1:
    case 5:
        // 1 line
        cairo_set_line_width( cr, 2 );
        cairo_set_source_rgb(cr, r, g, b );
        cairo_arc(cr, 0, 0, 10, 0, 2 * M_PI );
        cairo_stroke(cr);
        break;
    case 2:
    case 4:
        cairo_set_line_width( cr, 2 );
        cairo_set_source_rgb(cr, r, g, b );
        cairo_arc(cr, 0, 0, 10, 0, 2 * M_PI );
        cairo_stroke(cr);

        cairo_set_line_width( cr, 2 );
        cairo_set_source_rgb(cr, r, g, b );
        cairo_arc(cr, 0, 0, 15, 0, 2 * M_PI );
        cairo_stroke(cr);
        break;
    case 3:
        cairo_set_line_width( cr, 2 );
        cairo_set_source_rgb(cr, r, g, b );
        cairo_arc(cr, 0, 0, 10, 0, 2 * M_PI );
        cairo_stroke(cr);

        cairo_set_line_width( cr, 2 );
        cairo_set_source_rgb(cr, r, g, b );
        cairo_arc(cr, 0, 0, 15, 0, 2 * M_PI );
        cairo_stroke(cr);

        cairo_set_line_width( cr, 2 );
        cairo_set_source_rgb(cr, r, g, b );
        cairo_arc(cr, 0, 0, 20, 0, 2 * M_PI );
        cairo_stroke(cr);
        break;
    }
    annotation_data->cursor_step++;
    annotation_data->cursor_step = annotation_data->cursor_step % 60;

    gtk_widget_queue_draw( widget );
}


G_MODULE_EXPORT void
on_cursor_click            (GtkToggleButton   *toolbutton,
                                   gpointer         func_data)
{
    g_print("on_cursor_click\n");
    // vlc --screen-mouse-pointer does not work on linux so
    // instead what we want to do is show an image just under where
    // the mouse pointer is going to be
    if ( annotation_data->is_cursor_visible == TRUE) {
        // hide cursor window
        //g_printf("Hiding cursor window\n");
        annotation_data->is_cursor_visible = FALSE;
        gtk_widget_hide(annotation_data->cursor_window);
        gtk_window_set_keep_above(GTK_WINDOW(annotation_data->cursor_window), FALSE);
        annotation_data->cursor_timer=0;
    } else {
        if ( annotation_data->cursor_window_gtk_builder == NULL ) {
            // build cursor window
            g_printf("Building cursor window\n");
            annotation_data->cursor_window = create_cursor_window();
            gtk_widget_input_shape_combine_region (annotation_data->cursor_window, NULL);
            drill_window_in_cursor_area ();
        }

        //g_printf("Showing cursor window\n");
        annotation_data->is_cursor_visible = TRUE;
        gtk_window_present(GTK_WINDOW(annotation_data->cursor_window));
        gtk_widget_show_all(annotation_data->cursor_window);
        // needed this hide and show in here to make window appear again
        // after the initial hide - very weird!
        gtk_widget_hide(annotation_data->cursor_window);
        gtk_widget_show_all(annotation_data->cursor_window);
        gtk_window_set_keep_above(GTK_WINDOW(annotation_data->cursor_window), TRUE);
        move_cursor_window(NULL);
        annotation_data->cursor_timer =  g_timeout_add (100, move_cursor_window, NULL);
    }
}

G_MODULE_EXPORT void
on_clapperboard_click            (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
    g_print("on_clapperboard_click");
    gboolean grab_value = bar_data->grab;
    bar_data->grab = FALSE;
    annotate_release_grab ();
    if ( annotation_data->clapperboard_cairo_context == NULL ) {
        int width = gtk_widget_get_allocated_width(annotation_data->annotation_window);
        int height = gtk_widget_get_allocated_height(annotation_data->annotation_window);
        annotation_data->clapperboard_cairo_context = create_new_context(width, height);
        load_color_onto_context(BLACK, annotation_data->clapperboard_cairo_context);

    }

    annotation_data->is_clapperboard_visible = TRUE;

    // make the screen black and then go back to what it was before
    bar_data->grab = grab_value;
    start_tool (bar_data);
    gtk_widget_queue_draw(annotation_data->annotation_window);
    begin_clapperboard_countdown();
}

G_MODULE_EXPORT void
on_new_click              (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
    g_print("on_new_click");

    // restart video
    stop_recorder();
    GtkWidget* beginRecordingButton = GTK_WIDGET( gtk_builder_get_object( annotation_data->recordingstudio_window_gtk_builder, "record" ) );
    on_record_click( (GtkToggleButton*) beginRecordingButton, func_data );
}

G_MODULE_EXPORT void
on_recordingstudio_window_destroy_event (GtkWidget *widget, GdkEvent *event, gpointer data) {

    g_print("recording studio window being destroyed\n");
}

G_MODULE_EXPORT gboolean
on_recordingstudio_window_delete_event (GtkWidget *widget, GdkEvent *event, gpointer data) {
    gtk_widget_hide(widget);
    return TRUE;
}

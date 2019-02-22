/*
 * Ardesia -- a program for painting on the screen
 * with this program you can play, draw, learn and teach
 * This program has been written such as a freedom sonet
 * We believe in the freedom and in the freedom of education
 *
 * Copyright (C) 2009 Pilolli Pietro <pilolli.pietro@gmail.com>
 *
 * Ardesia is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Ardesia is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gdk/gdk.h>
#include <utils.h>
#include <bar_callbacks.h>
#include <bar.h>
#include <annotation_window.h>
#include <background_window.h>
#include <text_window.h>
#include <color_selector.h>
#include <preference_dialog.h>
#include <info_dialog.h>
#include <iwb_saver.h>
#include <recorder.h>
#include <saver.h>
#include <pdf_saver.h>
#include <share_confirmation_dialog.h>
#include <cairo_functions.h>


/* Windows state event: this occurs when the windows state changes. */
G_MODULE_EXPORT gboolean
on_bar_window_state_event         (GtkWidget            *widget,
                                   GdkEventWindowState  *event,
                                   gpointer              func_data)
{
    g_print("DEBUG: bat state event\n");
  BarData *bar_data = (BarData *) func_data;

  /* Track the minimized signals */
  if(gdk_window_get_state (gtk_widget_get_window (widget)) & GDK_WINDOW_STATE_ICONIFIED)
    {
      release_lock (bar_data);
    }

  return TRUE;
}

G_MODULE_EXPORT gboolean
on_bar_draw_event          (GtkWidget *widget,
                    cairo_t   *cr,
                    gpointer   user_data) {
    g_print("bar draw event\n");
return FALSE;
}

void
on_bar_hide_event (GtkWidget *widget,
               gpointer   user_data) {
    g_print("bar hide event\n");

    if ( bar_data->screenshot_pending == TRUE ) {

    }
}


/* Configure events occurs.
 * Called at very start and selected defaults
 */
G_MODULE_EXPORT gboolean
on_bar_configure_event            (GtkWidget  *widget,
                                   GdkEvent   *event,
                                   gpointer   func_data)
{
    g_print("bar configure event (%d)\n", bar_data->screenshot_pending);

  if ( bar_data->screenshot_pending == TRUE ) {
      // we tried 2 methods:
      //  1. hide causing a hide event or a configure event
      //  2. moving window off screen first and then triggering the hide event
      // both appear to have occurred, but image still captures the window - perhaps
      // a double buffering artifact somewhere.
      //
      // we have to give it some time for the animation to play out
      // otherwise the window will still be visible in our snapshot
      // 2 seconds appears to work, 1 second sometimes works.
      sleep(2);
      g_printf("found a screenshot pending\n");

      GdkPixbuf* buffer = take_screenshot_now();
      bar_data->screenshot_pending = FALSE;
      gdk_window_move( gtk_widget_get_window(get_bar_widget()),
                      bar_data->screenshot_saved_location_x,
                      bar_data->screenshot_saved_location_y);
      bar_data->screenshot_saved_location_x =-1;
      bar_data->screenshot_saved_location_y = -1;
      gtk_widget_show( get_bar_widget() );
      bar_data->screenshot_callback( buffer );
      bar_data->screenshot_callback = NULL;
  } else {
      set_options (bar_data);
  }
  return TRUE;
}


/* Called when push the quit button or the window close action*/
G_MODULE_EXPORT gboolean
on_bar_quit                     (GtkToolButton   *toolbutton,
                                 gpointer         func_data)
{
    g_printf("on_bar_quit\n");
  BarData *bar_data = (BarData *) func_data;

  stop_recorder ();

  bar_data->grab = FALSE;
  /* Release grab. */
  annotate_release_grab ();

  export_iwb (get_iwb_filename ());
  quit_pdf_saver ();
  //start_share_dialog ();

  // handles removal of background and text data structures
  annotate_quit ();

  /* Quit the gtk engine. */
  gtk_main_quit ();
  return FALSE;
}


/* Called when push the info button. */
G_MODULE_EXPORT gboolean
on_bar_info                      (GtkToolButton   *toolbutton,
                                  gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  gboolean grab_value = bar_data->grab;
  bar_data->grab = FALSE;

  /* Release grab. */
  annotate_release_grab ();

  /* Start the info dialog. */
  start_info_dialog (toolbutton, GTK_WINDOW (get_bar_widget ()));

  bar_data->grab = grab_value;
  start_tool (bar_data);
  return TRUE;
}


/* Called when leave the window. */
G_MODULE_EXPORT gboolean
on_bar_leave_notify_event         (GtkWidget       *widget,
                                   GdkEvent        *event,
                                   gpointer func_data)
{
  BarData *bar_data = (BarData *) func_data;
  add_alpha (bar_data);
  start_tool (bar_data);
  return TRUE;
}


/* Called when enter the window. */
G_MODULE_EXPORT gboolean
on_bar_enter_notify_event         (GtkWidget       *widget,
                                   GdkEvent        *event,
                                   gpointer         func_data)
{
    g_printf("bar enter notify event\n");
  if (is_text_toggle_tool_button_active ())
    {
      stop_text_widget ();
    }

    
  return TRUE;
}


/* Push pointer button. */
G_MODULE_EXPORT void
on_bar_pointer_activate           (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  release_lock (bar_data);
}



/* Push text button. */
G_MODULE_EXPORT void
on_bar_text_activate              (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  lock (bar_data);
  replace_status_message(gettext("Text tool selected"));
}


/* Push mode button. */
G_MODULE_EXPORT void
on_bar_mode_activate              (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  take_pen_tool ();
  if (!bar_data->rectifier)
    {
      if (!bar_data->rounder)
        {
          /* Select the rounder mode. */
          GObject *rounder_obj = gtk_builder_get_object (bar_gtk_builder, "rounder");
          gtk_tool_button_set_icon_widget (toolbutton, GTK_WIDGET (rounder_obj));
          bar_data->rounder = TRUE;
          bar_data->rectifier = FALSE;
          replace_status_message(gettext("Rounder mode selected"));
        }
      else
        {
          /* Select the rectifier mode. */
          GObject *rectifier_obj = gtk_builder_get_object (bar_gtk_builder, "rectifier");
          gtk_tool_button_set_icon_widget (toolbutton, GTK_WIDGET (rectifier_obj));
          bar_data->rectifier = TRUE;
          bar_data->rounder = FALSE;
          replace_status_message(gettext("Polygon mode selected"));
        }
    }
  else
    {
      /* Select the free hand writing mode. */
      GObject *hand_obj = gtk_builder_get_object (bar_gtk_builder, "hand");
      gtk_tool_button_set_icon_widget (toolbutton, GTK_WIDGET (hand_obj));
      bar_data->rectifier = FALSE;
      bar_data->rounder = FALSE;
      replace_status_message(gettext("Freehand mode selected"));
    }
}


/* Push thickness button. */
G_MODULE_EXPORT void
on_bar_thick_activate             (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;

  if (bar_data->thickness == MICRO_THICKNESS)
    {
    replace_status_message(gettext("Brush thickness set to thin"));
      /* Set the thin icon. */
      GObject *thin_obj = gtk_builder_get_object (bar_gtk_builder, "thin");
      gtk_tool_button_set_icon_widget (toolbutton, GTK_WIDGET (thin_obj));
      bar_data->thickness = THIN_THICKNESS;
    }
  else if (bar_data->thickness == THIN_THICKNESS)
    {
        replace_status_message(gettext("Brush thickness set to medium"));
      /* Set the medium icon. */
      GObject *medium_obj = gtk_builder_get_object (bar_gtk_builder, "medium");
      gtk_tool_button_set_icon_widget (toolbutton, GTK_WIDGET (medium_obj));
      bar_data->thickness = MEDIUM_THICKNESS;
    }
  else if (bar_data->thickness==MEDIUM_THICKNESS)
    {
    replace_status_message(gettext("Brush thickness set to thick"));
      /* Set the thick icon. */
      GObject *thick_obj = gtk_builder_get_object (bar_gtk_builder, "thick");
      gtk_tool_button_set_icon_widget (toolbutton, GTK_WIDGET (thick_obj));
      bar_data->thickness = THICK_THICKNESS;
    }
  else if (bar_data->thickness==THICK_THICKNESS)
    {
        replace_status_message(gettext("Brush thickness set to micro"));
      /* Set the micro icon. */
      GObject *micro_obj = gtk_builder_get_object (bar_gtk_builder, "micro");
      gtk_tool_button_set_icon_widget (toolbutton, GTK_WIDGET (micro_obj));
      bar_data->thickness = MICRO_THICKNESS;
    }

}


/* Push arrow button. */
G_MODULE_EXPORT void
on_bar_arrow_activate             (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{

  BarData *bar_data = (BarData *) func_data;
  lock (bar_data);
  set_color (bar_data, bar_data->color);
  replace_status_message(gettext("Arrow tool selected"));
}


/* Push pencil button. */
G_MODULE_EXPORT void
on_bar_pencil_activate            (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{

  BarData *bar_data = (BarData *) func_data;
  lock (bar_data);
  set_color (bar_data, bar_data->color);
  replace_status_message(gettext("Pencil tool selected"));
}


/* Push highlighter button. */
G_MODULE_EXPORT void
on_bar_highlighter_activate       (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  lock (bar_data);
  set_color (bar_data, bar_data->color);
  replace_status_message(gettext("Highlighter tool selected"));
}


/* Push filler button. */
G_MODULE_EXPORT void
on_bar_filler_activate            (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  lock (bar_data);
  annotate_select_filler ();
  replace_status_message(gettext("Filler tool selected"));
}


/* Push eraser button. */
G_MODULE_EXPORT void
on_bar_eraser_activate            (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  lock (bar_data);
  annotate_select_eraser();
  replace_status_message(gettext("Eraser tool selected"));
}


/* Push save (screen-shoot) button. */
G_MODULE_EXPORT void
on_bar_screenshot_activate	      (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  gboolean grab_value = bar_data->grab;
  bar_data->grab = FALSE;
  /* Release grab. */
  annotate_release_grab ();
  replace_status_message(gettext("Taking screenshot"));
  gdk_window_set_cursor (gtk_widget_get_window (get_annotation_window ()), (GdkCursor *) NULL);
  start_save_image_dialog();
  bar_data->grab = grab_value;
  start_tool (bar_data);
}


/* Add page to pdf. */
G_MODULE_EXPORT void
on_bar_add_pdf_activate	          (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  gboolean grab_value = bar_data->grab;
  bar_data->grab = FALSE;

  /* Release grab. */
  annotate_release_grab ();

replace_status_message(gettext("Exporting as PDF"));
  add_pdf_page (GTK_WINDOW (get_bar_widget ()));
  bar_data->grab = grab_value;
  start_tool (bar_data);
}

/* Hide state event: this occurs when the show/hide widget event happens
 * 2018-08-09 : Added by TM
 */
G_MODULE_EXPORT void
on_bar_showhide_activate     (GtkToolButton            *toolButton,
                                   gpointer              func_data)
{
  BarData *bar_data = (BarData *) func_data;
  gboolean annotation_is_visible = bar_data->annotation_is_visible;

  /* Release grab lock. */
  annotate_release_grab ();
  bar_data->grab = FALSE;

  if ( annotation_is_visible == TRUE ) {
      /** currently annotations are visible so icon is the hidden **/
      GtkWidget* window = get_annotation_window ();
      if ( window != NULL ) {
          replace_status_message(gettext("Annotations hidden"));
          gtk_widget_hide( window ); // @TODO loses its position so we need to save the position and loses image
          bar_data->annotation_is_visible = FALSE;


          /* Set the stop tool-tip. */
          gtk_tool_item_set_tooltip_text ( (GtkToolItem *) toolButton,
                                          gettext ("Show Annotations"));



          /* Put the show icon. */
          GtkImage* icon = get_image_from_builder( gettext("show") );
          gtk_tool_button_set_icon_widget (toolButton, (GtkWidget*) icon);
      }
  } else {
      /** currently annotations are hidden so icon is the showing icon */

      GtkWidget* window = get_annotation_window ();
      if ( window != NULL ) {
          replace_status_message( gettext("Annotations visible") );
          gtk_widget_show( window );
          gtk_window_set_keep_above (GTK_WINDOW (window), TRUE);
          // @TODO move back to position, restore last image

          bar_data->annotation_is_visible = TRUE;

          /* Set the stop tool-tip. */
          gtk_tool_item_set_tooltip_text ( (GtkToolItem *) toolButton,
                                          gettext ("Hide Annotations"));



          /* Put the hide icon. */
          GtkImage* icon = get_image_from_builder( gettext("hide") );
          gtk_tool_button_set_icon_widget (toolButton, (GtkWidget*) icon);

          annotate_acquire_grab ();
      }
  }
}

/* Push recorder button. */
G_MODULE_EXPORT void
on_bar_recorder_activate          (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
    GError* error = (GError *) NULL;
  // we want to show the recording studio window at this point
  if ( annotation_data->recordingstudio_window == NULL ) {
      g_printf("Showing recording menu");
      if ( annotation_data->recordingstudio_options == NULL ) {
          annotation_data->recordingstudio_options = g_malloc( (gsize) sizeof(RecordingStudioData) );
      }

      // create new window  /* Initialize the main window. */
      annotation_data->recordingstudio_window_gtk_builder = gtk_builder_new ();
      gtk_builder_add_from_file (annotation_data->recordingstudio_window_gtk_builder, RECORDINGSTUDIO_UI_FILE, &error);

      if (error)
        {
          g_warning ("Failed to load builder file: %s", error->message);
          g_error_free (error);
          return;
        }

      annotation_data->recordingstudio_window = GTK_WIDGET (gtk_builder_get_object (annotation_data->recordingstudio_window_gtk_builder,
                           "recordingstudio_window") );

       if (annotation_data->recordingstudio_window  == NULL)
         {
           g_warning ("Failed to create the recording studio window");
           return;
         }
         gtk_builder_connect_signals (annotation_data->recordingstudio_window_gtk_builder, (gpointer) annotation_data);


      gtk_widget_show (annotation_data->recordingstudio_window);
  } else {
      // just show the window again
      gtk_widget_show (annotation_data->recordingstudio_window);
  }

}

gboolean
on_remove_background_button( GtkMenuItem *menuitem, gpointer user_data) {
    GtkWidget* widget = GTK_WIDGET( user_data );
    gint width = gtk_widget_get_allocated_width( widget );
    g_printf("button width: %d\n", width);
    gtk_widget_destroy( widget );
    gint cwidth = gtk_widget_get_allocated_width( annotation_data->background_selection_window );
    gint cheight = gtk_widget_get_allocated_height( annotation_data->background_selection_window );
    g_printf("window size: %d %d\n", cwidth, cheight);
    gtk_window_resize( GTK_WINDOW(annotation_data->background_selection_window), cwidth - width, cheight );
    return TRUE;
}

void
background_selection_on_toggled (GtkToggleToolButton *toggle_tool_button, gpointer userdata) {
    BackgroundButtonData* button_data = (BackgroundButtonData*) userdata;
    // left click
    g_printf("left click\n");
    annotation_data->background_button_last_selected = button_data->index;
    BackgroundButtonData* data = (BackgroundButtonData*) (g_slist_nth(annotation_data->background_button_data, button_data->index)->data);
    if ( data->mode == BACKGROUND_MODE_COLOR ) {
       update_background_color( data->color );
    } else if ( data->mode == BACKGROUND_MODE_FILE ) {
       update_background_image( data->filename );
    } else {
       clear_background_context();
       annotation_data->background_button_last_selected = BACKGROUND_NONE_SELECTED;
    }
}

gboolean
background_selection_on_button_press( GtkWidget* widget, GdkEvent *event, gpointer userdata ) {
    GdkEventButton *event_button;
    if (event->type == GDK_BUTTON_PRESS)
    {
        event_button = (GdkEventButton *) event;
        BackgroundButtonData* button_data = (BackgroundButtonData*) userdata;
        if ( button_data->index > 3 ) {
            if (event_button->button == GDK_BUTTON_SECONDARY) {
                g_printf("background_window_on_context_menu\n");
                // create a popup for delete
                GtkWidget* menu, *menuitem;
                menu = gtk_menu_new();
                menuitem = gtk_menu_item_new_with_label("Remove");
                gtk_menu_attach( GTK_MENU(menu), menuitem, 0, 1, 0, 1 );

                g_signal_connect( menuitem, "activate", (GCallback) on_remove_background_button, widget );

                gtk_widget_show_all(menu);
                gtk_menu_popup_at_pointer( GTK_MENU(menu), NULL );
                return TRUE;
            }
        }
    }
    return FALSE;
}

gboolean
on_add_new_background( GtkWidget* widget, gpointer user_data ) {
    start_preference_dialog( GTK_WINDOW(user_data) );
    return TRUE;
}

/**
 * Effects will not show until after another gtk_widget_show_all call
 * @param data [description]
 * @param size [description]
 */
void resize_image_to_button( BackgroundButtonData* data, gint size ) {
    gint x=0, y=0, w=size, h=size;
    cairo_t * cr = NULL;
    cairo_surface_t* surface = NULL;
    gchar* output = NULL;
    GError* error = NULL;
    GtkImage* image = GTK_IMAGE( gtk_tool_button_get_icon_widget( GTK_TOOL_BUTTON(data->button) ) );

    surface = cairo_image_surface_create( CAIRO_FORMAT_ARGB32, w, h );
    cr = cairo_create( surface );

    if ( data->filename != NULL ) {
        load_file_onto_context( data->filename, cr );
    } else if ( data->color != NULL ) {
        load_color_onto_context( data->color, cr );
    }

    GdkPixbuf* pixbuf = gdk_pixbuf_get_from_surface( surface, x, y, w, h );
    output = g_strdup_printf("test_output%d.png", data->index );
    gdk_pixbuf_save( pixbuf, output, "png", &error, NULL );
    if ( error != NULL ) {
        g_printf("error: %s\n", error->message);
    }

    if ( image == NULL ) {
        g_printf("create new image\n");
        image = GTK_IMAGE(gtk_image_new_from_pixbuf(pixbuf));
    } else {
        g_printf("setting image\n");
        gtk_image_set_from_pixbuf( image, pixbuf );
    }
    data->size = size;

    gtk_tool_button_set_icon_widget( GTK_TOOL_BUTTON( data->button ), GTK_WIDGET(image) );

    cairo_surface_destroy( surface );
    cairo_destroy( cr );
}


gboolean
on_background_selection_window_configure_event(GtkWidget *widget, GdkEvent  *event, gpointer user_data) {
    if ( widget == annotation_data->background_selection_window && event->type == GDK_CONFIGURE ) {
        gint elements = g_slist_length( annotation_data->background_button_data );
        gint h = gtk_widget_get_allocated_height( GTK_WIDGET(annotation_data->background_selection_window) );
        gint m = (int) log2( (double) h );
        m = (int) pow( 2.0 , m );

        for( gint ii=0; ii < elements; ii++ ) {
            BackgroundButtonData* data = (BackgroundButtonData*) ( g_slist_nth( annotation_data->background_button_data, ii )->data );
            GtkImage* image = GTK_IMAGE( gtk_tool_button_get_icon_widget( GTK_TOOL_BUTTON(data->button) ) );
            if ( image == NULL || data->size != m ) {
                g_printf("redrawing image (%d)\n", m);
                resize_image_to_button( data, m );
            }
        }
        // without this the button icons remain blank and the window does not
        // resize to fit added buttons
        gtk_widget_show_all( annotation_data->background_selection_window );
    }
    return FALSE;
}

// void
// background_selection_on_button_size_allocate(GtkWidget* widget, GdkRectangle *allocation, gpointer user_data) {
//     if ( annotation_data->background_button_data != NULL ) {
//         BackgroundButtonData* data = (BackgroundButtonData*) ( g_slist_nth( annotation_data->background_button_data, 0 )->data );
//         gint expected_width = ( g_slist_length( annotation_data->background_button_data ) + 1 ) * data->size;
//         g_printf("on_button size allocate %d %d\n", allocation->width, allocation->height);
//         gint window_width = gtk_widget_get_allocated_width( annotation_data->background_selection_window );
//         //data->size > allocation->height - 11 &&
//         if ( expected_width > window_width ) {
//             // resize at lower size
//             gint m = (int) log2( (double) allocation->height );
//             m = (int) pow( 2.0 , m-1 );
//             gint elements = g_slist_length( annotation_data->background_button_data );
//             for( gint ii=0; ii < elements; ii++ ) {
//                 //if ( data->button == widget ) {
//                     data = (BackgroundButtonData*) ( g_slist_nth( annotation_data->background_button_data, ii )->data );
//                     resize_image_to_button( data, m );
//                 //}
//             }
//             gtk_widget_set_size_request( widget, m, -1 );
//             //gtk_widget_show_all( annotation_data->background_selection_window );
//         }
//     }
// }

void
add_background_button(gchar* label, gint mode, gchar* filename, gchar* color) {
    GtkToolItem* button = NULL;

    if ( annotation_data->background_button_data == NULL ) {
        button = gtk_radio_tool_button_new(NULL);
    } else {
        BackgroundButtonData* data = (BackgroundButtonData*) (g_slist_last(annotation_data->background_button_data)->data);
        button = gtk_radio_tool_button_new_from_widget( GTK_RADIO_TOOL_BUTTON(data->button) );
    }
    if ( label == NULL ) {
        label = "No Label";
    }
    if ( g_slist_length(annotation_data->background_button_data) == 0 ) {
        gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON(button), TRUE );
    } else {
        gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON(button), FALSE );
    }
    if ( label != NULL ) {
        gtk_tool_item_set_tooltip_text( GTK_TOOL_ITEM(button), label);
    }
    gtk_box_pack_start( GTK_BOX(annotation_data->background_selection_container), GTK_WIDGET(button), TRUE, TRUE, 0 );

    BackgroundButtonData* data = g_new(BackgroundButtonData, 1);
    data->mode = mode;
    data->filename = filename;
    data->color = color;
    data->button = button;
    data->index = g_slist_length( annotation_data->background_button_data );
    data->size = 32;
    annotation_data->background_button_data = g_slist_append( annotation_data->background_button_data, data );

    // used added button
    g_signal_connect( button, "toggled", (GCallback) background_selection_on_toggled, data );
    g_signal_connect( button, "button_press_event", (GCallback) background_selection_on_button_press, data );
    // g_signal_connect( button, "size-allocate", (GCallback) background_selection_on_button_size_allocate, data );

    if ( g_slist_length(annotation_data->background_button_data) > 3 ) {
        gtk_widget_show_all( annotation_data->background_selection_window );
    }
}

void
on_background_selection_window_destroy (GtkWidget *object, gpointer   user_data) {
    annotation_data->background_selection_window = NULL;
    annotation_data->background_selection_container = NULL;
    g_slist_free( annotation_data->background_button_data );
    annotation_data->background_button_data = NULL;
    annotation_data->background_button_last_selected = BACKGROUND_NONE_SELECTED;
}

void
on_background_selection_size_allocate(GtkWidget* widget, GdkRectangle *allocation, gpointer user_data) {
//    g_printf("size allocate %d %d\n", allocation->width, allocation->height);
}

void create_background_window(/* arguments */) {
    GtkWidget* window = NULL;
    GtkToolItem* button = NULL;
    GtkBox* box = NULL;

    window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_title( GTK_WINDOW(window), "Backgrounds");
    gtk_window_set_default_size( GTK_WINDOW(window), 400, 128 );

    box = GTK_BOX(gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 ));

    //actionbar = GTK_ACTION_BAR( gtk_action_bar_new() );
    //gtk_box_pack_start( box, GTK_WIDGET(actionbar), TRUE, TRUE, 0 );
    gtk_container_add( GTK_CONTAINER(window), GTK_WIDGET(box) );

    annotation_data->background_selection_window = window;
    annotation_data->background_selection_container = GTK_WIDGET(box);

    add_background_button( "Transparent", BACKGROUND_MODE_NONE, TRANSPARENT_BACKGROUND_FILE, NULL );
    add_background_button( "Blackboard", BACKGROUND_MODE_COLOR, NULL, BLACK );
    add_background_button( "Whiteboard", BACKGROUND_MODE_COLOR, NULL, WHITE );
    add_background_button( "Paper", BACKGROUND_MODE_FILE, PAPER_BACKGROUND_FILE, NULL );

    button = gtk_tool_button_new(NULL, "Add");
    gtk_box_pack_start( box, GTK_WIDGET(button), TRUE, TRUE, 0 );

    g_signal_connect( button, "clicked", (GCallback) on_add_new_background, window );
    g_signal_connect( window, "destroy", (GCallback) on_background_selection_window_destroy, NULL );
    g_signal_connect( window, "configure-event", (GCallback) on_background_selection_window_configure_event, NULL );
    g_signal_connect( window, "size-allocate", (GCallback) on_background_selection_size_allocate, NULL );
    gtk_widget_show_all( window );
}

/* Push fonts button. */
G_MODULE_EXPORT void
on_bar_fonts_clicked(GtkToolButton   *toolbutton, gpointer         func_data)
{
    create_text_settings_window();
}

/* Push preference button. */
G_MODULE_EXPORT void
on_bar_preferences_activate	      (GtkToolButton   *toolbutton,
                                       gpointer         func_data)
{
    BarData *bar_data = (BarData *) func_data;
    gboolean grab_value = bar_data->grab;
    bar_data->grab = FALSE;
    /* Release grab. */
    annotate_release_grab ();

    gdk_window_set_cursor (gtk_widget_get_window (get_annotation_window ()), (GdkCursor *) NULL);

    if ( annotation_data->background_button_last_selected == BACKGROUND_NONE_SELECTED ) {
        if ( annotation_data->background_selection_window != NULL ) {
            gtk_widget_show_all( annotation_data->background_selection_window );
        } else {
            create_background_window();
        }
    } else {
        if ( annotation_data->background_selection_window == NULL ) {
            clear_background_context();
            bar_data->grab = grab_value;
            start_tool (bar_data);
            annotation_data->background_button_last_selected = BACKGROUND_NONE_SELECTED;
        } else {
            clear_background_context();
            bar_data->grab = grab_value;
            start_tool (bar_data);
            annotation_data->background_button_last_selected = BACKGROUND_NONE_SELECTED;
            // // need to create hole in the background layer for this window
            // gtk_widget_input_shape_combine_region (annotation_data->annotation_window, NULL);
            // drill_window_in_bar_area( annotation_data->annotation_window, get_bar_widget() );
            // drill_window_in_bar_area( annotation_data->annotation_window, annotation_data->background_selection_window );
            // //gtk_window_present( GTK_WINDOW(annotation_data->background_selection_window) );

        }
    }
}


/* Push undo button. */
G_MODULE_EXPORT void
on_bar_undo_activate              (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  annotate_undo ();
}


/* Push redo button. */
G_MODULE_EXPORT void
on_bar_redo_activate              (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  annotate_redo ();
}


/* Push clear button. */
G_MODULE_EXPORT void
on_bar_clear_activate             (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
    replace_status_message(gettext("Screen has been cleared"));
  annotate_clear_screen ();
}


/* Push colour selector button. */
G_MODULE_EXPORT void
on_bar_color_activate	            (GtkToggleToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  gboolean grab_value = bar_data->grab;
  gchar* new_color = "";

  if (!gtk_toggle_tool_button_get_active (toolbutton))
    {
      return;
    }

  /* Release grab. */
  annotate_release_grab ();

  bar_data->grab = FALSE;
  gdk_window_set_cursor (gtk_widget_get_window (get_annotation_window ()), (GdkCursor *) NULL);
  new_color = start_color_selector_dialog (GTK_TOOL_BUTTON (toolbutton),
                                           GTK_WINDOW (get_bar_widget ()),
                                           bar_data->color);

  if (new_color)  // if it is a valid colour
    {
      set_color (bar_data, new_color);
      g_free (new_color);
    }

  bar_data->grab = grab_value;
  start_tool (bar_data);
}


/* Push blue colour button. */
G_MODULE_EXPORT void
on_bar_blue_activate              (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  set_color (bar_data, BLUE);
}


/* Push red colour button. */
G_MODULE_EXPORT void
on_bar_red_activate               (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  set_color (bar_data, RED);
}


/* Push green colour button. */
G_MODULE_EXPORT void
on_bar_green_activate             (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  set_color (bar_data, GREEN);
}


/* Push yellow colour button. */
G_MODULE_EXPORT void
on_bar_yellow_activate            (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  set_color (bar_data, YELLOW);
}


/* Push white colour button. */
G_MODULE_EXPORT void
on_bar_white_activate             (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  set_color (bar_data, WHITE);
}

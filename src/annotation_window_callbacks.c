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


/*
 * Functions for handling various (GTK+)-Events.
 */


#include <annotation_window_callbacks.h>
#include <annotation_window.h>
#include <utils.h>
#include <input.h>
#include <background_window.h>
#include <text_window.h>
#include <text_window_callbacks.h>
#include <cairo_functions.h>




/* On configure event. */
G_MODULE_EXPORT gboolean
on_configure       (GtkWidget      *widget,
                    GdkEventExpose *event,
                    gpointer        user_data)
{
  AnnotateData *data = (AnnotateData *) user_data;
  GdkWindowState state = gdk_window_get_state (gtk_widget_get_window (widget));

  g_printf("DEBUG: Annotation window get configure event (%d,%d,%d,%d)\n",
             gtk_widget_get_allocated_width (widget),
             gtk_widget_get_allocated_height (widget),
             state,
             gtk_widget_is_focus (widget)
             );
//
//   cairo_region_t* existing_surface = gdk_window_get_visible_region( gtk_widget_get_window(data->annotation_window) );
//   if ( existing_surface == NULL ) {
//       g_printf("on_configure: no existing surface\n");
//   } else {
//       g_printf("on_configure: existing surface\n");
//   }
//
//   if (data->debug)
//     {
//       g_printerr("DEBUG: Annotation window get configure event (%d,%d)\n",
//                  gtk_widget_get_allocated_width (widget),
//                  gtk_widget_get_allocated_height (widget));
//     }
//
//
//
//   //gint is_fullscreen = state & GDK_WINDOW_STATE_FULLSCREEN;
//
//   // if (!is_fullscreen)
//   //   {
//   //     return FALSE;
//   //   }
//
 // build_background_window();
 // gtk_window_set_keep_above (GTK_WINDOW (background_window), FALSE);
 // gtk_window_set_keep_above (GTK_WINDOW (data->annotation_window), TRUE);
 // initialize_annotation_cairo_context (data);

// gtk_widget_queue_resize( GTK_WINDOW (data->annotation_window) );
  if (!data->is_grabbed)
    {
      return FALSE;
    }

    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);
    if ( data->is_background_visible == TRUE ) {
        annotation_window_change(width, height);
        gtk_widget_set_opacity (data->annotation_window, 1.0);
    } else {
        //gtk_widget_set_opacity (data->annotation_window, 0.0);
    }
    if ( data->is_annotation_visible == TRUE ) {
        annotation_window_change(width, height);
    }
    if ( data->is_text_editor_visible == TRUE ) {
        annotation_window_change(width, height);
    }


//
//   /* Postcondition; data->annotation_cairo_context is not NULL. */
  return TRUE;
//


 // *** just these two lines plus full screen will show the square
 // initialize_annotation_cairo_context (data);
 // annotate_restore_surface ();


 // bring in cairo_context code
//  if (data->annotation_cairo_context == NULL)
//    {
//        g_printf("initializing annotation cairo context\n");
//          /* Initialize a transparent window. */
//          data->annotation_cairo_context = gdk_cairo_create (gtk_widget_get_window (data->annotation_window));
//
//      if (cairo_status (data->annotation_cairo_context) != CAIRO_STATUS_SUCCESS)
//        {
//          g_printerr ("Failed to allocate the annotation cairo context");
//          annotate_quit ();
//          exit (EXIT_FAILURE);
//        }
//        cairo_set_operator (data->annotation_cairo_context, CAIRO_OPERATOR_OVER);
//
//        if (data->savepoint_list == NULL)
//        {
//          // THIS MUST BE RUN
//          // adds a blank save point
//          annotate_clear_screen ();
//        }
//        // this must be included
//        gtk_window_set_opacity (GTK_WINDOW (data->annotation_window), 1.0);
//
//        guint i = data->current_save_index;
//
//        AnnotateSavepoint *savepoint = (AnnotateSavepoint *) g_slist_nth_data (data->savepoint_list, i);
//
//        if (!savepoint)
//          {
//              g_printf("savepoint is FALSE\n");
//              draw_test_square(data->annotation_cairo_context );
//            return;
//          }
//
//
//         // cairo_new_path (data->annotation_cairo_context);
//          cairo_set_operator (data->annotation_cairo_context, CAIRO_OPERATOR_OVER);
// draw_test_square(data->annotation_cairo_context );
// draw_test_square_with_color( data->annotation_cairo_context, 0, 0, 255 );
//        if (savepoint->filename)
//          {
//              g_printf("load savepoint from filename %s\n", savepoint->filename);
//            /* Load the file in the annotation surface. */
//            cairo_surface_t *image_surface = cairo_image_surface_create_from_png (savepoint->filename);
//            if (data->debug)
//              {
//                g_printerr ("The save-point %s has been loaded from file\n", savepoint->filename);
//              }
//
//            if (image_surface)
//              {
//                  g_printf("writing surface from vellum\n");
//                cairo_save( data->annotation_cairo_context);
//                cairo_new_path(data->annotation_cairo_context);
//                cairo_set_operator (data->annotation_cairo_context, CAIRO_OPERATOR_SOURCE);
//                cairo_set_source_surface (data->annotation_cairo_context, image_surface, 0, 0);
//                cairo_paint (data->annotation_cairo_context);
//                cairo_stroke (data->annotation_cairo_context);
//                cairo_surface_flush( cairo_get_target(data->annotation_cairo_context) );
//                cairo_surface_destroy (image_surface);
//
//                draw_test_square_with_color( data->annotation_cairo_context, 0, 255, 0 );
//                cairo_restore( data->annotation_cairo_context);
//
//              }
//
//          }
//          int width = gtk_widget_get_allocated_width(data->annotation_window);
//          int height = gtk_widget_get_allocated_width(data->annotation_window);
//
//          //gtk_widget_queue_draw_area (data->annotation_window, 0, 0, width, height );
//          // try and redraw
//          //draw_test_square(data->annotation_cairo_context );
//      }
//
//
//
//  return FALSE;
}

G_MODULE_EXPORT gboolean
on_keypress (GtkWidget *widget,
             GdkEvent  *event,
             gpointer   user_data) {
    AnnotateData* data = (AnnotateData*) user_data;
    GdkEventKey* ev = (GdkEventKey*) event;
    gboolean retval = FALSE;

     g_printf("DEBUG: Annotation on_keypress event (%d, %d)\n",
              ev->type,
              ev->keyval
           );

    if ( data->is_text_editor_visible == TRUE ) {
        retval = on_text_window_key_press_event(widget, event, text_data);

    }

    gtk_widget_queue_draw( widget );
    return retval;
}

G_MODULE_EXPORT gboolean
on_keyrelease (GtkWidget *widget,
            GdkEvent  *event,
            gpointer   user_data) {

                GdkEventKey* ev = (GdkEventKey*) event;
                 g_printf("DEBUG: Annotation on_keyrelease event (%d, %d)\n",
                          ev->type,
                          ev->keyval
                       );
                return FALSE;
}

/** When the window changes z-order or state */
G_MODULE_EXPORT gboolean
on_window_state_event (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   user_data){
    // AnnotateData *data = (AnnotateData *) user_data;
    // g_printf("DEBUG: Annotation window state event (%d)\n",
    //          event->type
    //       );
    // if (data->debug)
    // {
    //     g_printerr("DEBUG: Annotation window state event (%d)\n",
    //              event->type
    //           );
    // }
    //
    // if ( event->type == GDK_WINDOW_STATE ) {
    //     GdkEventWindowState* state = (GdkEventWindowState*) event;
    //     g_printf("New window State: %d %d\n", state->changed_mask, state->new_window_state);
    //
    //     // the state of the window has changed
    //     //g_printerr ("on_window_state_event: Moving ardesia_bar_window to top\n");
    //     //gtk_window_set_keep_above (GTK_WINDOW (ardesia_bar_window), TRUE);
    //     //gtk_widget_show (ardesia_bar_window);
    // }
    //
    // // TRUE will stop the propagation to other windows
    // return FALSE; // propagate the event further

    return FALSE;
}

/* On screen changed.
 * Required to make window transparent
 **/
G_MODULE_EXPORT void
on_screen_changed       (GtkWidget  *widget,
                         GdkScreen  *previous_screen,
                         gpointer    user_data)
{
    AnnotateData *data = (AnnotateData *) user_data;
  if (data->debug)
    {
      g_printerr ("DEBUG: Annotation window get screen-changed event\n");
    }

  GdkScreen *screen = gtk_widget_get_screen (GTK_WIDGET (widget));
  GdkVisual *visual = gdk_screen_get_rgba_visual (screen);

  if (visual == NULL)
    {
      visual = gdk_screen_get_system_visual (screen);
    }

  gtk_widget_set_visual (widget, visual);
}




/* Expose event: this occurs when the window is shown.
 */
G_MODULE_EXPORT gboolean
on_expose          (GtkWidget *widget,
                    cairo_t   *cr,
                    gpointer   user_data)
{
    AnnotateData *annotation_data = (AnnotateData *) user_data;

 g_printerr ("DEBUG: Annotation window get draw event\n");

    clear_cairo_context( cr ); // blank the current window for repainting

    if ( annotation_data->is_background_visible == TRUE ) {
        // draw background layer on context cr
        if ( background_data->cr ) {
            draw_cairo_context( cr, background_data->cr );
        }
    }

    if ( annotation_data->is_annotation_visible == TRUE ) {
        // draw annotation layer on context cr
        initialize_annotation_cairo_context (annotation_data);
        draw_cairo_context( cr, annotation_data->annotation_cairo_context );
    }

    if ( annotation_data->is_text_editor_visible == TRUE ) {
        if ( text_data->cr ) {
            // draw the text editor layer
            draw_cairo_context( cr, text_data->cr );
        }
    }

    // draw clapperboard on top of everything else
    if ( annotation_data->is_clapperboard_visible == TRUE ) {
        if ( annotation_data->clapperboard_cairo_context ) {
            draw_cairo_context( cr, annotation_data->clapperboard_cairo_context );
        }
    }

  return TRUE;
}


/*
 * Event-Handlers to perform the drawing.
 */


/* This is called when the button is pushed. */
G_MODULE_EXPORT gboolean
on_button_press    (GtkWidget      *win,
                    GdkEventButton *ev,
                    gpointer        user_data)
{

  AnnotateData *data = (AnnotateData *) user_data;
  gboolean retval = FALSE;

  if ( data->is_annotation_visible == TRUE &&
        data->is_text_editor_visible == FALSE ) {
      retval = annotation_window_button_press( ev, data );
  }

  return retval;
}


/* This shots when the pointer is moving. */
G_MODULE_EXPORT gboolean
on_motion_notify   (GtkWidget       *win,
                    GdkEventMotion  *ev,
                    gpointer         user_data)
{

  AnnotateData *data = (AnnotateData *) user_data;
  gboolean retval = FALSE;
  if ( data->is_annotation_visible == TRUE &&
        data->is_text_editor_visible == FALSE ) {
      retval = annotation_window_mouse_move( ev, data );
  }
  return retval;
}


/* This shots when the button is released. */
G_MODULE_EXPORT gboolean
on_button_release  (GtkWidget       *win,
                    GdkEventButton  *ev,
                    gpointer         user_data)
{
    g_printf("annotation_window::on_button_release\n");
  AnnotateData *data = (AnnotateData *) user_data;
  gboolean retval = FALSE;
  if ( data->is_text_editor_visible == TRUE ) {
      retval = on_text_window_button_release( win, ev, text_data );
  } else if ( data->is_annotation_visible == TRUE ) {
      retval = annotation_window_button_release( ev, data );
  }
  return retval;
}


/* On device added. */
void on_device_removed  (GdkDeviceManager  *device_manager,
                         GdkDevice         *device,
                         gpointer           user_data)
{
  AnnotateData *data = (AnnotateData *) user_data;

  if(data->debug)
    {
      g_printerr ("DEBUG: device '%s' removed\n", gdk_device_get_name(device));
    }

  remove_input_device (device, data);
}


/* On device removed. */
void on_device_added    (GdkDeviceManager  *device_manager,
                         GdkDevice         *device,
                         gpointer           user_data)
{
  AnnotateData *data = (AnnotateData *) user_data;

  if(data->debug)
    {
      g_printerr ("DEBUG: device '%s' added\n", gdk_device_get_name (device));
    }

  add_input_device (device, data);
}

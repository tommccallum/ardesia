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

/* Widget for text insertion */


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <utils.h>
#include <annotation_window.h>
#include <text_window.h>
#include <keyboard.h>
#include <bar_callbacks.h>
#include <text_window_callbacks.h>
#include <cairo_functions.h>

#ifdef _WIN32
#  include <windows_utils.h>
#endif


/* The structure used to store the status. */
TextData *text_data = (TextData *) NULL;

/* The structure used to configure text input. */
TextConfig *text_config = (TextConfig *) NULL;

/** Setup text configuration, used from ardesia.c */
TextConfig*
create_text_config() {
    TextConfig* text_config = g_malloc ((gsize) sizeof (TextConfig));
    text_config->fontfamily = "monospace";
    text_config->leftmargin = 0;
    text_config->tabsize = 80;
    text_config->start_x = 0;
    return text_config;
}





/* Stop the timer to handle the blocking cursor. */
void
stop_timer              ()
{
  if (text_data->timer>0)
    {
      g_source_remove (text_data->timer);
      text_data->timer = -1;
    }
}

void
start_blink_cursor() {
    // start blink cursor every second
    text_data->blink_show=TRUE;
    blink_cursor (NULL);
    text_data->timer = g_timeout_add (750, blink_cursor, NULL);
}

void
stop_blink_cursor() {
    stop_timer ();
    text_data->blink_show=FALSE;
    blink_cursor (NULL);
}



// /* Create the text window. */
// static void
// create_text_window           (GtkWindow *parent)
// {
//   GError *error = (GError *) NULL;
//
//   if (!text_data->text_window_gtk_builder)
//     {
//       /* Initialize the main window. */
//       text_data->text_window_gtk_builder = gtk_builder_new ();
//
//       /* Load the gtk builder file created with glade. */
//       gtk_builder_add_from_file (text_data->text_window_gtk_builder, TEXT_UI_FILE, &error);
//
//       if (error)
//         {
//           g_warning ("Failed to load builder file: %s", error->message);
//           g_error_free (error);
//           return;
//         }
//
//     }
//
//   if (!text_data->window)
//     {
//       GObject *text_obj = gtk_builder_get_object (text_data->text_window_gtk_builder, "text_window");
//       text_data->window = GTK_WIDGET (text_obj);
//
//       /* Connect all the callback from gtkbuilder xml file. */
//       gtk_builder_connect_signals (text_data->text_window_gtk_builder, (gpointer) text_data->window);
//
//       /* This trys to set an alpha channel. */
//       on_text_window_screen_changed(text_data->window, NULL, text_data);
//       gtk_window_set_opacity (GTK_WINDOW (text_data->window), 1);
//       #ifdef _WIN32
//         /* I use a layered window that use the black as transparent color. */
//         setLayeredGdkWindowAttributes (gtk_widget_get_window  (text_data->window), RGB (0,0,0), 0, LWA_COLORKEY);
//       #endif
//
//       //gtk_widget_grab_focus (text_data->window);
//       gtk_widget_show_all( text_data->window );
//
//       // test window should be same as the annotation window
//       int width = gtk_widget_get_allocated_width(annotation_window);
//       int height = gtk_widget_get_allocated_width(annotation_window);
//       g_printf("resizing text window: %d %d\n", width, height);
//       gtk_widget_set_size_request (GTK_WIDGET (text_data->window), width, height);
//
//       // move to be over the top of the annotation window
//       int x,y;
//       gdk_window_get_root_coords( gtk_widget_get_window(annotation_window), 0,0, &x, &y );
//       g_printf("relocating text window: %d %d\n", x, y);
//       gdk_window_move( gtk_widget_get_window(text_data->window), x, y);
//
//       // keep above other windows
//       gtk_window_set_keep_above (GTK_WINDOW (text_data->window), TRUE);
//   }
// }





/* Blink cursor. */
gboolean blink_cursor        (gpointer data)
{

  if ( (text_data->pos) &&
        (text_data->cr)
    ) {

    gint height = text_data->max_font_height;
    cairo_t* cr = text_data->cr;

    cairo_save( cr );
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);


    if (text_data->blink_show)
    {
        cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
        cairo_set_line_width (cr, text_data->pen_width);
        cairo_set_source_color_from_string (cr, text_data->color);
        cairo_rectangle (cr, text_data->pos->x, text_data->pos->y - height, TEXT_CURSOR_WIDTH, height);
        text_data->blink_show = FALSE;
    }
    else
    {
        cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
        cairo_rectangle (cr, text_data->pos->x, text_data->pos->y - height, TEXT_CURSOR_WIDTH, height);

        cairo_rectangle (cr,
                       text_data->pos->x-1,
                       text_data->pos->y - height - 1,
                       TEXT_CURSOR_WIDTH  + 2,
                       height + 2);
        text_data->blink_show=TRUE;
    }

    cairo_fill (cr);
    cairo_stroke (cr);
    cairo_restore( cr );
    gtk_widget_queue_draw(annotation_data->annotation_window);
  }

  return TRUE;
}








/* Set the text cursor. */
static gboolean
assign_text_cursor_to_window              (GtkWidget  *window)
{

  gdouble decoration_height = 4;
  gint height = text_data->max_font_height + decoration_height * 2;
  gint width = TEXT_CURSOR_WIDTH * 3;
  g_printf("assign new cursor to window %d %d %s\n", width,height, text_data->color);
  cairo_surface_t *text_surface_t = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
  cairo_t *text_pointer_cr = cairo_create (text_surface_t);
  GdkRGBA *foreground_color_p = rgba_to_gdkcolor (text_data->color);
  GdkCursor *cursor = (GdkCursor *) NULL;
  GdkPixbuf *pixbuf = (GdkPixbuf *) NULL;

  if (text_pointer_cr)
    {
        clear_cairo_context (text_pointer_cr);
        cairo_save(text_pointer_cr);
        cairo_set_source_color_from_string (text_pointer_cr, text_data->color);
        cairo_set_operator (text_pointer_cr, CAIRO_OPERATOR_SOURCE);
        cairo_set_line_width (text_pointer_cr, 2);

        cairo_line_to (text_pointer_cr, 1, 1);
        cairo_line_to (text_pointer_cr, width-1, 1);
        cairo_line_to (text_pointer_cr, width-1, decoration_height);
        cairo_line_to (text_pointer_cr, 2*width/3+1, decoration_height);
        cairo_line_to (text_pointer_cr, 2*width/3+1, height-decoration_height);
        cairo_line_to (text_pointer_cr, width-1, height-decoration_height);
        cairo_line_to (text_pointer_cr, width-1, height-1);
        cairo_line_to (text_pointer_cr, 1, height-1);
        cairo_line_to (text_pointer_cr, 1, height-decoration_height);
        cairo_line_to (text_pointer_cr, width/3-1, height-decoration_height);
        cairo_line_to (text_pointer_cr, width/3-1, decoration_height);
        cairo_line_to (text_pointer_cr, 1, decoration_height);
        cairo_close_path (text_pointer_cr);

        cairo_stroke (text_pointer_cr);
        cairo_restore(text_pointer_cr);

        cairo_destroy (text_pointer_cr);
        pixbuf = gdk_pixbuf_get_from_surface (text_surface_t,
                                            0,
                                            0,
                                            width,
                                            height);


        cursor = gdk_cursor_new_from_pixbuf(gdk_window_get_display (gtk_widget_get_window(window)),
                                            pixbuf,
                                            width/2-decoration_height,
                                            height-decoration_height);

        gdk_window_set_cursor (gtk_widget_get_window(window), cursor);
        gtk_widget_queue_draw(window);
    }


  g_object_unref (cursor);
  g_object_unref (pixbuf);
  cairo_surface_destroy (text_surface_t);
  g_free (foreground_color_p);


  return TRUE;
}


/* Add a save-point with the text. Called from stop_text_widget*/
void
save_text()
{
  if ( text_data != NULL )
    {
      stop_blink_cursor();
      if ( text_data->cr ) {
          if (text_data->letterlist)
            {
              annotate_push_context (text_data->cr);
              g_slist_free_full(text_data->letterlist,
                               (GDestroyNotify) destroy_text_properties);
              text_data->letterlist = NULL;
            }
        }
    }
}

static void
clear_if_empty() {
    if (!text_data->letterlist) {
        if ( text_data->cr != NULL ) {
            g_printf("cleaning text window\n");
            clear_cairo_context (text_data->cr);
        }
    }
}

// static void
// embed_tools_window(GtkWidget* widget) {
//     // make room for tools screen shape if appropriate
//     gtk_widget_input_shape_combine_region(widget, NULL);
//     drill_window_in_bar_area (widget);
// }


static void
set_cursor_height(GtkWidget* widget) {

    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_width(widget);

    // this is to find out what size we should make the cursor
    // we don't need the window cairo context for this
    // so we just create a blank surface
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height );
    cairo_t* cr = cairo_create(surface);
    cairo_save(cr);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_line_width (cr, text_data->pen_width);
    cairo_set_source_color_from_string (cr, text_data->color);
    cairo_set_font_size (cr, text_data->pen_width * 5);

    /* Select the font */
    cairo_select_font_face (cr, text_config->fontfamily,
                            CAIRO_FONT_SLANT_NORMAL,
                            CAIRO_FONT_WEIGHT_NORMAL);

    /* This is a trick; we must found the maximum height of the font. */
    cairo_text_extents (cr, "|" , &text_data->extents);
    text_data->max_font_height = text_data->extents.height;
    cairo_restore(cr);

    cairo_surface_destroy( surface );
    cairo_destroy( cr );


}

/* Initialization routine. Called on text expose*/
void
init_text_widget             (GtkWidget *widget)
{
 // embed_tools_window(widget);

  set_cursor_height(widget);
  assign_text_cursor_to_window (widget);

#ifdef _WIN32
  grab_pointer (text_data->window, TEXT_MOUSE_EVENTS);
#endif

  clear_if_empty();

}

static void
create_text_data() {
    g_print("create_text_data\n");
    if ( text_data == NULL ) {
        g_print("createing new text_data and adding defaults\n");
        text_data = g_malloc ((gsize) sizeof (TextData));

        // set defaults back
        text_data->cr = NULL;
        text_data->pos = g_malloc ( (gsize) sizeof (Pos));
        text_data->pos->x = 0.0;
        text_data->pos->y = 0.0;
        text_data->letterlist = NULL;
        text_data->virtual_keyboard_pid = (GPid) 0;
        text_data->timer = -1;
        text_data->blink_show = TRUE;
        text_data->color =  "FF0000FF";
        text_data->pen_width = 1;
    }
}

/* Start the widget for the text insertion.
 * Triggered by the leaving of the mouse of the tool bar.
 * @param widget        window that called function (generally bar_window)
 * @param color         user selected color for text
 * @param thickness     user selected thickness for text
 */
void start_text_widget      (GtkWidget  *widget,
                             gchar      *color,
                             gint        thickness)
{
  g_printf("start_text_widget (%s)\n", color);
  create_text_data();
  text_data->color =  color;
  text_data->pen_width = thickness;
  text_data->cr = create_new_context( gtk_widget_get_allocated_width(widget),
                                        gtk_widget_get_allocated_width(widget) );
  init_text_widget(widget);
  annotation_data->is_text_editor_visible = TRUE;
}


/* Stop the text insertion widget. Triggered when mouse enters the bar again */
void
stop_text_widget             ()
{
    annotation_data->is_text_editor_visible = FALSE;
    g_printf("stop_text_widget\n");
  if (text_data)
    {
      stop_blink_cursor();
      stop_virtual_keyboard ();

      save_text (); // destroys letter list and passes CR to annotation window

      if (text_data->cr)
        {
          cairo_destroy (text_data->cr);
          text_data->cr = NULL;
        }

      if (text_data->pos)
        {
          g_free (text_data->pos);
          text_data->pos = NULL;
        }

      g_free (text_data);
      text_data = NULL;
    }
}

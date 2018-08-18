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

#include <utils.h>
#include <background_window.h>
#include <background_window_callbacks.h>
#include <annotation_window.h>
#include <cairo_functions.h>

BackgroundData *background_data;

BackgroundData*
create_background_data          ()
{
    g_printf("Creating background data object\n");
  BackgroundData *background_data   = g_malloc ((gsize) sizeof (BackgroundData));
  background_data->color = (gchar *) NULL;
  background_data->image = (gchar *) NULL;
  background_data->cr    = (cairo_t *) NULL;
  background_data->type = 0;
  return background_data;
}


/* Destroy the background data structure */
void
destroy_background_data         ()
{
  if (background_data)
    {

      if (background_data->cr)
        {
          cairo_destroy (background_data->cr);
          background_data->cr = (cairo_t *) NULL;
        }

      if (background_data->color)
        {
          g_free (background_data->color);
          background_data->color = (gchar *) NULL;
        }

      if (background_data)
        {
          g_free (background_data);
          background_data = (BackgroundData *) NULL;
        }

    }
}


/* Clear the background. */
void clear_background_context      ()
{
    g_printf("clear background window, destroying cairo context\n");
    background_data->type = 0;
  // /*
  //  * @HACK Deny the mouse input to go below the window putting the opacity greater than 0
  //  * I avoid a complete transparent window because in some operating system this would become
  //  * transparent to the pointer input also.
  //  *
  //  */
  // gtk_window_set_opacity (GTK_WINDOW (annotation_data->annotation_window), BACKGROUND_OPACITY);

  clear_cairo_context (background_data->cr);

  gtk_widget_queue_draw( annotation_data->annotation_window );
}

// void
// make_background_window_transparent() {
//     /* This trys to set an alpha channel. */
//     on_back_screen_changed (background_data->background_window, NULL, background_data);
//     gtk_window_set_opacity (GTK_WINDOW (background_data->background_window), BACKGROUND_OPACITY);
// }
//
// void
// position_background_window(int x, int y, int width, int height) {
//     gtk_window_set_keep_above (GTK_WINDOW (background_data->background_window), TRUE);
//
//     make_background_window_transparent();
//
//     gtk_widget_set_size_request (background_data->background_window, width, height );
//     gtk_widget_show (background_data->background_window);
// }

/* Create the background window. */
// GtkWidget *
// create_background_window     ()
// {
//   GError *error = (GError *) NULL;
//   GObject *background_obj = (GObject *) NULL;
//
//   background_data = create_background_data ();
//
//   /* Initialize the background window. */
//   background_data->background_window_gtk_builder = gtk_builder_new ();
//
//   /* Load the gtk builder file created with glade. */
//   gtk_builder_add_from_file (background_data->background_window_gtk_builder, BACKGROUND_UI_FILE, &error);
//
//   if (error)
//     {
//       g_warning ("Failed to load builder file: %s", error->message);
//       g_error_free (error);
//       return background_data->background_window;
//     }
//
//   background_obj = gtk_builder_get_object (background_data->background_window_gtk_builder, "backgroundWindow");
//   background_data->background_window = GTK_WIDGET (background_obj);
//
//   /* Connect all the callback from gtkbuilder xml file. */
//   gtk_builder_connect_signals (background_data->background_window_gtk_builder, (gpointer) background_data);
//
//   return  background_data->background_window;
// }

//
// /* Get the background type */
// gint
// get_background_type          ()
// {
//   return background_data->type;
// }
//
//
// /* Get the background image */
// gchar *
// get_background_image         ()
// {
//   if (background_data)
//     {
//       return background_data->image;
//     }
//   return NULL;
// }
//
//
// /* Get the background colour */
// gchar *
// get_background_color         ()
// {
//   return background_data->color;
// }
//
//
// /* Set the background type. */
// void
// set_background_type          (gint type)
// {
//     // if ( background_data->type != type ) {
//     //     // we want to destroy context and recreate
//     //     clear_background_window();
//     // }
//   background_data->type = type;
// }
//
//
// /* Set the background image. */
// void
// set_background_image         (gchar *name)
// {
//   background_data->image = name;
// }


/* Update the background image. */
void
update_background_image      (gchar *name)
{
    g_printf("Updating background image: %s\n",name);
    background_data->type = 2;
    background_data->image = name;
    load_file_onto_context( background_data->image, background_data->cr);
    annotation_data->is_background_visible = TRUE;
    gtk_widget_queue_draw( annotation_data->annotation_window );
}
//
//
// /* Set the background colour. */
// void
// set_background_color         (gchar* rgba)
// {
//   background_data->color = g_strdup_printf ("%s", rgba);
// }


/* Update the background colour. */
void
update_background_color      (gchar* rgba)
{
    g_printf("Updating background color\n");
    background_data->type = 1;
  background_data->color = rgba;
  load_color_onto_context( background_data->color, background_data->cr);
  annotation_data->is_background_visible = TRUE;
  gtk_widget_queue_draw( annotation_data->annotation_window );
}
//
//
// /* Get the background window. */
// GtkWidget *
// get_background_window        ()
// {
//   return background_data->background_window;
// }
//
//
// /* Set the background window. */
// void
// set_background_window        (GtkWidget *widget)
// {
//   background_data->background_window = widget;
// }

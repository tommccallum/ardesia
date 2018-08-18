
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


#include <utils.h>
#include <commandline.h>
#include <bar.h>

BarData *bar_data = NULL;

/*
 * Calculate the better position where put the bar.
 */
static void
calculate_position (GtkWidget *ardesia_bar_window,
                    gint       d_width,
                    gint       d_height,
                    gint      *x,
                    gint      *y,
                    gint       w_width,
                    gint       w_height,
                    gint       position)
{
  *y = ((d_height - w_height - SPACE_FROM_BORDER)/2);
  /* Vertical layout. */
  if (position==WEST)
    {
      *x = 0;
    }
  else if (position==EAST)
    {
      *x = d_width - w_width;
    }
  else
    {
      /* Horizontal layout. */
      *x = (d_width - w_width)/2;
      if (position==NORTH)
        {
          /* Assuming that the bar is in south. */
          *y = 0; // on north SPACE_FROM_BORDER;
        }
      else if (position==SOUTH)
        {
          /* South. assuming bar is on south. */
          *y = d_height - SPACE_FROM_BORDER - w_height;
        }
      else
        {
          /* Invalid position. */
          perror ("Valid positions are NORTH, SOUTH, WEST or EAST\n");
          exit (EXIT_FAILURE);
        }
    }
}


/*
 * Calculate the initial position.
 */
static void
calculate_initial_position (GtkWidget *ardesia_bar_window,
                            gint *x,
                            gint *y,
                            gint w_width,
                            gint w_height,
                            GdkRectangle* rect,
                            gint position)
{
    // GdkDisplay* display = gdk_display_get_default();
    // assert( display != NULL );
    // GdkMonitor* monitor = gdk_display_get_monitor_at_window(display, gtk_widget_get_window(ardesia_bar_window));
    // assert( monitor != NULL );
    // //int scaleFactor = gdk_monitor_get_scale_factor (monitor);
    // GdkRectangle* rect = g_new(GdkRectangle,1);
    // gdk_monitor_get_geometry( monitor, rect);
    // assert(rect != NULL);
    // g_printf("Monitor Geometry: %d %d\n", rect->width, rect->height);
    gint d_width = rect->width;
    gint d_height = rect->height;
    //g_free(rect);
  /* Resize if larger that screen width. */
  if (w_width>d_width)
    {
      w_width = d_width;
      gtk_window_resize (GTK_WINDOW (ardesia_bar_window), w_width, w_height);
    }

  /* Resize if larger that screen height. */
  if (w_height>d_height)
    {
      gint tollerance = 15;
      w_height = d_height - tollerance;
      gtk_widget_set_size_request (ardesia_bar_window, w_width, w_height);
    }

  calculate_position (ardesia_bar_window, d_width, d_height, x, y, w_width, w_height, position);
}



/* Allocate and initialize the bar data structure. */
static BarData *
init_bar_data ()
{
  BarData *bar_data = (BarData *) g_malloc ((gsize) sizeof (BarData));
  bar_data->color = g_strdup_printf ("%s", "FF0000FF");
  bar_data->annotation_is_visible = TRUE;
  bar_data->grab = TRUE;
  bar_data->rectifier = FALSE;
  bar_data->rounder = FALSE;
  bar_data->thickness = MEDIUM_THICKNESS;
  bar_data->screenshot_pending = FALSE;
  bar_data->screenshot_callback = NULL;
  bar_data->screenshot_saved_location_x = -1;
  bar_data->screenshot_saved_location_y = -1;
  return bar_data;
}

gchar*
get_xdg_config_file (const char *name)
{
  const gchar *user_dir = g_get_user_config_dir();
  const gchar* const *system_dirs;
  const gchar* const *dir;
  gchar *file;

  system_dirs = g_get_system_config_dirs();
  file = g_build_filename(user_dir, name, NULL);
  if (g_file_test(file, G_FILE_TEST_EXISTS) == TRUE)
  {
      return file;
  }

  free(file);

  for (dir = system_dirs; *dir; ++dir )
  {
      file = g_build_filename(*dir, name, NULL);
      if (g_file_test(file, G_FILE_TEST_EXISTS) == TRUE) {
          return file;
      }
      free(file);
  }
  return NULL;
}

/* Create the ardesia bar window.
 * @rect    the monitor rectangle to place toolbar on
 */
GtkWidget *
create_bar_window (CommandLine *commandline,
                   GdkRectangle* rect,
                   GtkWidget   *parent)
{
  GtkWidget *bar_window = (GtkWidget *) NULL;
  bar_data = (BarData *) NULL;
  GError *error = (GError *) NULL;
  gchar *file = UI_FILE;
  gint x = 0;
  gint y = 0;
  gint width = 0;
  gint height = 0;


  /* Set up style for ardesia */
  gchar* gtkcss_file = get_xdg_config_file("ardesia/gtk.css");
  if (gtkcss_file)
    {
      GtkCssProvider *css = gtk_css_provider_new ();
      gtk_css_provider_load_from_path (css, gtkcss_file, NULL);
      g_free (gtkcss_file);
      gtk_style_context_add_provider_for_screen (gdk_screen_get_default(),
                                                 GTK_STYLE_PROVIDER(css),
                                                 GTK_STYLE_PROVIDER_PRIORITY_USER);
    }

  bar_gtk_builder = gtk_builder_new ();

  if (commandline->position>2)
    {
      /* North or south. */
      file = UI_HOR_FILE;
    }
  else
    {
      /* East or west. */
      // GdkDisplay* display = gdk_display_get_default();
      // GdkMonitor* monitor = gdk_display_get_primary_monitor(display);
      // //int scaleFactor = gdk_monitor_get_scale_factor (monitor);
      // GdkRectangle* rect = NULL;
      // gdk_monitor_get_geometry( monitor, rect);
      int height_in_application_pixels = rect->height;
      if (height_in_application_pixels < 720)
        {
          /*
           * The bar is too long and then I use an horizontal layout;
           * this is done to have the full bar for net book and screen
           * with low vertical resolution.
           */
          file = UI_HOR_FILE;
          commandline->position=NORTH;
        }

    }


  /* Load the bar_gtk_builder file with the definition of the ardesia bar gui. */
  g_printf("File: %s\n", file);
  gtk_builder_add_from_file (bar_gtk_builder, file, &error);
  if (error)
    {
      g_warning ("Failed to load builder file: %s", error->message);
      g_error_free (error);
      g_object_unref (bar_gtk_builder);
      bar_gtk_builder = NULL;
      return bar_window;
    }

  bar_data = init_bar_data ();

  bar_window = GTK_WIDGET (gtk_builder_get_object (bar_gtk_builder, BAR_WIDGET_NAME));
  gtk_widget_set_name (bar_window, BAR_WIDGET_NAME);

  /* Connect all the callback from bar_gtk_builder xml file. */
  gtk_builder_connect_signals (bar_gtk_builder, (gpointer) bar_data);

  //gtk_window_set_transient_for (GTK_WINDOW (bar_window), GTK_WINDOW (parent));


  if (commandline->decorated)
    {
      gtk_window_set_decorated (GTK_WINDOW (bar_window), TRUE);
    }

  gtk_window_get_size (GTK_WINDOW (bar_window) , &width, &height);

  // returns the bounds of where we would the area to be
  //GdkRectangle* bounds =  get_toolbar_area();

  /* x and y will be the bar left corner coordinates. */
  calculate_initial_position (bar_window,
                              &x,
                              &y,
                              width,
                              height,
                              rect,
                              commandline->position);

  /* The position is calculated respect the top left corner
   * and then I set the north west gravity.
   */
  gtk_window_set_gravity (GTK_WINDOW (bar_window), GDK_GRAVITY_NORTH_WEST);

  /* Move the window in the desired position. */
  gtk_window_move (GTK_WINDOW (bar_window), rect->x + x, rect->y + y);


  return bar_window;
}

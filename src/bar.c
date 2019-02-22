
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
#include "annotation_window.h"
#include "text_window.h"
#include "background_window.h"

/* Timer used to up-rise the window. */
static gint timer = -1;



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
  bar_data->color = g_strdup_printf ("%s", "FFFF00FF"); // default to yellow
  bar_data->annotation_is_visible = TRUE;
  bar_data->grab = TRUE;
  bar_data->rectifier = FALSE;
  bar_data->rounder = FALSE;
  bar_data->thickness = THIN_THICKNESS;
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


  // select arrow tool and the color yellow
  GtkToggleToolButton* arrowButton = GTK_TOGGLE_TOOL_BUTTON (gtk_builder_get_object (bar_gtk_builder, "buttonPointer"));
  GtkToggleToolButton* yellowButton = GTK_TOGGLE_TOOL_BUTTON (gtk_builder_get_object (bar_gtk_builder, "buttonYellow"));
  gtk_toggle_tool_button_set_active( yellowButton, TRUE );
  gtk_toggle_tool_button_set_active( arrowButton, TRUE );

  return bar_window;
}


GtkStatusbar* getStatusbar() {
    GObject *g_object = gtk_builder_get_object (bar_gtk_builder, gettext("statusbar") );
    return GTK_STATUSBAR( g_object );
}

void
replace_status_message( gchar* message ) {
    GtkStatusbar* bar = getStatusbar();
    if ( bar != NULL ) {
        //guint contextID = gtk_statusbar_get_context_id( bar, gettext("context description"));
        gtk_statusbar_pop( bar, 0 );
        gtk_statusbar_push( bar, 0, message );
    }
}

void
setStatusbarLabel( gchar* message ) {
    GObject *g_object = gtk_builder_get_object (bar_gtk_builder, gettext("labelCurrentSelection") );
    GtkLabel* label = GTK_LABEL( g_object );
    gtk_label_set_label( label, gettext(message) );
}
/* Try to up-rise the window;
 * this is used for the window manager
 * that does not support the stay above directive.
 */
gboolean
bar_to_top         (gpointer data)
{
  if (!gtk_widget_get_visible (GTK_WIDGET (data)))
    {
       gtk_window_present (GTK_WINDOW (data));
       gtk_widget_grab_focus (data);
       gdk_window_lower (gtk_widget_get_window (GTK_WIDGET (data)));
    }
  return TRUE;
}


/* Is the toggle tool button specified with name is active? */
gboolean
is_toggle_tool_button_active      (gchar *toggle_tool_button_name)
{
  GObject *g_object = gtk_builder_get_object (bar_gtk_builder, toggle_tool_button_name);
  GtkToggleToolButton *toggle_tool_button = GTK_TOGGLE_TOOL_BUTTON (g_object);
  return gtk_toggle_tool_button_get_active (toggle_tool_button);
}

/* Get GtkImage object from builder
 * 2018-08-09: TM
 */
GtkImage* get_image_from_builder(gchar *image_name) {
    GObject *g_object = gtk_builder_get_object (bar_gtk_builder, image_name);
    GtkImage *image = GTK_IMAGE (g_object);
    return image;
}

/* Is the show/hide toggle tool button active?
 * 2018-08-09 : Added by TM
 */
// static
// gboolean is_hide_toggle_tool_button_active    ()
// {
//   return is_toggle_tool_button_active ("buttonShowHide");
// }

/* Is the text toggle tool button active? */

gboolean is_text_toggle_tool_button_active       ()
{
  return is_toggle_tool_button_active ("buttonText");
}


/* Is the highlighter toggle tool button active? */

gboolean is_highlighter_toggle_tool_button_active          ()
{
  return is_toggle_tool_button_active ("buttonHighlighter");
}


/* Is the filler toggle tool button active? */

gboolean is_filler_toggle_tool_button_active          ()
{
  return is_toggle_tool_button_active ("buttonFiller");
}


/* Is the eraser toggle tool button active? */

gboolean is_eraser_toggle_tool_button_active     ()
{
  return is_toggle_tool_button_active ("buttonEraser");
}


/* Is the eraser toggle tool button active? */

gboolean is_pen_toggle_tool_button_active     ()
{
  return is_toggle_tool_button_active ("buttonPencil");
}


/* Is the pointer toggle tool button active? */

gboolean is_pointer_toggle_tool_button_active    ()
{
  return is_toggle_tool_button_active ("buttonPointer");
}


/* Is the pointer toggle tool button active? */

gboolean is_arrow_toggle_tool_button_active      ()
{
  return is_toggle_tool_button_active ("buttonArrow");
}


/* Add alpha channel to build the RGBA string. */
void
add_alpha               (BarData *bar_data)
{
    assert(strlen(bar_data->color) == 8 );
  if (is_highlighter_toggle_tool_button_active ())
    {
      strncpy (&bar_data->color[6], SEMI_OPAQUE_ALPHA, 2);
    }
  else
    {
      strncpy (&bar_data->color[6], OPAQUE_ALPHA, 2);
    }
}


/* Select the pen tool. */
void
take_pen_tool           ()
{
  GObject *pencil_obj = gtk_builder_get_object (bar_gtk_builder, "buttonPencil");
  GtkToggleToolButton *pencil_tool_button = GTK_TOGGLE_TOOL_BUTTON (pencil_obj);

  /* Select the pen as default tool. */
  if (is_eraser_toggle_tool_button_active ())
    {
      GObject *eraser_obj = gtk_builder_get_object (bar_gtk_builder, "buttonEraser");
      GtkToggleToolButton *eraser_tool_button = GTK_TOGGLE_TOOL_BUTTON (eraser_obj);
      gtk_toggle_tool_button_set_active (eraser_tool_button, FALSE);
      gtk_toggle_tool_button_set_active (pencil_tool_button, TRUE);
    }

  if (is_pointer_toggle_tool_button_active ())
    {
      GObject *pointer_obj = gtk_builder_get_object (bar_gtk_builder, "buttonPointer");
      GtkToggleToolButton *pointer_tool_button = GTK_TOGGLE_TOOL_BUTTON (pointer_obj);
      gtk_toggle_tool_button_set_active (pointer_tool_button, FALSE);
      gtk_toggle_tool_button_set_active (pencil_tool_button, TRUE);
    }

  if (is_filler_toggle_tool_button_active ())
    {
      GObject *filler_obj = gtk_builder_get_object (bar_gtk_builder, "buttonFiller");
      GtkToggleToolButton *filler_tool_button = GTK_TOGGLE_TOOL_BUTTON (filler_obj);
      gtk_toggle_tool_button_set_active (filler_tool_button, FALSE);
      gtk_toggle_tool_button_set_active (pencil_tool_button, TRUE);
    }
}


/* Release to lock the mouse */
void
release_lock                 (BarData *bar_data)
{
    g_printf("releasing lock (grab: %d)\n", bar_data->grab);
  if (bar_data->grab)
    {
      /* Lock enabled. */
      bar_data->grab = FALSE;
      annotate_release_grab ();

      /* Try to up-rise the window. */
      timer = g_timeout_add (BAR_TO_TOP_TIMEOUT, bar_to_top, get_annotation_window());
#ifdef _WIN32 // WIN32
      if (gtk_window_get_opacity (GTK_WINDOW (get_annotation_window ()))!=0)
        {
          /*
           * @HACK This allow the mouse input go below the window putting
           * the opacity to 0; when will be found a better way to make
           * the window transparent to the the pointer input we might
           * remove the previous hack.
           * @TODO Transparent window to the pointer input in a better way.
           */
           gtk_window_set_opacity (GTK_WINDOW (get_annotation_window ()), 0);
        }
#endif

        gtk_widget_queue_draw(annotation_data->annotation_window );
        
    }
}


/* Lock the mouse. */
void
lock (BarData *bar_data)
{
  if (! bar_data->grab)
    {
      // Unlock
      bar_data->grab = TRUE;

      /* delete the old timer */
      if (timer!=-1)
        {
          g_source_remove (timer);
          timer = -1;
        }

#ifdef _WIN32 // WIN32

      /*
       * @HACK Deny the mouse input to go below the window putting the opacity greater than 0
       * @TODO remove the opacity hack when will be solved the next todo.
       */
      if (gtk_window_get_opacity (GTK_WINDOW (get_background_window ()))==0)
        {
          gtk_window_set_opacity (GTK_WINDOW (get_background_window ()), BACKGROUND_OPACITY);
        }
#endif
    }
}


/* Set color; this is called each time that the user want change color. */
void
set_color                    (BarData  *bar_data,
                              gchar    *selected_color)
{
  take_pen_tool ();
  lock (bar_data);
  assert( strlen(selected_color) >= 6  );
  strncpy (bar_data->color, selected_color, 6);
  annotate_set_color (bar_data->color);
}


/* Pass the options to the annotation window. */
void set_options      (BarData *bar_data)
{

  annotate_set_rectifier (bar_data->rectifier);

  annotate_set_rounder (bar_data->rounder);

  annotate_set_thickness (bar_data->thickness);

  annotate_set_arrow (is_arrow_toggle_tool_button_active ());

  if (is_pen_toggle_tool_button_active ()         ||
      is_highlighter_toggle_tool_button_active () ||
      is_arrow_toggle_tool_button_active ())
    {
      annotate_set_color (bar_data->color);
      annotate_select_pen ();
    }
  else if (is_eraser_toggle_tool_button_active ())
    {
      annotate_select_eraser ();
    }

}






/* Start to paint with the selected tool. */
void
start_tool                   (BarData *bar_data)
{
  if (bar_data->grab)
    {
        annotate_release_grab (); // release the old cursor
        annotate_acquire_grab (); // grab the pointer again so that button release will respond

      if (is_text_toggle_tool_button_active ())
        {
          /* Text button then start the text widget. */
          start_text_widget (annotation_data->annotation_window,
                             bar_data->color,
                             bar_data->thickness);
        }
      else
        {
            g_print("DEBUG: start_tool (non-text tool selected)\n");
            // this call is required as the leave event for the bar occurs
            // when we enter the toolbar object
            stop_text_widget();
          /* Is an other tool for paint or erase. */
          set_options (bar_data);
        }

    }
}

gboolean end_clapperboad_countdown(gpointer user_data) {
    g_print("END on_clapperboard_click");
    gboolean grab_value = bar_data->grab;
    bar_data->grab = FALSE;
    annotate_release_grab ();

    // ideally we want to go back to our background settings that we had before
    annotation_data->is_clapperboard_visible = FALSE;

    // make the screen black and then go back to what it was before
    bar_data->grab = grab_value;
    start_tool (bar_data);
    gtk_widget_queue_draw(annotation_data->annotation_window);
    return FALSE;
}

void begin_clapperboard_countdown() {
    timer = g_timeout_add (BAR_TO_TOP_TIMEOUT, end_clapperboad_countdown, NULL);
}

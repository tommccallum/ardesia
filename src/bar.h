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


#include <config.h>
#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <ardesia.h>


#ifdef _WIN32
#  include <windows_utils.h>
#  define UI_FILE "..\\share\\ardesia\\ui\\vertical_bar.glade"
#  define UI_HOR_FILE "..\\share\\ardesia\\ui\\horizontal_bar.glade"
#else
#  define UI_FILE PACKAGE_DATA_DIR"/ardesia/ui/vertical_bar.glade"
#  define UI_HOR_FILE PACKAGE_DATA_DIR"/ardesia/ui/horizontal_bar.glade"
#endif


/* Distance space from border to the ardesia bar in pixel unit. */
#define SPACE_FROM_BORDER 35

#define MICRO_THICKNESS   3
#define THIN_THICKNESS    6
#define MEDIUM_THICKNESS  12
#define THICK_THICKNESS   18

/* Semi opaque (and then semi transparent) alpha;
 * this is used to make the highlighter effect.
 */
#define SEMI_OPAQUE_ALPHA "88"

/* full opaque alpha */
#define OPAQUE_ALPHA "FF"


/* The time-out after that the tool try to up-rise the window;
 * this is done to prevent the window lowering in the case that
 * the window manager does not support the stay above directive.
 */
#define  BAR_TO_TOP_TIMEOUT 1000


/* Structure that contains the info passed to the callbacks. */
typedef struct
{

  /* rectifier flag. */
  gboolean rectifier;

  /* rounder flag. */
  gboolean rounder;

  /* selected colour in RGBA format. */
  gchar* color;

  /* selected line thickness. */
  gint thickness;

  /* annotation is visible. */
  gboolean annotation_is_visible;

  /* grab when leave. */
  gboolean grab;

  /* we want to take a snapshot */
  gboolean screenshot_pending;
  void (*screenshot_callback) ( GdkPixbuf* );
  gint screenshot_saved_location_x;
  gint screenshot_saved_location_y;
}BarData;

BarData* bar_data;

/* Create the ardesia bar window. */
GtkWidget *
create_bar_window (CommandLine *commandline,
                    GdkRectangle* rect,
                   GtkWidget   *parent);


                   gboolean
                   bar_to_top         (gpointer data);
gboolean is_toggle_tool_button_active      (gchar *toggle_tool_button_name);
GtkImage* get_image_from_builder(gchar *image_name);
gboolean is_text_toggle_tool_button_active();
gboolean is_highlighter_toggle_tool_button_active();
gboolean is_filler_toggle_tool_button_active();

                   gboolean is_eraser_toggle_tool_button_active     ();

                   gboolean is_pen_toggle_tool_button_active     ();

                   gboolean is_pointer_toggle_tool_button_active    ();

gboolean is_arrow_toggle_tool_button_active      ();
void add_alpha               (BarData *bar_data);
                   void take_pen_tool           ();
                   void release_lock                 (BarData *bar_data);
void lock (BarData *bar_data);
void set_color                    (BarData  *bar_data,gchar    *selected_color);
void set_options      (BarData *bar_data);
void start_tool                   (BarData *bar_data);
void begin_clapperboard_countdown();

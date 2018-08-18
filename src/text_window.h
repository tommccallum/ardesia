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

/** Widget for text insertion */

#ifndef __TEXT_WINDOW_H
#define __TEXT_WINDOW_H

#include <ctype.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <cairo.h>


#ifdef _WIN32
#  include <cairo-win32.h>
#else
#  ifdef __APPLE__
#    include <cairo-quartz.h>
#  else
#    include <cairo-xlib.h>
#  endif
#endif

#define TEXT_CURSOR_WIDTH 4


#ifdef _WIN32
#  define TEXT_MOUSE_EVENTS         (GDK_POINTER_MOTION_MASK|  \
                                     GDK_BUTTON_PRESS_MASK  |  \
                                     GDK_BUTTON_RELEASE_MASK|  \
                                     GDK_PROXIMITY_IN       |  \
                                     GDK_PROXIMITY_OUT      |  \
                                     GDK_MOTION_NOTIFY      |  \
                                     GDK_BUTTON_PRESS          \
                                    )

#  define TEXT_UI_FILE "..\\share\\ardesia\\ui\\text_window.glade"
#else
#  define TEXT_UI_FILE PACKAGE_DATA_DIR"/ardesia/ui/text_window.glade"
#endif

// Per character settings so we can extend the application to have
// multiple characteristics.
typedef struct
{

  gdouble x;

  gdouble y;

  gchar* color;

  gchar* background_color;

  gint pen_width; // width of normal text

  gchar* character;

  gchar* font_family; // e.g serif

  guint font_size;

  guint font_weight;

  gboolean bold;

  gboolean italics;

  gboolean subscript;

  gboolean superscript;

  cairo_text_extents_t extents;

} CharInfo;


typedef struct
{

  gdouble x;

  gdouble y;

} Pos;


typedef struct
{

  /* Gtkbuilder to build the window. */
  GtkBuilder *text_window_gtk_builder;

  GtkWidget  *window;

  GPid virtual_keyboard_pid;

  cairo_t *cr;

  Pos *pos;

  GSList *letterlist;

  gchar *color;

  gint pen_width;

  gdouble max_font_height;

  cairo_text_extents_t extents;

  gint timer;

  gboolean blink_show;

}TextData;


/* Option for text config */
typedef struct
{
  gchar *fontfamily;
  gint leftmargin;
  gint tabsize;
  gint start_x;          // where first character will go
}TextConfig;

TextData *text_data;
TextConfig *text_config;

TextConfig*
create_text_config();

void
make_cairo_context_for_text_window();

cairo_t*
create_new_text_window_context();

cairo_t*
create_copy_of_text_window_context(cairo_t* current_context) ;

// void
// render_draw_frame(cairo_t* source_context) ;

void
stop_timer              ();

void
start_blink_cursor();

void
stop_blink_cursor();

void
draw_test_text(cairo_t* cr, gchar* text);

gboolean
blink_cursor        (gpointer data);

void
save_text          ();

void
init_text_widget             (GtkWidget *widget);

/* Start text widget. */
void
start_text_widget            (GtkWidget  *parent,
                              gchar      *color,
                              gint        thickness);


/* Stop text widget. */
void
stop_text_widget        ();


#endif //__TEXT_WINDOW_H

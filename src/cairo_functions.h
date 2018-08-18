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

#ifndef __CAIRO_FUNCTIONS_H
#define __CAIRO_FUNCTIONS_H


#include <glib.h>
#include <gtk/gtk.h>
#include <cairo.h>

#ifdef _WIN32
#  include <cairo-win32.h>
#  include <gdkwin32.h>
#  include <winuser.h>
#else
#  ifdef __APPLE__
#    include <cairo-quartz.h>
#  else
#    include <cairo-xlib.h>
#  endif
#endif

void
draw_cairo_context (cairo_t* dest, cairo_t* source) ;

cairo_surface_t*
scale_image( gchar* image, gint new_width, gint new_height ) ;
void
load_file_onto_context(gchar* filename, cairo_t* cr);
void
load_color_onto_context(gchar* hex_color, cairo_t* cr);
cairo_t*
create_new_context() ;
cairo_t*
create_copy_of_context(cairo_t* current_context);
void
draw_test_text(cairo_t* cr, gchar* text);

#endif //__CAIRO_FUNCTIONS_H

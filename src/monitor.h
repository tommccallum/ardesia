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

#ifndef __MONITOR_H
#define __MONITOR_H

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gprintf.h>

guint DRAW_ON_MONITOR;
guint DRAW_ON_CLIPAREA;
guint DRAW_ON_FULLDESKTOP;

typedef struct
{
    guint monitor_index;
    gboolean primary;
    GdkRectangle* rect;
} Monitor;


Monitor*
copy_monitor_struct( Monitor* m );

void
destroy_monitor_struct( gpointer data);

void
print_monitor_struct( gpointer data, gpointer userdata );

int
is_to_left_of( gconstpointer a, gconstpointer b, gpointer data );

GList*
create_monitor_list();

void
destroy_monitor_list();

void
print_monitor_list(GList* monitors);

#endif

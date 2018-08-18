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

#include <monitor.h>

guint DRAW_ON_MONITOR = 0;
guint DRAW_ON_CLIPAREA = 1;
guint DRAW_ON_FULLDESKTOP = 2;

/**
 * Creates a GList of monitors sorted by the pixel area they service
 * top-leftmost monitor is monitor 1 to rightmost monitor
 * @return [description]
 */
GList*
create_monitor_list() {
    // lets get some information about the displays first
    GdkDisplay* display = gdk_display_get_default();
    int monitorCount = gdk_display_get_n_monitors( display );
    g_printf( "Number of monitors: %d\n", monitorCount );

    GList* monitors = NULL;
    for( int ii=0; ii < monitorCount; ii++ ) {
        GdkRectangle* rect = g_new( GdkRectangle, 1 );
        GdkMonitor* monitor = gdk_display_get_monitor(display, ii);
        gboolean primary = gdk_monitor_is_primary( monitor );
        if ( primary ) {
            g_printf("Monitor %d is primary\n",ii);
        } else {
            g_printf("Monitor %d is not primary\n",ii);
        }
        gdk_monitor_get_geometry( monitor, rect );
        g_printf("Monitor %d Geometry: %d %d %d %d\n", ii, rect->x, rect->y, rect->width, rect->height);

        Monitor* m = g_new(Monitor, 1);
        m->primary = primary;
        m->monitor_index = ii;
        m->rect = rect;

        // place monitors in order
        monitors = g_list_insert_sorted_with_data( monitors, m, (GCompareDataFunc) is_to_left_of, NULL );
    }
    return monitors;
}

void
print_monitor_list(GList* monitors) {
    if ( monitors != NULL ) {
        g_printf("Monitor objects created:\n");
        g_list_foreach( monitors, (GFunc) print_monitor_struct, NULL );
    }
}

void
destroy_monitor_list(GList* monitors) {
    if ( monitors != NULL ) {
        g_list_free_full( monitors, (GDestroyNotify) destroy_monitor_struct );
    }
}

Monitor* copy_monitor_struct( Monitor* m ) {
    Monitor* new_m = g_new(Monitor,1);
    GdkRectangle* rect = g_new(GdkRectangle,1);
    new_m->monitor_index = m->monitor_index;
    new_m->rect = rect;
    new_m->rect->x = m->rect->x;
    new_m->rect->y = m->rect->y;
    new_m->rect->width = m->rect->width;
    new_m->rect->height = m->rect->height;

    return new_m;
}

void
destroy_monitor_struct( gpointer data) {
    Monitor* m = (Monitor*) data;
    g_free(m->rect);
}

void
print_monitor_struct( gpointer data, gpointer userdata ) {
    Monitor* m = (Monitor*) data;
    g_printf("Monitor %d: %d %d %d %d\n", m->monitor_index, m->rect->x, m->rect->y, m->rect->width, m->rect->height );
}

int
is_to_left_of( gconstpointer a, gconstpointer b, gpointer data ) {
    Monitor* monitorA = (Monitor*) a;
    Monitor* monitorB = (Monitor*) b;
    if ( monitorA->rect->x < monitorB->rect->x ) {
        return -1; // A < B
    } else {
        return 1; // B < A
    }
}

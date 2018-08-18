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

#ifndef __WORKSPACE_H
#define __WORKSPACE_H

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <monitor.h>

typedef struct {
    gchar* date;
    gchar* workspace_dir;
    gchar* project_name;
    gchar* project_dir;
    gchar* iwb_filename;
    GList* monitors;
} Workspace;

Workspace*
create_workspace();

void
set_defaults_for_workspace(Workspace* workspace);

void
configure_workspace               (Workspace* workspace);

void
change_workspace_to( Workspace* workspace, gchar* filename );

void
build_workspace_filesystem( Workspace* workspace );

void
destroy_workspace(Workspace* workspace);

void
print_workspace(Workspace* workspace) ;

#endif

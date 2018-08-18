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

#ifndef __COMMANDLINE_H
#define __COMMANDLINE_H

#include <gdk/gdk.h>
#include <gtk/gtk.h>

/* The structure that contains the command line info. */
typedef struct
{

  /* The file name of the iwb. */
  gchar *iwb_filename;

  /* Is the debug mode enabled? */
  gboolean debug;

  /* Is the bar windows decorated? */
  gboolean decorated;

  /* Where is located the ardesia bar? */
  gint position;

  /* Options for text_window */
  gchar *fontfamily;
  gint text_leftmargin;
  gint text_tabsize;

  guint mode;
  // either the user can select a monitor for the workspace and tools screens
  gint tools_monitor;
  gint workspace_monitor;
  // or they can choose an area of the screen
  GdkRectangle* clipRect;
  gboolean is_opaque;

} CommandLine;

// Command line interface

CommandLine*
create_command_line();

void
destroy_command_line();

void
add_defaults_to_commandline();

/* Parse the command line in the standard getopt way. */
void
parse_options           ( CommandLine* commandline,
                        gint   argc,
                         char  *argv[]);

void
print_command_line(CommandLine* commandline);

#endif

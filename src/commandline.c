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

#include "commandline.h"
#include "monitor.h"
#include <config.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <ardesia.h>
#include <stdlib.h>


static struct option long_options[] = {
    /* These options set a flag. */
    {"help", no_argument,       0, 'h'},
    {"decorated", no_argument,  0, 'd'},
    {"verbose", no_argument,    0, 'V'},
    {"version", no_argument,    0, 'v'},
    /*
     * These options don't set a flag.
     * We distinguish them by their indices.
     */
    {"gravity", required_argument, 0, 'g'},
    {"font", required_argument, 0, 'f'},
    {"leftmargin", required_argument, 0, 'l'},
    {"tabsize", required_argument, 0, 't'},
    {"tools-monitor",required_argument, 0, 'm'},
    {"workspace-monitor",required_argument, 0, 'M'},
    {"x",required_argument,0,'x'},
    {"y",required_argument,0,'y'},
    {"width",required_argument,0,1000},
    {"height",required_argument,0,1001},
    {"opaque",no_argument,0,'o'},
    {0, 0, 0, 0}
};


CommandLine*
create_command_line() {
    CommandLine* commandline =  g_malloc ((gsize) sizeof (CommandLine));
    add_defaults_to_commandline( commandline );
    return commandline;
}

void
destroy_command_line(CommandLine* commandline) {
    g_printf("Destroying command line\n");
    if ( commandline->clipRect != NULL ) {
        g_free( commandline->clipRect );
    }
    g_free( commandline );
}

void
add_defaults_to_commandline( CommandLine* commandline ) {
    commandline->position = EAST;
    commandline->debug = FALSE;
    commandline->iwb_filename = NULL;
    commandline->decorated=FALSE;
    commandline->fontfamily = "serif";
    commandline->text_leftmargin = 0;
    commandline->text_tabsize = 80;
    commandline->mode = DRAW_ON_MONITOR;
    commandline->workspace_monitor = 1;
    commandline->tools_monitor = 1;
    commandline->clipRect = g_new(GdkRectangle,1) ;

    commandline->clipRect->x = 0;
    commandline->clipRect->y = 0;
    commandline->clipRect->width = 200;
    commandline->clipRect->height = 200;
    commandline->is_opaque = FALSE;
}

/* Print the version of the tool and exit. */
static void
print_version      ()
{
  g_printf ("Ardesia %s; the free digital sketchpad\n\n", PACKAGE_VERSION);
  exit (EXIT_SUCCESS);
}


/* Print the command line help. */
static void
print_help         ()
{
  gchar *original_year = "2009-2010";
  gchar *original_author = "Pietro Pilolli";
  gchar *new_year = "2018";
  gchar *new_author = "Tom McCallum";
  g_printf ("Usage: %s [options] [filename]\n\n", PACKAGE_NAME);
  g_printf ("Ardesia the free digital sketchpad\n\n");
  g_printf ("options:\n");
  g_printf ("  --verbose ,\t\t-V\t\tEnable verbose mode to see the logs\n");
  g_printf ("  --decorate,\t\t-d\t\tDecorate the window with the borders\n");
  g_printf ("  --gravity ,\t\t-g\t\tSet the gravity of the bar. Possible values are:\n");
  g_printf ("  \t\t\t\t\teast [default]\n");
  g_printf ("  \t\t\t\t\twest\n");
  g_printf ("  \t\t\t\t\tnorth\n");
  g_printf ("  \t\t\t\t\tsouth\n");
  g_printf ("  --font ,\t\t-f\t\tSet the font family for the text window. Possible values are:\n");
  g_printf ("  \t\t\t\t\tserif [default]\n");
  g_printf ("  \t\t\t\t\tsans-serif\n");
  g_printf ("  \t\t\t\t\tmonospace\n");
  g_printf ("  --leftmargin,\t\t-l\t\tSet the left margin in text window to set after hitting Enter\n");
  g_printf ("  --tabsize,\t\t-t\t\tSet the tabsize in pixel in text window\n");

  g_printf ("  --coverage,\t\t-c\t\tSet whether to fit to _monitor_, an _area_ or _full_\n");
  g_printf ("  --tools-monitor,\t-m\t\tSet which monitor has the tools window appear (default: 1)\n");
  g_printf ("  --workspace-monitor,\t-M\t\tSet which monitor the main window will appear over (default: 1)\n");
  g_printf ("  -x\t\t\t\t\tSet the x position of the main window (default: 0)\n");
  g_printf ("  -y\t\t\t\t\tSet the y position of the main window (default: 0)\n");
  g_printf ("  --width,\t\t\t\tSet the width of the main window (default: 200)\n");
  g_printf ("  --height,\t\t\t\tSet the height of the main window (default: 200)\n");

  g_printf ("  --opaque,\t\t-o\t\tForce the main window to be opaque and not transparent\n");
  g_printf ("  --help    ,\t\t-h\t\tShows the help screen\n");
  g_printf ("  --version ,\t\t-v\t\tShows version information and exit\n");
  g_printf ("\n");
  g_printf ("filename:\t\t  \t\tThe interactive Whiteboard Common File (iwb)\n");
  g_printf ("\n");
  g_printf ( "Originally written by %s in %s\n", original_author, original_year );
  g_printf ("%s (C) %s %s\n", PACKAGE_STRING, new_year, new_author);
  exit (EXIT_FAILURE);
}


/* Parse the command line in the standard getopt way. */
void
parse_options           (CommandLine* commandline,
                         gint   argc,
                         char  *argv[])
{

  /* Getopt_long stores the option index here. */
  while (1)
    {
      gint c;


      gint option_index = 0;
      c = getopt_long (argc,
                       argv,
                       "hdvVg:f:l:t:w:c:m:M:x:y:o",
                       long_options,
                       &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        {
          break;
        }

      switch (c)
        {
          case 'h':
            print_help ();
            break;
          case 'v':
            print_version ();
            break;
          case 'd':
            commandline->decorated=TRUE;
            break;
          case 'V':
            commandline->debug=TRUE;
            break;
          case 'g':
            if (g_strcmp0 (optarg, "east") == 0)
              {
                commandline->position = EAST;
              }
            else if (g_strcmp0 (optarg, "west") == 0)
              {
                commandline->position = WEST;
              }
            else if (g_strcmp0 (optarg, "north") == 0)
              {
                commandline->position = NORTH;
              }
            else if (g_strcmp0 (optarg, "south") == 0)
              {
                commandline->position = SOUTH;
              }
            else
              {
                print_help ();
              }
            break;
          case 'f':
            if (g_strcmp0 (optarg, "serif") == 0 ||
                g_strcmp0 (optarg, "sans-serif") == 0 ||
                g_strcmp0 (optarg, "monospace") == 0)
              {
                commandline->fontfamily = optarg;
              }
            break;
          case 'l':
            commandline->text_leftmargin = atoi(optarg);
            break;
          case 't':
            commandline->text_tabsize = atoi(optarg);
            break;
            case 'c':
            if (g_strcmp0 (optarg, "monitor") == 0)
              {
                commandline->mode = DRAW_ON_MONITOR;
              }
            else if (g_strcmp0 (optarg, "area") == 0)
              {
                commandline->mode = DRAW_ON_CLIPAREA;
            }
            else if ( g_strcmp0( optarg, "full") == 0 ) {
                commandline->mode = DRAW_ON_FULLDESKTOP;
            }
            else {
                    print_help();
                }
            break;

          case 'm':
            commandline->tools_monitor = atoi(optarg);
            break;
          case 'M':
            commandline->workspace_monitor = atoi(optarg);
            break;
          case 'x':
            commandline->clipRect->x = atoi(optarg);
            break;
          case 'y':
            commandline->clipRect->y = atoi(optarg);
            break;
          case 1000:
            commandline->clipRect->width = atoi(optarg);
            break;
          case 1001:
            commandline->clipRect->height = atoi(optarg);
            break;
          case 'o':
            commandline->is_opaque = TRUE;
            break;
          default:
            g_printf("Invalid argument given: %c\n", c);
            print_help ();
            break;
        }
    }

  if (optind<argc)
    {
      commandline->iwb_filename = argv[optind];
    }

}

void
print_command_line(CommandLine* commandline) {
    g_printf("Coverage: %s\n", ( commandline->mode == DRAW_ON_MONITOR ? "Monitor" : "Area" ) );
    g_printf("Tools Monitor: %d\n", commandline->tools_monitor);
    g_printf("Workspace Monitor: %d\n", commandline->workspace_monitor);
    g_printf("Rectangle: %d %d %d %d\n", commandline->clipRect->x, commandline->clipRect->y,
                                        commandline->clipRect->width, commandline->clipRect->height);
    g_printf("Is Opaque: %d\n", commandline->is_opaque);
}

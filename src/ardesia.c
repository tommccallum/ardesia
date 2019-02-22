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

#include <ardesia.h>
#include <utils.h>
#include <bar_callbacks.h>
#include <annotation_window.h>
#include <background_window.h>
#include <project_dialog.h>
#include <bar.h>
#include <commandline.h>
#include <text_window.h>

/*ch* External defined structure used to configure text input. (see text_window.c) */
#include <text_window.h>
extern TextConfig *text_config;
GtkWidget   *ardesia_bar_window;
GtkWidget   *background_window;
GtkWidget   *annotation_window;
Workspace   *workspace;
CommandLine* commandline = NULL;


GdkRectangle* get_toolbar_area() {
    if ( commandline != NULL ) {
        if ( commandline->mode == DRAW_ON_MONITOR ) {
            Monitor* monitor = g_list_nth_data( workspace->monitors, commandline->tools_monitor );
            return monitor->rect;
        } else {
            //gtk_getet_display(commandline->clipRect->x, commandline->clipRect->y);
        }
    }
    return NULL;
}

/**
 * Get the drawable area for annotation, text and background windows
 * @return NULL if not set, GdkRectangle if it is
 */
GdkRectangle* get_drawable_area() {
    if ( commandline != NULL ) {
        if ( commandline->mode == DRAW_ON_MONITOR ) {
            if ( commandline->workspace_monitor < 0 || commandline->workspace_monitor >= g_list_length(workspace->monitors) ) {
                g_warning("Workspace monitor was given an illegal value, moving to monitor 0.\n");
                commandline->workspace_monitor = 0;
            }
            Monitor* monitor = g_list_nth_data( workspace->monitors, commandline->workspace_monitor );
            return monitor->rect;
        } else if ( commandline->mode == DRAW_ON_FULLDESKTOP ) {
            GdkWindow* rootwindow = gdk_screen_get_root_window( gdk_screen_get_default() );
            int maxwidth = gdk_window_get_width( rootwindow );
            int maxheight = gdk_window_get_height( rootwindow );
            commandline->clipRect->x = 0;
            commandline->clipRect->y = 0;
            commandline->clipRect->width = maxwidth;
            commandline->clipRect->height = maxheight;
            return commandline->clipRect;
        } else {
            // check clipRect bounds
            GdkWindow* rootwindow = gdk_screen_get_root_window( gdk_screen_get_default() );
            int maxwidth = gdk_window_get_width( rootwindow );
            int maxheight = gdk_window_get_height( rootwindow );
            g_printf("Maximum Size: %d %d\n", maxwidth, maxheight);
            if ( commandline->clipRect->x < 0 ) {
                commandline->clipRect->x = 0;
            }
            if ( commandline->clipRect->y < 0 ) {
                commandline->clipRect->y = 0;
            }
            if ( commandline->clipRect->x > maxwidth ) {
                commandline->clipRect->x = maxwidth;
            }
            if ( commandline->clipRect->x > maxheight ) {
                commandline->clipRect->y = maxheight;
            }
            return commandline->clipRect;
        }
    }
    return NULL;
}



#ifndef _WIN32

/* Call the dialog that inform the user to enable a composite manager. */
static void
run_missing_composite_manager_dialog   ()
{
  GtkWidget *msg_dialog;
  msg_dialog = gtk_message_dialog_new (NULL,
                                       GTK_DIALOG_MODAL,
                                       GTK_MESSAGE_ERROR,
                                       GTK_BUTTONS_OK,
                                       gettext ("In order to run Ardesia you need to enable a composite manager"));

  gtk_dialog_run (GTK_DIALOG (msg_dialog));

  if (msg_dialog != NULL)
    {
      gtk_widget_destroy (msg_dialog);
      msg_dialog = NULL;
    }

  exit (EXIT_FAILURE);
}


/* Check if a composite manager is active. */
static void
check_composite_manager      ()
{
  GdkDisplay *display = gdk_display_get_default ();
  GdkScreen  *screen  = gdk_display_get_default_screen (display);
  gboolean composite = gdk_screen_is_composited (screen);

  if (!composite)
    {
      /* start the enable composite manager dialog. */
      run_missing_composite_manager_dialog ();
    }

}

#endif


/* Enable the localization support with gettext. */
static void
enable_localization_support       ()
{
#ifdef ENABLE_NLS
  setlocale (LC_ALL, "");
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  textdomain (GETTEXT_PACKAGE);
#endif
}

void
build_annotation_window() {
    annotate_init (workspace->iwb_filename,
                    commandline->debug,
                    NULL);
    annotation_data->is_opaque = commandline->is_opaque;
    annotation_window = create_annotation_window();
    if (annotation_window == NULL)
    {
        annotate_quit ();
        g_free (commandline);
        exit (EXIT_FAILURE);
    }

    GdkRectangle* rect = get_drawable_area();
    position_annotation_window(rect->x,rect->y,rect->width,rect->height); //rect->width,rect->height);

    gtk_widget_show (annotation_window);
}

void
build_toolbar_window() {
    GdkRectangle* rect = get_toolbar_area();
    ardesia_bar_window = create_bar_window (commandline, rect, annotation_window);

    if (ardesia_bar_window == NULL)
      {
        annotate_quit ();
        //destroy_background_window ();
        g_free (commandline);
        exit (EXIT_FAILURE);
      }

    gtk_window_set_keep_above (GTK_WINDOW (ardesia_bar_window), TRUE);
    gtk_widget_show (ardesia_bar_window);


}



/* This is the starting point of the program. */
int
main                              (int    argc,
                                   char  *argv[])
{
  // make global variables NULL
  commandline               = (CommandLine *)   NULL;
  ardesia_bar_window        = (GtkWidget *)     NULL;
  background_window         = (GtkWidget *)     NULL;
  annotation_window         = (GtkWidget *)     NULL;
  workspace                 = (Workspace*)       NULL;


  /* Enable the localization support with gettext. */
  enable_localization_support ();

  // start GTK
  gtk_init (&argc, &argv);

#ifndef _WIN32
  check_composite_manager ();
#endif

  // handle command line
  commandline = create_command_line();
  parse_options(commandline, argc, argv);
  print_command_line( commandline );

  /* Initialize new text configuration options. */
  text_config = create_text_config();
  text_config->fontfamily = commandline->fontfamily;
  text_config->leftmargin = commandline->text_leftmargin;
  text_config->tabsize = commandline->text_tabsize;

  // handle workspace
  workspace = create_workspace();
  if (commandline->iwb_filename) {
      change_workspace_to( workspace, commandline->iwb_filename );
  }
  build_workspace_filesystem(workspace);

  // create windows, one for the drawing and one for the toolbar
  build_annotation_window();
  build_toolbar_window();

  // move tool bar to appropriate screen
  //Monitor* toolMonitor = g_list_nth_data( monitors, commandline->toolsMonitor - 1 );
  //gtk_window_move( GTK_WINDOW(ardesia_bar_window), toolMonitor->rect->x, 0 );

  replace_status_message(g_strdup_printf("Project started in %s", workspace->project_dir));

  create_text_settings_window();

  // main loop for a GTK application
  gtk_main ();

  destroy_workspace(workspace);
  destroy_command_line(commandline);

  return 0;
}

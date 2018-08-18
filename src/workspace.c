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

#include <workspace.h>
#include <utils.h>
#include <monitor.h>

Workspace*
create_workspace() {
    g_printf("Creating workspace\n");
    Workspace* workspace = g_malloc( (gsize) sizeof (Workspace) );
    workspace->monitors = NULL;
    set_defaults_for_workspace( workspace );
    return workspace;
}

void
set_defaults_for_workspace(Workspace* workspace) {
    g_printf("Setting workspace defaults\n");
    // gchar *workspace_dir = (gchar *) NULL;
    if ( workspace->monitors == NULL ) {
        workspace->monitors = create_monitor_list();
    }
    workspace->date = get_date ();

    /* Show the project name wizard. */
    //project_name = start_project_dialog ();
    workspace->project_name = g_strdup_printf ("ardesia_project_%s", workspace->date);
    workspace->workspace_dir = NULL;
    workspace->project_dir = NULL;
    workspace->iwb_filename = NULL;
    // workspace_dir = configure_workspace (project_name);
    // project_dir = create_default_project_dir (workspace_dir, project_name);
    // g_free (workspace_dir);
}

void
print_workspace(Workspace* workspace) {
    g_printf("Project Name: %s\n", workspace->project_name);
    g_printf("Project Directory: %s\n", workspace->project_dir);
    g_printf("Workspace Directory: %s\n", workspace->workspace_dir);
    g_printf("iwb filename: %s\n", workspace->iwb_filename);
    g_printf("Date: %s\n", workspace->date);
    print_monitor_list(workspace->monitors);
}

void
destroy_workspace(Workspace* workspace) {
    g_printf("Destroying workspace\n");
    if ( workspace->project_dir != NULL ) {
        remove_dir_if_empty(workspace->project_dir);
    }
    g_free(workspace->date);
    g_free(workspace->project_name);
    g_free(workspace->project_dir);
    g_free(workspace->workspace_dir);
    g_free(workspace->iwb_filename);
    destroy_monitor_list( workspace->monitors );
    g_free(workspace);
}



/* Create a shorcut to the workspace on the desktop.
gchar *workspace_dir
*/
static void
create_workspace_shortcut (Workspace* workspace)
{
  g_printf("Creating workspace shortcut\n");
  gchar *desktop_entry_filename = g_strdup_printf ("%s%s%s_workspace",
                                                    get_desktop_dir (),
                                                    G_DIR_SEPARATOR_S,
                                                    PACKAGE_NAME);

#ifdef _WIN32
  windows_create_link (workspace->workspace_dir,
                       desktop_entry_filename,
                       "%SystemRoot%\\system32\\imageres.dll",
                       123);

#else
  xdg_create_link (workspace->workspace_dir , desktop_entry_filename, "folder-documents");
#endif
  g_free (desktop_entry_filename);
}


/* Create the default project dir under the workspace_dir.
gchar  *workspace_dir,
gchar  *project_name
*/
static void
create_default_project_dir        (Workspace* workspace)
{
    g_printf("Creating project directory\n");
    if ( workspace->project_dir != NULL ) {
        g_free( workspace->project_dir );
        workspace->project_dir = NULL;
    }
    workspace->project_dir = g_build_filename (workspace->workspace_dir,
                                 workspace->project_name,
                                 (gchar *) 0);
    if (!file_exists (workspace->project_dir))
    {
        if (g_mkdir_with_parents (workspace->project_dir, 0700)==-1)
        {
            g_warning ("Unable to create folder %s\n", workspace->project_dir);
        }
    }
}


/* Configure the workspace.
gchar *project_name)
*/
void
configure_workspace               (Workspace* workspace)
{
  g_printf("Configuring workspace\n");
  if ( workspace->workspace_dir != NULL ) {
      g_free(workspace->workspace_dir);
      workspace->workspace_dir = NULL;
  }
  // why is this const?
  const gchar *documents_dir = get_documents_dir ();

  /* The workspace directory is in the documents ardesia folder. */
  workspace->workspace_dir = g_build_filename (documents_dir,
                                    PACKAGE_NAME,
                                    (gchar *) 0);


}

void
change_workspace_to(Workspace* workspace, gchar* filename ) {

    gint init_pos = -1;
    gint end_pos = -1;

    if (g_path_is_absolute (filename))
      {
        filename = g_strdup (filename);
      }
    else
      {
        gchar *dir = g_get_current_dir ();
        filename = g_build_filename (dir, filename, (gchar *) 0);
        g_free (dir);
      }

    if (!file_exists(filename))
      {
        printf("No such file %s\n", filename);
        exit (EXIT_FAILURE);
      }

      if ( workspace->iwb_filename != NULL ) {
          g_free( workspace->iwb_filename);
          workspace->iwb_filename = NULL;
      }
      workspace->iwb_filename = filename;

    init_pos = g_substrlastpos (filename, G_DIR_SEPARATOR_S);
    end_pos  = g_substrlastpos (filename, ".");

    if ( workspace->project_name != NULL ) {
        g_free( workspace->project_name );
        workspace->project_name = NULL;
    }
    workspace->project_name = g_substr (filename, init_pos+1, end_pos-1);
    if ( workspace->project_dir != NULL ) {
        g_free( workspace->project_dir );
        workspace->project_dir = NULL;
    }
    workspace->project_dir = g_substr (filename, 0, init_pos-1);

}

void
build_workspace_filesystem(Workspace * workspace)
{
    configure_workspace( workspace );
    create_workspace_shortcut (workspace);
    create_default_project_dir( workspace );
    print_workspace( workspace );
}

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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <utils.h>
#include <glib.h>
#include <gdk/gdk.h>
#ifdef _WIN32
#  include <windows_utils.h>
#endif
#include <annotation_window.h>
#include <bar.h>
#include <workspace.h>
#include <ardesia.h>

// /* The name of the current project. */
// static gchar *project_name = (gchar *) NULL;
//
// /* The name of the current project. */
// static gchar *project_dir = (gchar *) NULL;
//
// /* The name of the current project. */
// static gchar *iwb_filename = (gchar *) NULL;

/* The list of the artefacts created in the current session. */
static GSList *artifacts = (GSList *) NULL;

gboolean
intersect( GdkRectangle* a, GdkRectangle* b ) {
    return !( a->x + a->width < b->x ||
              a->x > b->x + b->width ||
              a->y + a->height < b->y ||
              a->y > b->y + b->height
          );
}

void
draw_test_square( cairo_t* context ) {
    cairo_t* cr = context;
    cairo_save(cr);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_line_width (cr, 10.0);
    cairo_set_source_rgb (cr, 255, 255, 255);
    cairo_rectangle (cr, 10, 10, 100, 100);
    cairo_stroke (cr);
    cairo_restore(cr);
}

void
draw_test_square_with_color( cairo_t* context, int r, int g, int b ) {
    cairo_t* cr = context;
    cairo_save(cr);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_line_width (cr, 10.0);
    cairo_set_source_rgb (cr, r, g, b );
    cairo_rectangle (cr, 10, 10, 100, 100);
    cairo_stroke (cr);
    cairo_restore(cr);
}

/* Get the name of the current project. */
gchar*
get_project_name   ()
{
  return workspace->project_name;
}


/* Set the name of the current project. */
void
set_project_name   (gchar  *name)
{
  workspace->project_name = name;
}


/* Get the dir of the current project. */
gchar*
get_project_dir    ()
{
  return workspace->project_dir;
}


/* Set the directory of the current project. */
void
set_project_dir    (gchar *dir)
{
  workspace->project_dir = dir;
}


/* Get the iwb file of the current project. */
gchar *
get_iwb_filename   ()
{
  return workspace->iwb_filename;
}


/* Set the iwb file of the current project. */
void
set_iwb_filename   (gchar *file)
{
  workspace->iwb_filename = file;
}


/* Get the list of the path of the artefacts created in the session. */
GSList *
get_artifacts      ()
{
  return artifacts;
}


/* Add the path of an artefacts created in the session to the list. */
void
add_artifact       (gchar *path)
{
  gchar *copied_path = g_strdup_printf ("%s", path);
  artifacts = g_slist_prepend (artifacts, copied_path);
}


/* Free the structure containing the artefact list created in the session. */
void
free_artifacts     ()
{
  g_slist_foreach (artifacts, (GFunc)g_free, NULL);
}


/* Get the bar window widget. */
GtkWidget *
get_bar_widget ()
{
  return GTK_WIDGET (gtk_builder_get_object (bar_gtk_builder, BAR_WIDGET_NAME));
}


/** Get the distance between two points. */
gdouble
get_distance       (gdouble x1,
                    gdouble y1,
                    gdouble x2,
                    gdouble y2)
{
  /* Apply the Pitagora theorem to calculate the distance. */
  gdouble x_delta = fabs (x2-x1);
  gdouble y_delta = fabs (y2-y1);
  gdouble quad_sum = pow (x_delta, 2);
  quad_sum = quad_sum + pow (y_delta, 2);
  return sqrt (quad_sum);
}


/* Take a GdkColor and return the equivalent RGBA string. */
gchar *
gdkcolor_to_rgb    (GdkRGBA *gdkcolor)
{
  /* Transform in the  RGB format e.g. FF0000. */
  gchar *ret_str = g_strdup_printf ("%02X%02X%02X",
                                    (int) gdkcolor->red/255,
                                    (int) gdkcolor->green/255,
                                    (int) gdkcolor->blue/255);

  return ret_str;
}

gchar *
gdkrgba_to_rgba    (GdkRGBA *gdkcolor)
{
  /* Transform in the  RGB format e.g. FF0000. */
  gchar *ret_str = g_strdup_printf ("%02X%02X%02X%02X",
                                    (int) (gdkcolor->red * 255),
                                    (int) (gdkcolor->green * 255),
                                    (int) (gdkcolor->blue * 255),
                                    (int) (gdkcolor->alpha * 255)
                            );

  return ret_str;
}


/*
 * Take an rgb or a rgba string and return the pointer to the allocated GdkColor
 * neglecting the alpha channel; the gtkColor does not support the rgba color.
 */
GdkRGBA *
rgba_to_gdkcolor   (gchar  *rgba)
{
    assert(rgba != NULL );
    assert(strlen(rgba)==8);

    int br=0,bg=0,bb=0,ba=0;
    sscanf( rgba, "%02X%02X%02X%02X", &br, &bg, &bb, &ba);
    GdkRGBA *gdkcolor = g_malloc ( (gsize) sizeof (GdkRGBA));
    gdkcolor->red = (double) br / 255;
    gdkcolor->green = (double) bg / 255;
    gdkcolor->blue = (double) bb / 255;
    gdkcolor->alpha = (double) ba / 255;
    return gdkcolor;
}


/* Clear cairo context. */
void
clear_cairo_context     (cairo_t  *cr)
{
  if (cr)
    {
      cairo_save (cr); // without this the tool icon disappears
      cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
      cairo_paint (cr);
      cairo_restore (cr);
    }
}


/* Scale the surface with the width and height requested */
cairo_surface_t *
scale_surface      (cairo_surface_t  *surface,
                    gdouble           width,
                    gdouble           height)
{
  gdouble old_width = cairo_image_surface_get_width (surface);
  gdouble old_height = cairo_image_surface_get_height (surface);

  cairo_surface_t *new_surface = cairo_surface_create_similar (surface,
                                                               CAIRO_CONTENT_COLOR_ALPHA,
                                                               width,
                                                               height);

  cairo_t *cr = cairo_create (new_surface);

  /* Scale *before* setting the source surface (1) */
  cairo_scale (cr, width / old_width, height / old_height);
  cairo_set_source_surface (cr, surface, 0, 0);

  /* To avoid getting the edge pixels blended with 0 alpha, which would
   * occur with the default EXTEND_NONE. Use EXTEND_PAD for 1.2 or newer (2)
   */
  cairo_pattern_set_extend (cairo_get_source(cr), CAIRO_EXTEND_REFLECT);

  /* Replace the destination with the source instead of overlaying */
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);

  /* Do the actual drawing */
  cairo_paint (cr);

  cairo_destroy (cr);

  return new_surface;
}


/* Set the cairo surface colour to the RGBA string. */
void
cairo_set_source_color_from_string     (cairo_t  *cr,
                                        gchar    *color)
{
  if (cr)
    {
      guint r,g,b,a;
      sscanf (color, "%02X%02X%02X%02X", &r, &g, &b, &a);

      cairo_set_source_rgba (cr,
                             1.0 * r / 255,
                             1.0 * g / 255,
                             1.0 * b / 255,
                             1.0 * a / 255);

    }
}


/* Save the contents of the pixbuf in the file with name file name. */
gboolean
save_pixbuf_on_png_file      (GdkPixbuf    *pixbuf,
                              const gchar  *filename)
{
  gint width = gdk_pixbuf_get_width (pixbuf);
  gint height = gdk_pixbuf_get_height (pixbuf);

  cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                                         width,
                                                         height);

  cairo_t *cr = cairo_create (surface);
  gdk_cairo_set_source_pixbuf (cr, pixbuf, 0, 0);
  cairo_paint (cr);

  /* Write to the png surface. */
  cairo_surface_write_to_png (surface, filename);
  cairo_destroy (cr);
  cairo_surface_destroy (surface);
  return TRUE;
}

gboolean
is_bar_window_over_annotation_window() {
    GtkWidget *bar= get_bar_widget ();
    gint x, y, width, height;

    gtk_window_get_position (GTK_WINDOW (bar), &x, &y);
    gtk_window_get_size (GTK_WINDOW (bar), &width, &height);

    GdkRectangle* rB = g_new( GdkRectangle, 1 );
    rB->x = x;
    rB->y = y;
    rB->width = width;
    rB->height = height;

    GtkWidget* widget = annotation_data->annotation_window;
    gint ann_width=0, ann_height=0, ann_x=0, ann_y=0;
    gtk_window_get_position (GTK_WINDOW (widget), &ann_x, &ann_y);
    gtk_window_get_size (GTK_WINDOW (widget), &ann_width, &ann_height);

    GdkRectangle* rA = g_new( GdkRectangle, 1 );
    rA->x = ann_x;
    rA->y = ann_y;
    rA->width = ann_width;
    rA->height = ann_height;

    gboolean result = intersect( rA, rB );
    g_free(rA);
    g_free(rB);
    return result;
}

GdkPixbuf*
take_screenshot_now() {
    GtkWidget* widget = annotation_data->annotation_window;
    gint ann_width=0, ann_height=0, ann_x=0, ann_y=0;
    gtk_window_get_position (GTK_WINDOW (widget), &ann_x, &ann_y);
    gtk_window_get_size (GTK_WINDOW (widget), &ann_width, &ann_height);

    GdkWindow *root_window = gdk_get_default_root_window ();

    GdkPixbuf* snapshot =  gdk_pixbuf_get_from_window (root_window,
                                     ann_x,
                                     ann_y,
                                     ann_width,
                                     ann_height);
    return snapshot;
}

/* Grab the screenshoot and put it in the image buffer. */
void
grab_screenshot    (void (*screenshot_callback) ( GdkPixbuf*))
{
g_printf("grab_snapshot\n");
    // check if tool bar and annotation bar intersect
    if ( is_bar_window_over_annotation_window() ) {
        // this generates a window state change once the window has been hidden
        bar_data->screenshot_pending = TRUE;
        bar_data->screenshot_callback = screenshot_callback;
        int x=0, y=0;
        gdk_window_get_root_origin( gtk_widget_get_window(get_bar_widget()), &x, &y);
        bar_data->screenshot_saved_location_x = x;
        bar_data->screenshot_saved_location_y = y;
        g_printf("hiding bar\n");
        int width = gtk_widget_get_allocated_width( get_bar_widget() );
        gdk_window_move( gtk_widget_get_window(get_bar_widget()), -width-500, 0);
        gtk_widget_hide( get_bar_widget() );
    } else {
        GdkPixbuf* buffer =  take_screenshot_now();
        screenshot_callback( buffer );
    }
}


/*
 * This is function return if the point (x,y) in inside the ardesia bar window.
 */
gboolean
inside_bar_window       (gdouble xp,
                         gdouble yp)
{
  gint x = 0;
  gint y = 0;
  gint width = 0;
  gint height = 0;
  gdouble xd = 0;
  gdouble yd = 0;
  GtkWindow *bar_window = GTK_WINDOW (get_bar_widget ());
  gtk_window_get_position (bar_window, &x, &y);
  xd = (gdouble) x;
  yd = (gdouble) y;

  gtk_window_get_size (bar_window, &width, &height);

  if ( (yp>=yd) && (yp<yd+height) )
    {

      if ( (xp>=xd) && (xp<xd+width) )
        {
          return TRUE;
        }

    }

  return FALSE;
}


/* Drill the window in the area where the ardesia bar is located.
 * This is the bit that should allow mouse signals to get through to the bar.
 */
void
drill_window_in_bar_area     (GtkWidget* layer, GtkWidget* window)
{
  GtkWidget *bar= window;
  gint x, y, width, height;

  gtk_window_get_position (GTK_WINDOW (bar), &x, &y);
  gtk_window_get_size (GTK_WINDOW (bar), &width, &height);

  GdkRectangle* rB = g_new( GdkRectangle, 1 );
  rB->x = x;
  rB->y = y;
  rB->width = width;
  rB->height = height;

  gint ann_width=0, ann_height=0, ann_x=0, ann_y=0;
  gtk_window_get_position (GTK_WINDOW (layer), &ann_x, &ann_y);
  gtk_window_get_size (GTK_WINDOW (layer), &ann_width, &ann_height);

  GdkRectangle* rA = g_new( GdkRectangle, 1 );
  rA->x = ann_x;
  rA->y = ann_y;
  rA->width = ann_width;
  rA->height = ann_height;

  if ( intersect( rA, rB ) == TRUE ) {
      g_free( rB );
      rB = NULL;

      // bar
      const cairo_rectangle_int_t widget_rect = { x+1, y+1, width-1, height-1 };
      cairo_region_t *widget_reg = cairo_region_create_rectangle (&widget_rect);
      // annotation window
      const cairo_rectangle_int_t ann_rect = { 0, 0, ann_width, ann_height };
      cairo_region_t *ann_reg = cairo_region_create_rectangle (&ann_rect);

      cairo_region_subtract (ann_reg, widget_reg);
      cairo_region_destroy (widget_reg);

      // we also want to drill down other windows if they too are visible
      if ( annotation_data->background_selection_window != NULL ) {
          g_printf("drilling hole for background selection window\n");
          gtk_window_get_position (GTK_WINDOW (annotation_data->background_selection_window), &x, &y);
          gtk_window_get_size (GTK_WINDOW (annotation_data->background_selection_window), &width, &height);
          rB = g_new( GdkRectangle, 1 );
          rB->x = x;
          rB->y = y;
          rB->width = width;
          rB->height = height;
          if ( intersect( rA, rB ) == TRUE ) {
              g_printf("drilling hole for background selection window - intersect was true\n");
              cairo_rectangle_int_t widget_rect2 = { x+1, y+1, width-1, height-1 };
              widget_reg = cairo_region_create_rectangle (&widget_rect2);
              cairo_region_subtract (ann_reg, widget_reg);
              cairo_region_destroy (widget_reg);
          }
          g_free( rB );
          rB = NULL;
      }
      if ( annotation_data->font_window != NULL ) {
          g_printf("drilling hole for font window\n");
          gtk_window_get_position (GTK_WINDOW (annotation_data->font_window), &x, &y);
          gtk_window_get_size (GTK_WINDOW (annotation_data->font_window), &width, &height);
          rB = g_new( GdkRectangle, 1 );
          rB->x = x;
          rB->y = y;
          rB->width = width;
          rB->height = height;
          if ( intersect( rA, rB ) == TRUE ) {
              g_printf("drilling hole for background selection window - intersect was true\n");
              cairo_rectangle_int_t widget_rect3 = { x+1, y+1, width-1, height-1 };
              widget_reg = cairo_region_create_rectangle (&widget_rect3);
              cairo_region_subtract (ann_reg, widget_reg);
              cairo_region_destroy (widget_reg);
          }
          g_free( rB );
          rB = NULL;
      }


      // drill with input shape the pointer will go below the window.
      // @TODO this is not additive currently so we need to keep a current region
      gtk_widget_input_shape_combine_region (layer, ann_reg);

      // drill with shape; the area will be transparent.
      // this is currently doing nothing useful.
      //gtk_widget_shape_combine_region (layer, ann_reg);
      //gdk_window_shape_combine_region ( gtk_widget_get_window(layer), ann_reg, 0, 0);

      cairo_region_destroy (ann_reg);

  }
  g_free( rA );
  if ( rB != NULL ) {
      g_free( rB );
  }

}


/*
 * Get the current date and format in a printable format;
 * the returned value must be free with the g_free.
 */
gchar *
get_date      ()
{
  struct tm *t;
  time_t now;
  // gchar *time_sep = ":";
  gchar *date = "";

  time (&now);
  t = localtime (&now);

// #ifdef _WIN32
//   /* The ":" character on windows is avoided in file name and then
//    * I use the "." character instead.
//    */
//   time_sep = ".";
// #endif

  char buffer[1024];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H%M%S", t);

  // date = g_strdup_printf ("%d-%d-%d_%d%s%d%s%d",
  //                         t->tm_year+1900,
  //                         t->tm_mday,
  //                         t->tm_mon+1 < 10 ? t->tm_mon+1 : ,
  //                         t->tm_hour,
  //                         time_sep,
  //                         t->tm_min,
  //                         time_sep,
  //                         t->tm_sec);
  date = g_strdup_printf("%s", buffer);

  return date;
}


/* Return if a file exists. */
gboolean
file_exists        (gchar *filename)
{
  return g_file_test (filename, G_FILE_TEST_EXISTS);
}


/*
 * Return a file name containing
 * the project name and the current date.
 */
gchar *
get_default_filename    ()
{
  gchar *date = get_date ();
  gchar *filename = g_strdup_printf ("%s_%s", workspace->project_name, date);
  g_free (date);

  return filename;
}


/*
 * Get the home directory.
 */
const gchar *
get_home_dir       (void)
{
  const char *homedir = g_getenv ("HOME");
  if (! homedir)
    {
      homedir = g_get_home_dir ();
    }
  return homedir;
}


/*
 * Get the desktop directory.
 */
const gchar *
get_desktop_dir    (void)
{
  return g_get_user_special_dir (G_USER_DIRECTORY_DESKTOP);
}


/*
 * Get the documents directory.
 */
const gchar *
get_documents_dir  (void)
{
  const gchar *documents_dir = g_get_user_special_dir (G_USER_DIRECTORY_DOCUMENTS);
  if (documents_dir == NULL)
    {
       documents_dir = get_home_dir ();
    }
  return documents_dir;
}


/* Delete a directory recursively. */
void
rmdir_recursive    (gchar *path)
{
  GDir *cur_dir;
  const gchar *dir_file;

  cur_dir = g_dir_open (path, 0, NULL);

  if (cur_dir)
    {
      while ( (dir_file = g_dir_read_name (cur_dir)))
        {
          gchar *fpath = g_build_filename (path, dir_file, NULL);

          if (fpath)
            {
              if (g_file_test (fpath, G_FILE_TEST_IS_DIR))
                {
                  rmdir_recursive (fpath);
                }
              else
                {
                  g_unlink (fpath);
                }
              g_free (fpath);
            }
        }

      g_dir_close (cur_dir);
    }

  g_rmdir (path);
}


/* Remove directory if it is empty */
void
remove_dir_if_empty     (gchar* dir_path)
{
  GDir        *dir = (GDir *) NULL;
  gint         file_occurrence = 0;

  /* if the project dir is empty delete it */
  dir  = g_dir_open (dir_path, 0, NULL);

  while (g_dir_read_name (dir))
    {
      file_occurrence++;
    }

  if (file_occurrence == 0)
    {
      rmdir_recursive (dir_path);
    }
}


/* Allocate a new point belonging to the stroke passing the values. */
AnnotatePoint *
allocate_point     (gdouble  x,
                    gdouble  y,
                    gdouble  width,
                    gdouble  pressure)
{
  AnnotatePoint *point =  g_malloc ( (gsize) sizeof (AnnotatePoint));
  point->x = x;
  point->y = y;
  point->width = width;
  point->pressure = pressure;
  return point;
}


/* Send an email. */
void
send_email         (gchar   *to,
                    gchar   *subject,
                    gchar   *body,
                    GSList  *attachment_list)
{
#ifdef _WIN32
  windows_send_email (to, subject, body, attachment_list);
#else

  guint attach_lenght = g_slist_length (attachment_list);
  guint i = 0;

  gchar* mailer = "xdg-email";
  gchar* subject_param = "--subject";
  gchar* body_param = "--body";
  gchar* attach_param = "--attach";

  gchar* args = g_strdup_printf ("%s %s %s %s '%s'", mailer, subject_param, subject, body_param, body);

  for (i=0; i<attach_lenght; i++)
    {
      gchar* attachment = (gchar*) g_slist_nth_data (attachment_list, i);
      gchar* attachment_str = g_strdup_printf ("%s '%s'", attach_param, attachment);
      gchar* new_args = g_strdup_printf ("%s %s", args, attachment_str);
      g_free (args);
      args = new_args;
      g_free (attachment_str);
    }

  gchar* new_args = g_strdup_printf ("%s %s&", args, to);
  g_free (args);

  if (system (new_args)<0)
    {
      g_warning ("Problem running command: %s", new_args);
    }

  g_free (new_args);

#endif
}


/* Send artifacts with email. */
void
send_artifacts_with_email    (GSList *attachment_list)
{
  gchar *to = "ardesia-developer@googlegroups.com";
  gchar *subject = "ardesia-contribution";
  gchar *body = g_strdup_printf ("%s,\n%s,%s.",
				 "Dear ardesia developer group",
				 "I want share my work created with Ardesia with you",
				 "please for details see the attachment" );

  send_email (to, subject, body, attachment_list);
  g_free (body);
}


/* Send trace with email. */
void
send_trace_with_email        (gchar *attachment)
{
  GSList *attachment_list = NULL;
  gchar *to = "ardesia-developer@googlegroups.com";
  gchar *subject = "ardesia-bug-report";

  gchar *body = g_strdup_printf ("%s,\n%s,%s.",
                                 "Dear ardesia developer group",
                                 "An application error occurred",
                                 "please for details see the attachment with the stack trace" );

  attachment_list = g_slist_prepend (attachment_list, attachment);
  send_email (to, subject, body, attachment_list);
  g_free (body);
}


/* Is the desktop manager gnome. */
gboolean
is_gnome           ()
{
#ifdef _WIN32
  return FALSE;
#endif

  gchar *current_desktop = getenv ("XDG_CURRENT_DESKTOP");
  if (current_desktop)
    {
      if (strcmp (current_desktop, "GNOME")!=0)
        {
          return FALSE;
        }
    }
  return TRUE;
}


/* Create desktop entry passing value. */
void
xdg_create_desktop_entry      (gchar  *filename,
                               gchar  *type,
                               gchar  *name,
                               gchar  *icon,
                               gchar  *exec)
{
  FILE *fp = fopen (filename, "w");
  if (fp)
    {
      fprintf (fp, "[Desktop Entry]\n");
      fprintf (fp, "Type=%s\n", type);
      fprintf (fp, "Name=%s\n", name);
      fprintf (fp, "Icon=%s\n", icon);
      fprintf (fp, "Exec=%s", exec);
      fclose (fp);
      chmod (filename, 0751);
    }
}


/* Create a desktop link. */
void
xdg_create_link         (gchar  *src,
                         gchar  *dest,
                         gchar  *icon)
{
  gchar *link_extension = "desktop";
  gchar *link_filename = g_strdup_printf ("%s.%s", dest, link_extension);

  if (! g_file_test (link_filename, G_FILE_TEST_EXISTS))
    {
       gchar *exec = g_strdup_printf ("xdg-open %s\n", src);
       xdg_create_desktop_entry (link_filename, "Application", PACKAGE_NAME, icon, exec);
       g_free (exec);
    }

  g_free (link_filename);
}


/* Get the last position where sub-string occurs in the string. */
gint
g_substrlastpos         (const char *str,
                         const char *substr)
{
  gint len = (gint) strlen (str);
  gint i = 0;

  for (i = len-1; i >= 0; --i)
    {

      if (str[i] == *substr)
        {
          return i;
        }

    }
  return -1;
}


/* Sub-string of string from start to end position. */
gchar *
g_substr           (const gchar *string,
                    gint         start,
                    gint         end)
{
  gint number_of_char = (end - start + 1);
  gsize size = (gsize) sizeof (gchar) * number_of_char;
  return g_strndup (&string[start], size);
}


/*
 * This function create a segmentation fault;
 * it is useful to test the segmentation fault handler.
 */
void create_segmentation_fault    ()
{
  int *f=NULL;
  *f = 0;
}


void get_surface_size (cairo_surface_t *surface,
			       int *width, int *height)
	{
		cairo_t *cr;
		double x1, x2, y1, y2;

		cr = cairo_create (surface);
		cairo_clip_extents (cr, &x1, &y1, &x2, &y2);
		cairo_destroy (cr);

		*width = x2 - x1;
		*height = y2 - y1;
	}

void get_context_size (cairo_t *cr,
			       int *width, int *height)
	{
		double x1, x2, y1, y2;

		cairo_clip_extents (cr, &x1, &y1, &x2, &y2);

		*width = x2 - x1;
		*height = y2 - y1;
	}


// e.g. save_cairo_context( text_data->cr, "/tmp", "text", 1);
void
save_cairo_context( cairo_t* cr, gchar* savedir, gchar* category, int index ) {
    gchar* filename = g_strdup_printf ("%s%s%s_%s_%d_vellum.png",
                                            savedir,
                                            G_DIR_SEPARATOR_S,
                                            PACKAGE_NAME,
                                            category,
                                            index);

    int w; int h;
    get_context_size( cr, &w, &h );

    /* Load a surface with the data->annotation_cairo_context content and write the file. */
    cairo_surface_t* saved_surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                                w,
                                                h);

    cairo_surface_t* source_surface = cairo_get_target (cr);
    cairo_t* dest_cr = cairo_create (saved_surface);
    cairo_set_source_surface (dest_cr, source_surface, 0, 0);
    cairo_paint (dest_cr);
    /* Postcondition: the saved_surface now contains the save-point image. */


    /*  Will be create a file in the save-point folder with format PACKAGE_NAME_1.png. */
    cairo_surface_write_to_png (saved_surface, filename);
    cairo_surface_destroy (saved_surface);
    cairo_destroy (dest_cr);
    g_printf("Saving cairo context image to %s\n", filename);
    g_free( filename );

}

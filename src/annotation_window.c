/*
 * Ardesia-- a program for painting on the screen
 * Copyright (C) 2009 Pilolli Pietro <pilolli.pietro@gmail.com>
 *
 * Ardesia is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Ardesia is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY;without even the implied warranty of
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


#include <annotation_window.h>
#include <annotation_window_callbacks.h>
#include <utils.h>
#include <input.h>
#include <broken.h>
#include <bar_callbacks.h>
#include <bezier_spline.h>
#include <cursors.h>
#include <iwb_loader.h>
#include <fill.h>
#include <background_window.h>
#include <text_window.h>
#include <cairo_functions.h>

#ifdef _WIN32
#  include <windows_utils.h>
#endif


/* Internal data for the annotation window. */
AnnotateData* annotation_data;

/* Return the pressure passing the event. */
static gdouble
get_pressure       (GdkEvent *ev)
{
  gdouble ret_value = 1.0;
  gdouble pressure = ret_value;

  gboolean ret = gdk_event_get_axis (ev, GDK_AXIS_PRESSURE, &pressure);

  if (ret)
    {
      ret_value = pressure;
    }

  return ret_value;
}

/* Create a new paint context. */
static AnnotatePaintContext *
annotate_paint_context_new        (AnnotatePaintType type)
{
  AnnotatePaintContext *context = (AnnotatePaintContext *) NULL;
  context = g_malloc ((gsize) sizeof (AnnotatePaintContext));
  context->type = type;

  return context;
}


/* Calculate the direction in radiant. */
static gdouble
annotate_get_arrow_direction      (AnnotateDeviceData *devdata)
{
  /* Precondition: the list must be not null and the length might be greater than two. */
  AnnotatePoint *point = (AnnotatePoint *) NULL;
  AnnotatePoint *old_point = (AnnotatePoint *) NULL;
  gdouble delta = 2.0;
  gdouble ret = 0.0;
  GSList *out_ptr = devdata->coord_list;
  gdouble tollerance = annotate_get_thickness () * delta;

  /* Build the relevant point list with the standard deviation algorithm. */
  GSList *relevantpoint_list = build_meaningful_point_list (out_ptr, FALSE, tollerance);

  old_point = (AnnotatePoint *) g_slist_nth_data (relevantpoint_list, 1);
  point = (AnnotatePoint *) g_slist_nth_data (relevantpoint_list, 0);
  /* Give the direction using the last two point. */
  ret = atan2 (point->y-old_point->y, point->x-old_point->x);

  /* Free the relevant point list. */
  g_slist_foreach (relevantpoint_list, (GFunc) g_free, (gpointer) NULL);
  g_slist_free (relevantpoint_list);
  relevantpoint_list = (GSList *) NULL;

  return ret;
}


/* Colour selector; if eraser than select the transparent colour else allocate the right colour. */
static void
select_color            ()
{
  if (!annotation_data->annotation_cairo_context)
    {
      return;
    }

  if (annotation_data->cur_context)
    {
      if (annotation_data->cur_context->type != ANNOTATE_ERASER) //pen or arrow tool
        {
          /* Select the colour. */
          if (annotation_data->color)
            {
              if (annotation_data->debug)
                {
                  g_printerr ("Select colour %s\n", annotation_data->color);
                }

              cairo_set_source_color_from_string (annotation_data->annotation_cairo_context,
                                                  annotation_data->color);
            }

          cairo_set_operator (annotation_data->annotation_cairo_context, CAIRO_OPERATOR_SOURCE);
        }
      else
        {

          /* It is the eraser tool. */
          if (annotation_data->debug)
            {
              g_printerr ("Select transparent colour to erase\n");
            }

          cairo_set_operator (annotation_data->annotation_cairo_context, CAIRO_OPERATOR_CLEAR);
        }
    }
}


#ifdef _WIN32


/* Acquire the grab pointer. */
static void
annotate_acquire_pointer_grab ()
{
  grab_pointer (annotation_data->annotation_window, GDK_ALL_EVENTS_MASK);
}


/* Release the grab pointer. */
static void
annotate_release_pointer_grab     ()
{
  ungrab_pointer (gdk_display_get_default ());
}

#endif


/* Update the cursor icon. */
static void
update_cursor      ()
{
  if (!annotation_data->annotation_window)
    {
      return;
    }

#ifdef _WIN32
  annotate_release_pointer_grab ();
#endif

  gdk_window_set_cursor (gtk_widget_get_window (annotation_data->annotation_window), annotation_data->cursor);

#ifdef _WIN32
  annotate_acquire_pointer_grab ();
#endif
}


/* Dis-allocate cursor. */
static void
disallocate_cursor     ()
{
  if (annotation_data->cursor)
    {
      g_object_unref (annotation_data->cursor);
      annotation_data->cursor = (GdkCursor *) NULL;
    }
}


/* Take the input mouse focus. */
static void
annotate_acquire_input_grab  ()
{
#ifdef _WIN32
  grab_pointer (annotation_data->annotation_window, GDK_ALL_EVENTS_MASK);
#endif

#ifndef _WIN32
  /*
   * MACOSX; will do nothing.
   */
  gtk_widget_input_shape_combine_region (annotation_data->annotation_window, NULL);
  drill_window_in_bar_area (annotation_data->annotation_window);
#endif

}


/* Destroy cairo context. */
static void
destroy_cairo           (cairo_t* ctxt)
{
  guint refcount =  (guint) cairo_get_reference_count (ctxt);

  guint i = 0;

  for  (i=0; i<refcount; i++)
    {
      cairo_destroy (ctxt);
    }

  ctxt = (cairo_t *) NULL;
}


/* This an ellipse taking the top left edge coordinates
 * and the width and the height of the bounded rectangle.
 */
static void
annotate_draw_ellipse   (AnnotateDeviceData *devdata,
                         gdouble x,
                         gdouble y,
                         gdouble width,
                         gdouble height,
                         gdouble pressure)
{
  if (annotation_data->debug)
    {
      g_printerr ("Draw ellipse: 2a=%f 2b=%f\n", width, height);
    }

  annotate_modify_color (devdata, annotation_data, pressure);

  cairo_save (annotation_data->annotation_cairo_context);

  /* The ellipse is done as a 360 degree arc translated. */
  cairo_translate (annotation_data->annotation_cairo_context, x + width / 2., y + height / 2.);
  cairo_scale (annotation_data->annotation_cairo_context, width / 2., height / 2.);
  cairo_arc (annotation_data->annotation_cairo_context, 0., 0., 1., 0., 2 * M_PI);
  cairo_restore (annotation_data->annotation_cairo_context);

}


/* Draw a curve using a cubic bezier splines passing to the list's coordinate. */
static void
annotate_draw_curve    (AnnotateDeviceData *devdata,
                        GSList             *list)
{
  guint length = g_slist_length (list);

  if (list)
    {
      guint i = 0;
      for (i=0; i<length; i=i+3)
        {
          AnnotatePoint *first_point = (AnnotatePoint *) g_slist_nth_data (list, i);
          if (!first_point)
            {
              return;
            }
          if (length == 1)
            {
              /* It is a point. */
              annotate_draw_point (devdata, first_point->x, first_point->y, first_point->pressure);
            }
          else
            {
              AnnotatePoint *second_point = (AnnotatePoint *) g_slist_nth_data (list, i+1);
              if (!second_point)
                {
                  return;
                }
              else
                {
                  AnnotatePoint *third_point = (AnnotatePoint *) g_slist_nth_data (list, i+2);
                  if (!third_point)
                    {
                      /* draw line from first to second point */
                      annotate_draw_line (devdata, second_point->x, second_point->y, FALSE);
                      return;
                    }
                  annotate_modify_color (devdata, annotation_data, second_point->pressure);
                  cairo_curve_to (annotation_data->annotation_cairo_context,
                                  first_point->x,
                                  first_point->y,
                                  second_point->x,
                                  second_point->y,
                                  third_point->x,
                                  third_point->y);
                }
            }
        }
    }
}


/* Rectify the line. */
static void
rectify            (AnnotateDeviceData *devdata,
                    gboolean            closed_path)
{
  gdouble tollerance = annotate_get_thickness ();
  GSList *broken_list = broken (devdata->coord_list, closed_path, TRUE, tollerance);

  if (annotation_data->debug)
    {
      g_printerr ("rectify\n");
    }

  /* Restore the surface without the last path handwritten. */
  annotate_restore_surface ();

  annotate_draw_point_list (devdata, broken_list);

  annotate_coord_dev_list_free (devdata);
  devdata->coord_list = broken_list;

}


/* Roundify the line. */
static void
roundify           (AnnotateDeviceData *devdata,
                    gboolean            closed_path)
{
  gdouble tollerance = annotate_get_thickness ();

  /* Build the meaningful point list with the standard deviation algorithm. */
  GSList *meaningful_point_list = (GSList *) NULL;

  /* Restore the surface without the last path handwritten. */
  annotate_restore_surface ();

  meaningful_point_list = build_meaningful_point_list (devdata->coord_list, closed_path, tollerance);

  if ( g_slist_length (meaningful_point_list) < 4)
    {
      /* Draw the point line as is and jump the bezier algorithm. */
      annotate_draw_point_list (devdata, meaningful_point_list);
    }
  else if ((closed_path) && (is_similar_to_an_ellipse (meaningful_point_list, tollerance)))
    {
      GSList *rect_list = build_outbounded_rectangle (meaningful_point_list);

      if (rect_list)
        {
          AnnotatePoint *point1 = (AnnotatePoint *) g_slist_nth_data (rect_list, 0);
          AnnotatePoint *point2 = (AnnotatePoint *) g_slist_nth_data (rect_list, 1);
          AnnotatePoint *point3 = (AnnotatePoint *) g_slist_nth_data (rect_list, 2);
          gdouble p1p2 = get_distance(point1->x, point1->y, point2->x, point2->y);
          gdouble p2p3 = get_distance(point2->x, point2->y, point3->x, point3->y);
          gdouble e_threshold = 0.5;
          gdouble a = 0;
          gdouble b = 0;
          if (p1p2>p2p3)
            {
              b = p2p3/2;
              a = p1p2/2;
            }
          else
            {
              a = p2p3/2;
              b = p1p2/2;
            }
          gdouble e = 1-powf((b/a), 2);
          /* If the eccentricity is roundable to 0 it is a circle */
          if ((e >= 0) && (e <= e_threshold))
            {
              /* Move the down right point in the right position to square the circle */
              gdouble quad_distance = (p1p2+p2p3)/2;
              point3->x = point1->x+quad_distance;
              point3->y = point1->y+quad_distance;
            }

          annotate_draw_ellipse (devdata, point1->x, point1->y, point3->x-point1->x, point3->y-point1->y, point1->pressure);
          g_slist_foreach (rect_list, (GFunc)g_free, NULL);
          g_slist_free (rect_list);
        }
    }

  else
    {
      /* It is not an ellipse; I use bezier to spline the path. */
      GSList *splined_list = spline (meaningful_point_list);
      annotate_draw_curve (devdata, splined_list);

      annotate_coord_dev_list_free (devdata);
      devdata->coord_list = splined_list;

    }

  g_slist_foreach (meaningful_point_list, (GFunc) g_free, (gpointer) NULL);
  g_slist_free (meaningful_point_list);
}


/* Create the annotation window. */
GtkWidget *
create_annotation_window     ()
{
  GtkWidget* widget = (GtkWidget *) NULL;
  GError* error = (GError *) NULL;

  /* Initialize the main window. */
  annotation_data->annotation_window_gtk_builder = gtk_builder_new ();

  /* Load the gtk builder file created with glade. */
  gtk_builder_add_from_file (annotation_data->annotation_window_gtk_builder, ANNOTATION_UI_FILE, &error);

  if (error)
    {
      g_warning ("Failed to load builder file: %s", error->message);
      g_error_free (error);
      return widget;
    }

  widget = GTK_WIDGET (gtk_builder_get_object (annotation_data->annotation_window_gtk_builder,
                       "annotationWindow"));


  annotation_data->annotation_window = widget;
  if (annotation_data->annotation_window == NULL)
    {
      g_warning ("Failed to create the annotation window");
      return NULL;
    }


    /* Connect all the callback from gtkbuilder xml file. */
    gtk_builder_connect_signals (annotation_data->annotation_window_gtk_builder, (gpointer) annotation_data);

    /* Connect some extra callbacks in order to handle the hotplugged input devices. */
    // after GDK 3.2 the GdkSeat has the device-added and device-removed signals
    GdkSeat* seat = gdk_display_get_default_seat(gdk_display_get_default ());

    g_signal_connect (seat,
                      "device-added",
                      G_CALLBACK (on_device_added),
                      annotation_data);
    g_signal_connect (seat,
                      "device-removed",
                      G_CALLBACK (on_device_removed),
                      annotation_data);

  return widget;
}


static void
make_annotation_window_transparent() {
    if ( annotation_data->is_opaque  == FALSE ) {
        /* This trys to set an alpha channel. */
        on_screen_changed (annotation_data->annotation_window, NULL, annotation_data);

        /* Put the opacity to 0 to avoid the initial flickering. */
        gtk_widget_set_opacity (annotation_data->annotation_window, 0.01);

#ifdef _WIN32
      /* @TODO Use RGBA colormap and avoid to use the layered window. */
      /* I use a layered window that use the black as transparent colour. */
      setLayeredGdkWindowAttributes (gtk_widget_get_window (annotation_data->annotation_window),
                                     RGB (0,0,0),
                                     0,
                                     LWA_COLORKEY);
#endif
    }
}

/* Set-up the application. */
void
position_annotation_window          ( int x, int y, int width, int height )
{
    if ( annotation_data->annotation_window != NULL ) {
        g_printf("setting annotation window position %d %d %d %d\n", x, y, width, height);
      gtk_window_move( GTK_WINDOW(annotation_data->annotation_window) , x, y );

      gtk_window_set_keep_above (GTK_WINDOW (annotation_data->annotation_window), TRUE);

      make_annotation_window_transparent();

      gtk_widget_set_size_request (annotation_data->annotation_window, width, height );
      gtk_widget_show_all (annotation_data->annotation_window);
    }

}


/* Create the directory where put the save-point files. */
static void
create_savepoint_dir    ()
{
  const gchar *tmpdir = g_get_tmp_dir ();
  gchar *images = "images";
  gchar *project_name = get_project_name ();
  gchar *ardesia_tmp_dir = g_build_filename (tmpdir, PACKAGE_NAME, (gchar *) 0);
  gchar *project_tmp_dir = g_build_filename (ardesia_tmp_dir, project_name, (gchar *) 0);

  if (g_file_test (ardesia_tmp_dir, G_FILE_TEST_IS_DIR))
    {
      /* The folder already exist;I delete it. */
      rmdir_recursive (ardesia_tmp_dir);
    }

  annotation_data->savepoint_dir = g_build_filename (project_tmp_dir, images, (gchar *) 0);
  g_mkdir_with_parents (annotation_data->savepoint_dir, 0777);
  g_free (ardesia_tmp_dir);
  g_free (project_tmp_dir);
}


/* Delete the save-point. */
static void
delete_savepoint        (AnnotateSavepoint *savepoint)
{
  if (savepoint)
    {

      if (annotation_data->debug)
        {
          g_printerr ("The save-point %s has been removed\n", savepoint->filename);
        }

      if (savepoint->filename)
        {
          g_remove (savepoint->filename);
          g_free (savepoint->filename);
          savepoint->filename = (gchar *) NULL;
        }
      annotation_data->savepoint_list = g_slist_remove (annotation_data->savepoint_list, savepoint);
      g_free (savepoint);
      savepoint = (AnnotateSavepoint *) NULL;
    }
}


/* Free the list of the  save-point for the redo. */
static void
annotate_redolist_free       ()
{
  guint i = annotation_data->current_save_index;
  GSList *stop_list = g_slist_nth (annotation_data->savepoint_list, i);

  while (annotation_data->savepoint_list != stop_list)
    {
      AnnotateSavepoint *savepoint = (AnnotateSavepoint *) g_slist_nth_data (annotation_data->savepoint_list, 0);
      delete_savepoint (savepoint);
    }
}


/* Free the list of all the save-point. */
static void
annotate_savepoint_list_free ()
{
  g_slist_foreach (annotation_data->savepoint_list, (GFunc) delete_savepoint, (gpointer) NULL);

  annotation_data->savepoint_list = (GSList *) NULL;
}


/* Delete the ardesia temporary directory */
static void
delete_ardesia_tmp_dir       ()
{
  gchar *ardesia_tmp_dir = g_build_filename (g_get_tmp_dir (), PACKAGE_NAME, (gchar *) 0);
  rmdir_recursive (ardesia_tmp_dir);
  g_free (ardesia_tmp_dir);
}


/* Draw an arrow starting from the point
 * whith the width and the direction in radiant
 */
static void
draw_arrow_in_point     (AnnotatePoint      *point,
                         gdouble             width,
                         gdouble             direction)
{

  gdouble width_cos = width * cos (direction);
  gdouble width_sin = width * sin (direction);

  /* Vertex of the arrow. */
  gdouble arrow_head_0_x = point->x + width_cos;
  gdouble arrow_head_0_y = point->y + width_sin;

  /* Left point. */
  gdouble arrow_head_1_x = point->x - width_cos + width_sin;
  gdouble arrow_head_1_y = point->y -  width_cos - width_sin;

  /* Origin. */
  gdouble arrow_head_2_x = point->x - 0.8 * width_cos;
  gdouble arrow_head_2_y = point->y - 0.8 * width_sin;

  /* Right point. */
  gdouble arrow_head_3_x = point->x - width_cos - width_sin;
  gdouble arrow_head_3_y = point->y +  width_cos - width_sin;

  cairo_save (annotation_data->annotation_cairo_context);
  cairo_stroke (annotation_data->annotation_cairo_context);

  /* Initialize cairo properties. */
  cairo_set_line_join (annotation_data->annotation_cairo_context, CAIRO_LINE_JOIN_MITER);
  cairo_set_operator (annotation_data->annotation_cairo_context, CAIRO_OPERATOR_SOURCE);
  cairo_set_line_width (annotation_data->annotation_cairo_context, width);

  /* Draw the arrow. */
  cairo_move_to (annotation_data->annotation_cairo_context, arrow_head_2_x, arrow_head_2_y);
  cairo_line_to (annotation_data->annotation_cairo_context, arrow_head_1_x, arrow_head_1_y);
  cairo_line_to (annotation_data->annotation_cairo_context, arrow_head_0_x, arrow_head_0_y);
  cairo_line_to (annotation_data->annotation_cairo_context, arrow_head_3_x, arrow_head_3_y);

  cairo_close_path (annotation_data->annotation_cairo_context);
  cairo_fill_preserve (annotation_data->annotation_cairo_context);
  cairo_stroke (annotation_data->annotation_cairo_context);
  cairo_surface_flush( cairo_get_target(annotation_data->annotation_cairo_context) );
  cairo_restore (annotation_data->annotation_cairo_context);

  if (annotation_data->debug)
    {
      g_printerr ("with vertex at (x,y)= (%f : %f)\n",  arrow_head_0_x , arrow_head_0_y);
    }
}


/* Configure pen option for cairo context. */
void
annotate_configure_pen_options    (AnnotateData       *data)
{

  if (annotation_data->annotation_cairo_context)
    {
      cairo_new_path (annotation_data->annotation_cairo_context);
      cairo_set_line_cap (annotation_data->annotation_cairo_context, CAIRO_LINE_CAP_ROUND);
      cairo_set_line_join (annotation_data->annotation_cairo_context, CAIRO_LINE_JOIN_ROUND);

      if (annotation_data->cur_context->type == ANNOTATE_ERASER)
        {
          annotation_data->cur_context = annotation_data->default_eraser;
          cairo_set_operator (annotation_data->annotation_cairo_context, CAIRO_OPERATOR_CLEAR);
          cairo_set_line_width (annotation_data->annotation_cairo_context, annotate_get_thickness ());
        }
      else
        {
          cairo_set_operator (annotation_data->annotation_cairo_context, CAIRO_OPERATOR_SOURCE);
          cairo_set_line_width (annotation_data->annotation_cairo_context, annotate_get_thickness ());
        }
    }
    select_color ();
}


/*
 * Add a save point for the undo/redo;
 * this code must be called at the end of each painting action.
 * Called on_button_release
 * Called on annotate_push_context
 * Called on annotate_fill
 * Called on annotate_clear_screen
 */
void
annotate_add_savepoint  ()
{
  AnnotateSavepoint *savepoint = g_malloc ((gsize) sizeof (AnnotateSavepoint));
  cairo_surface_t *saved_surface = (cairo_surface_t *) NULL;
  cairo_surface_t *source_surface = (cairo_surface_t *) NULL;
  cairo_t *cr = (cairo_t *) NULL;

  /* The story about the future is deleted. */
  annotate_redolist_free ();

  guint savepoint_index = g_slist_length (annotation_data->savepoint_list) + 1;

  savepoint->filename = g_strdup_printf ("%s%s%s_%d_vellum.png",
                                          annotation_data->savepoint_dir,
                                          G_DIR_SEPARATOR_S,
                                          PACKAGE_NAME,
                                          savepoint_index);

  /* Add a new save-point. */
  annotation_data->savepoint_list = g_slist_prepend (annotation_data->savepoint_list, savepoint);
  annotation_data->current_save_index = 0;

  int w; int h;
  get_context_size( annotation_data->annotation_cairo_context, &w, &h );

  /* Load a surface with the annotation_data->annotation_cairo_context content and write the file. */
  saved_surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                              w,
                                              h);

  source_surface = cairo_get_target (annotation_data->annotation_cairo_context);
  cr = cairo_create (saved_surface);
  cairo_set_source_surface (cr, source_surface, 0, 0);
  cairo_paint (cr);
  /* Postcondition: the saved_surface now contains the save-point image. */


  /*  Will be create a file in the save-point folder with format PACKAGE_NAME_1.png. */
  cairo_surface_write_to_png (saved_surface, savepoint->filename);
  cairo_surface_destroy (saved_surface);
  if (annotation_data->debug)
    {
      g_printerr ("The save point %s has been stored in file\n", savepoint->filename);
    }

  cairo_destroy (cr);
}


/* Initialize the annotation cairo context */
void
initialize_annotation_cairo_context    (AnnotateData *data)
{

  if (annotation_data->annotation_cairo_context == NULL)
    {
        g_printf("initializing annotation cairo context\n");
      /* Initialize a transparent window. */
#ifdef _WIN32
      /* The hdc has depth 32 and the technology is DT_RASDISPLAY. */
      HDC hdc = GetDC (GDK_WINDOW_HWND (gtk_widget_get_window (annotation_data->annotation_window)));
      /*
       * @TODO Use an HDC that support the ARGB32 format to support the alpha channel;
       * this might fix the highlighter bug.
       * In the documentation is written that the now the resulting surface is in RGB24 format.
       *
       */
      cairo_surface_t *surface = cairo_win32_surface_create (hdc);

      annotation_data->annotation_cairo_context = cairo_create (surface);
#else
        int width = gtk_widget_get_allocated_width(annotation_data->annotation_window);
        int height = gtk_widget_get_allocated_height(annotation_data->annotation_window);
      annotation_data->annotation_cairo_context = create_new_context(width, height);
      background_data->cr = create_new_context(width, height);

#endif

  if (cairo_status (annotation_data->annotation_cairo_context) != CAIRO_STATUS_SUCCESS)
    {
      g_printerr ("Failed to allocate the annotation cairo context");
      annotate_quit ();
      exit (EXIT_FAILURE);
    }
    cairo_set_operator (annotation_data->annotation_cairo_context, CAIRO_OPERATOR_OVER);

  if (annotation_data->savepoint_list == NULL)
    {
      /* Clear the screen and create the first empty savepoint. */
      annotate_clear_screen ();
    }

#ifndef _WIN32
      gtk_widget_set_opacity (annotation_data->annotation_window, 1.0);
#endif

   annotate_acquire_grab ();

    }
}


/* Draw the last save point on the window restoring the surface. */
void
annotate_restore_surface     ()
{
g_printf ("annotate_restore_surface\n");

  if (annotation_data->debug)
    {
      g_printerr ("Restore surface\n");
    }

  if (annotation_data->annotation_cairo_context)
    {
      guint i = annotation_data->current_save_index;
      g_printf("i=%d\n",i);
      if (g_slist_length (annotation_data->savepoint_list)==i)
        {
            g_printf("length == i\n");
          cairo_new_path (annotation_data->annotation_cairo_context); // clears path and current point
          clear_cairo_context (annotation_data->annotation_cairo_context);
          return;
        }

      AnnotateSavepoint *savepoint = (AnnotateSavepoint *) g_slist_nth_data (annotation_data->savepoint_list, i);

      if (!savepoint)
        {
            g_printf("savepoint is FALSE\n");
          return;
        }

      cairo_save( annotation_data->annotation_cairo_context);
      clear_cairo_context( annotation_data->annotation_cairo_context);
      cairo_new_path (annotation_data->annotation_cairo_context);
      cairo_set_operator (annotation_data->annotation_cairo_context, CAIRO_OPERATOR_SOURCE);

      if (savepoint->filename)
        {
            g_printf("load savepoint from filename %s\n", savepoint->filename);
          /* Load the file in the annotation surface. */
          cairo_surface_t *image_surface = cairo_image_surface_create_from_png (savepoint->filename);
          if (annotation_data->debug)
            {
              g_printerr ("The save-point %s has been loaded from file\n", savepoint->filename);
            }

          if (image_surface)
            {
            g_printf("paint savepoint %s\n", savepoint->filename);
              cairo_set_source_surface (annotation_data->annotation_cairo_context, image_surface, 0, 0);
              cairo_paint (annotation_data->annotation_cairo_context);
              cairo_stroke (annotation_data->annotation_cairo_context);
              //cairo_surface_flush( cairo_get_target(annotation_data->annotation_cairo_context) );
              cairo_surface_destroy (image_surface);
            }

        }
        cairo_restore( annotation_data->annotation_cairo_context);
        // try and redraw
    }

    gtk_widget_queue_draw( annotation_data->annotation_window);

    //gtk_widget_show (annotation_data->annotation_window);
}


/* Get the annotation window. */
GtkWidget *
get_annotation_window   ()
{
  return annotation_data->annotation_window;
}


/* Set colour. */
void
annotate_set_color      (gchar      *color)
{
  annotation_data->color = color;
}


/* Set rectifier. */
void
annotate_set_rectifier  (gboolean  rectify)
{
  annotation_data->rectify = rectify;
}


/* Set rounder. */
void
annotate_set_rounder    (gboolean roundify)
{
  annotation_data->roundify = roundify;
}


/* Set arrow. */
void
annotate_set_arrow      (gboolean    arrow)
{
  annotation_data->arrow = arrow;
}


/* Set the line thickness. */
void
annotate_set_thickness  (gdouble thickness)
{
  annotation_data->thickness = thickness;
}


/* Get the line thickness. */
gdouble
annotate_get_thickness  ()
{
  if (annotation_data->cur_context->type == ANNOTATE_ERASER)
    {
      /* the eraser is bigger than pen */
      gdouble corrective_factor = 2.5;
      return annotation_data->thickness * corrective_factor;
    }

  return annotation_data->thickness;
}


/* Add to the list of the painted point the point (x,y). */
void
annotate_coord_list_prepend  (AnnotateDeviceData  *devdata,
                              gdouble              x,
                              gdouble              y,
                              gdouble              width,
                              gdouble              pressure)
{
  AnnotatePoint *point = g_malloc ((gsize) sizeof (AnnotatePoint));
  point->x = x;
  point->y = y;
  point->width = width;
  point->pressure = pressure;
  devdata->coord_list = g_slist_prepend (devdata->coord_list, point);
  devdata->length++;
  replace_status_message(g_strdup_printf("%d points", devdata->length));
}


/* Free the coord list belonging to the the owner devdata device. */
void
annotate_coord_dev_list_free (AnnotateDeviceData *devdata)
{
  if (devdata->coord_list)
    {
      g_slist_foreach (devdata->coord_list, (GFunc) g_free, (gpointer) NULL);
      g_slist_free (devdata->coord_list);
      devdata->coord_list = (GSList *) NULL;
      devdata->length = 0;
      //replace_status_message(g_strdup_printf("%d points", devdata->length));
  } else {
      devdata->length = 0;
  }
}


/* Modify colour according to the pressure. */
void
annotate_modify_color   (AnnotateDeviceData *devdata,
                         AnnotateData       *data,
                         gdouble             pressure)
{
  /* Pressure value is from 0 to 1;this value modify the RGBA gradient. */
  guint r,g,b,a;
  gdouble old_pressure = pressure;

  /* If you put an higher value you will have more contrast
   * between the lighter and darker colour depending on pressure.
   */
  gdouble contrast = 96;
  gdouble corrective = 0;

  /* The pressure is greater than 0. */
  if ( (!annotation_data->annotation_cairo_context) || (!annotation_data->color))
    {
      return;
    }

  if (pressure >= 1)
    {
      cairo_set_source_color_from_string (annotation_data->annotation_cairo_context,
                                          annotation_data->color);
    }
  else if (pressure <= 0.1)
    {
      pressure = 0.1;
    }

  assert( strlen( annotation_data->color ) == 8 );
  sscanf (annotation_data->color, "%02X%02X%02X%02X", &r, &g, &b, &a);

  if (devdata->coord_list != NULL)
    {
      AnnotatePoint *last_point = (AnnotatePoint *) g_slist_nth_data (devdata->coord_list, 0);
      old_pressure = last_point->pressure;
    }

  corrective = (1- ( 3 * pressure + old_pressure)/4) * contrast;
  cairo_set_source_rgba (annotation_data->annotation_cairo_context,
                         (r + corrective)/255,
                         (g + corrective)/255,
                         (b + corrective)/255,
                         (gdouble) a/255);
}


/* Paint the context over the annotation window. */
void
annotate_push_context (cairo_t * cr)
{
    cairo_save( annotation_data->annotation_cairo_context);
    cairo_surface_t* source_surface = (cairo_surface_t *) NULL;
    if (annotation_data->debug)
    {
      g_printerr ("The text window content has been painted over the annotation window\n");
    }

    // this clears the current path from the cairo context
    cairo_new_path (annotation_data->annotation_cairo_context);
    // this gets the target surface for the cairo context
    source_surface = cairo_get_target (cr);

    cairo_set_operator (annotation_data->annotation_cairo_context, CAIRO_OPERATOR_ADD);

    // creates a pattern from surface at x,y on the context
    // at 0, left screen -> right screen, right screen disappears
    // at -1920, left screen -> disappears, right screen is good
    cairo_set_source_surface (annotation_data->annotation_cairo_context, source_surface, 0, 0);
    // paints the current source everywhere in clip region
    cairo_paint (annotation_data->annotation_cairo_context);
    // strokes the current path according to current line settings
    cairo_stroke (annotation_data->annotation_cairo_context);

    cairo_restore( annotation_data->annotation_cairo_context);
    annotate_add_savepoint ();
}


/* Select the default pen tool. */
void
annotate_select_pen          ()
{
  if (annotation_data->debug)
    {
      g_printerr ("The pen with colour %s has been selected\n",
                  annotation_data->color);
    }

  if (annotation_data->default_pen)
    {
      annotation_data->cur_context = annotation_data->default_pen;
      annotation_data->old_paint_type = ANNOTATE_PEN;

      disallocate_cursor ();

      set_pen_cursor (&annotation_data->cursor,
                      annotation_data->thickness,
                      annotation_data->color,
                      annotation_data->arrow);

      update_cursor ();
    }
}





/* Select the default filler tool. */
void
annotate_select_filler       ()
{
  if (annotation_data->debug)
    {
      g_printerr ("The pen with colour %s has been selected\n",
                  annotation_data->color);
    }

  if (annotation_data->default_pen)
    {
      annotation_data->cur_context = annotation_data->default_filler;
      annotation_data->old_paint_type = ANNOTATE_FILLER;

      disallocate_cursor ();

      set_filler_cursor (&annotation_data->cursor);

      update_cursor ();
    }
}


/* Select the default eraser tool. */
void
annotate_select_eraser       ()
{
  if (annotation_data->debug)
    {
      g_printerr ("The eraser has been selected\n");
    }

  annotation_data->cur_context = annotation_data->default_eraser;
  annotation_data->old_paint_type = ANNOTATE_ERASER;

  disallocate_cursor ();

  set_eraser_cursor (&annotation_data->cursor, annotate_get_thickness ());

  update_cursor ();
}


/* Unhide the cursor. */
void
annotate_unhide_cursor       ()
{
  if (annotation_data->is_cursor_hidden)
    {
      update_cursor ();
      annotation_data->is_cursor_hidden = FALSE;
    }
}


/* Hide the cursor icon. */
void
annotate_hide_cursor         ()
{
  gdk_window_set_cursor (gtk_widget_get_window (annotation_data->annotation_window),
                         annotation_data->invisible_cursor);

  annotation_data->is_cursor_hidden = TRUE;
}


/* acquire the grab. */
void
annotate_acquire_grab        ()
{
  ungrab_pointer     (gdk_display_get_default ());
  if  (!annotation_data->is_grabbed)
    {

      if (annotation_data->debug)
        {
          g_printerr ("Acquire grab\n");
        }

      annotate_acquire_input_grab ();
      annotation_data->is_grabbed = TRUE;
    }
}


/* Draw line from the last point drawn to (x2,y2);
 * if stroke is false the cairo path is not forgotten
 */
void
annotate_draw_line      (AnnotateDeviceData  *devdata,
                         gdouble              x2,
                         gdouble              y2,
                         gboolean             stroke)
{
    cairo_save( annotation_data->annotation_cairo_context);
  if (!stroke)
    {
      cairo_line_to (annotation_data->annotation_cairo_context, x2, y2);
    }
  else
    {
      AnnotatePoint *last_point = (AnnotatePoint *) g_slist_nth_data (devdata->coord_list, 0);
      if (last_point)
        {
          cairo_move_to (annotation_data->annotation_cairo_context, last_point->x, last_point->y);
        }
      else
        {
          cairo_move_to (annotation_data->annotation_cairo_context, x2, y2);
        }
      cairo_line_to (annotation_data->annotation_cairo_context, x2, y2);
      cairo_stroke (annotation_data->annotation_cairo_context);
    }
    cairo_restore( annotation_data->annotation_cairo_context);
}


/* Draw the point list. */
void
annotate_draw_point_list     (AnnotateDeviceData *devdata,
                              GSList             *list)
{
    cairo_save( annotation_data->annotation_cairo_context);
  if (list)
    {
      guint i = 0;
      guint length = g_slist_length (list);
      for (i=0; i<length; i=i+1)
        {
          AnnotatePoint *point = (AnnotatePoint *) g_slist_nth_data (list, i);
          if (!point)
            {
              return;
            }

          if (length == 1)
            {
              /* It is a point. */
              annotate_draw_point (devdata, point->x, point->y, point->pressure);
              break;
            }
          annotate_modify_color (devdata, annotation_data, point->pressure);
          /* Draw line between the two points. */
          annotate_draw_line (devdata, point->x, point->y, FALSE);
        }
    }
    cairo_restore( annotation_data->annotation_cairo_context);
}


/* Draw an arrow using some polygons. */
void
annotate_draw_arrow     (AnnotateDeviceData  *devdata,
                         gdouble              distance)
{
  gdouble direction = 0;
  gdouble pen_width = annotate_get_thickness ();
  gdouble arrow_minimum_size = pen_width * 2;

  AnnotatePoint *point = (AnnotatePoint *) g_slist_nth_data (devdata->coord_list, 0);

  if (distance < arrow_minimum_size)
    {
      return;
    }

  if (annotation_data->debug)
    {
      g_printerr ("Draw arrow: ");
    }

  if (g_slist_length (devdata->coord_list) < 2)
    {
      /* If it has length lesser then two then is a point and it has no sense draw the arrow. */
      return;
    }

  /* Postcondition length >= 2 */
  direction = annotate_get_arrow_direction (devdata);

  if (annotation_data->debug)
    {
      g_printerr ("Arrow direction %f\n", direction/M_PI*180);
    }

  draw_arrow_in_point (point, pen_width, direction);
}


/* Fill the contiguos area around point with coordinates (x,y). */
void
annotate_fill                (AnnotateDeviceData *devdata,
                              AnnotateData       *data,
                              gdouble             x,
                              gdouble             y)
{
  cairo_save( annotation_data->annotation_cairo_context);
  int width = gtk_widget_get_allocated_width(annotation_data->annotation_window);
  int height = gtk_widget_get_allocated_width(annotation_data->annotation_window);
  cairo_surface_t  *image_surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                              width,
                                              height);


  cairo_surface_t *source_surface = cairo_get_target (annotation_data->annotation_cairo_context);

  // first we paint a new context with our current window image
  cairo_t *cr = cairo_create (image_surface);
  cairo_set_source_surface (cr, source_surface, 0, 0);
  cairo_paint (cr);

  select_color (devdata);

  if (annotation_data->debug)
    {
      g_printerr ("Fill with fill flood algorithm\n");
    }

  flood_fill (annotation_data->annotation_cairo_context,
              image_surface,
              annotation_data->color,
              x,
              y);
  cairo_surface_flush( cairo_get_target(annotation_data->annotation_cairo_context) );
  cairo_surface_destroy (image_surface);

  cairo_restore( annotation_data->annotation_cairo_context);
  annotate_add_savepoint ();
}


/* Draw a point in x,y respecting the context. */
void
annotate_draw_point          (AnnotateDeviceData  *devdata,
                              gdouble              x,
                              gdouble              y,
                              gdouble              pressure)
{
  /* Modify a little bit the colour depending on pressure. */
  cairo_save( annotation_data->annotation_cairo_context );
  annotate_modify_color (devdata, annotation_data, pressure);
  cairo_move_to (annotation_data->annotation_cairo_context, x, y);
  cairo_line_to (annotation_data->annotation_cairo_context, x, y);
  cairo_restore( annotation_data->annotation_cairo_context );
}


/* Call the geometric shape recognizer. */
void
annotate_shape_recognize     (AnnotateDeviceData  *devdata,
                              gboolean             closed_path)
{
  if (annotation_data->rectify)
    {
      rectify (devdata, closed_path);
    }
  else if (annotation_data->roundify)
    {
      roundify (devdata, closed_path);
    }
}


/* Select eraser, pen or other tool for tablet. */
void
annotate_select_tool (AnnotateData *data,
                      GdkDevice *masterdevice,
                      GdkDevice *slavedevice,
                      guint state)
{
  AnnotateDeviceData *masterdata = g_hash_table_lookup (data->devdatatable, masterdevice);
  AnnotateDeviceData *slavedata = g_hash_table_lookup (data->devdatatable, slavedevice);

  if (slavedevice)
    {
      if (gdk_device_get_source (slavedevice) == GDK_SOURCE_ERASER)
        {
          annotate_select_eraser ();
          data->old_paint_type = ANNOTATE_PEN;
        }
      else
        {
          if (data->old_paint_type == ANNOTATE_ERASER)
            {
              annotate_select_eraser ();
            }
          else
            {
              annotate_select_pen ();
            }
        }

    }
  else
    {
      g_printerr ("Attempt to select non existent device!\n");
      data->cur_context = data->default_pen;
    }

  masterdata->lastslave = slavedevice;
  masterdata->state = state;
  slavedata->state = state;
}


/* Free the memory allocated by paint context */
void
annotate_paint_context_free (AnnotatePaintContext *context)
{
  if (context)
    {
      g_free (context);
      context = (AnnotatePaintContext *) NULL;
    }
}


/* Quit the annotation. */
void
annotate_quit           ()
{
    // destroy data structures of other contexts
    if ( background_data ) {
        destroy_background_data();
    }


  if (annotation_data)
    {
      if (annotation_data->color)
        {
          g_free (annotation_data->color);
          annotation_data->color = NULL;
        }

      /* Destroy cursors. */
      disallocate_cursor ();
      cursors_main_quit ();

      /* Destroy cairo object. */
      destroy_cairo (annotation_data->annotation_cairo_context);

      if (annotation_data->invisible_cursor)
        {
          g_object_unref (annotation_data->invisible_cursor);
          annotation_data->invisible_cursor = (GdkCursor *) NULL;
        }

        if ( annotation_data->clapperboard_cairo_context ) {
            destroy_cairo( annotation_data->clapperboard_cairo_context );
        }

        if ( annotation_data->recordingstudio_options) {
            g_free( annotation_data->recordingstudio_options);
        }

        if ( annotation_data->recordingstudio_window) {
            gtk_widget_destroy (annotation_data->recordingstudio_window);
            annotation_data->recordingstudio_window = (GtkWidget *) NULL;
        }

      /* Free all. */
      if (annotation_data->annotation_window)
        {
          gtk_widget_destroy (annotation_data->annotation_window);
          annotation_data->annotation_window = (GtkWidget *) NULL;
        }

      remove_input_devices (annotation_data);
      annotate_savepoint_list_free ();

      delete_ardesia_tmp_dir();

      if (annotation_data->savepoint_dir)
        {
          g_free (annotation_data->savepoint_dir);
          annotation_data->savepoint_dir = (gchar *) NULL;
        }

      if (annotation_data->default_pen)
        {
          annotate_paint_context_free (annotation_data->default_pen);
        }

     if (annotation_data->default_eraser)
        {
          annotate_paint_context_free (annotation_data->default_eraser);
        }

    if (annotation_data->default_filler)
        {
          annotate_paint_context_free (annotation_data->default_filler);
        }
    }

    if ( annotation_data->monitor ) {
        g_free( annotation_data->monitor );
        annotation_data->monitor = NULL;
    }
}


/* Release input grab;the input event will be passed below the window. */
void
annotate_release_input_grab  ()
{
    g_printf("annotate_release_input_grab\n");
  ungrab_pointer (gdk_display_get_default ());
  //gdk_window_set_cursor (gtk_widget_get_window (annotation_data->annotation_window), (GdkCursor *) NULL);
#ifndef _WIN32
  /*
   * @TODO implement correctly gtk_widget_input_shape_combine_mask
   * in the quartz gdkwindow or use an equivalent native function;
   * the current implementation in macosx this does not do nothing.
   */
  /*
   * This allows the mouse event to be passed below the transparent annotation;
   * at the moment this call works only on Linux
   */
  gtk_widget_input_shape_combine_region (annotation_data->annotation_window, NULL);

  const cairo_rectangle_int_t ann_rect = { 0, 0, 0, 0 };
  cairo_region_t *r = cairo_region_create_rectangle (&ann_rect);

  gtk_widget_input_shape_combine_region (annotation_data->annotation_window, r);
  cairo_region_destroy (r);

#else
  /*
   * @TODO WIN32 implement correctly gtk_widget_input_shape_combine_mask
   * in the win32 gdkwindow or use an equivalent native function.
   * Now in the gtk implementation the gtk_widget_input_shape_combine_mask
   * call the gtk_widget_shape_combine_mask that is not the desired behaviour.
   *
   */

#endif
}


/* Release the pointer pointer. */
void
annotate_release_grab   ()
{
  if (annotation_data->is_grabbed)
    {

      if (annotation_data->debug)
        {
          g_printerr ("Release grab\n");
        }

      annotate_release_input_grab ();
      gtk_window_present (GTK_WINDOW (get_bar_widget ()));
      annotation_data->is_grabbed = FALSE;
    }
}


/* Undo reverting to the last save point. */
void
annotate_undo           ()
{
  if (annotation_data->debug)
    {
      g_printerr ("Undo\n");
    }

  if (annotation_data->savepoint_list)
    {
      if (annotation_data->current_save_index != g_slist_length (annotation_data->savepoint_list)-1)
        {
          annotation_data->current_save_index = annotation_data->current_save_index + 1;
          annotate_restore_surface ();
        }
    }
}


/* Redo to the last save point. */
void
annotate_redo           ()
{
  if (annotation_data->debug)
    {
      g_printerr ("Redo\n");
    }

  if (annotation_data->savepoint_list)
    {
      if (annotation_data->current_save_index != 0)
        {
          annotation_data->current_save_index = annotation_data->current_save_index - 1;
          annotate_restore_surface ();
        }
    }
}


/* Clear the annotations windows and make an empty savepoint. */
void
annotate_clear_screen   ()
{
    if (annotation_data->debug)
    {
        g_printerr ("Clear annotation window\n");
    }
    if ( annotation_data->annotation_cairo_context ) {
        /* clear existing cairo context */
        cairo_new_path (annotation_data->annotation_cairo_context);
        clear_cairo_context (annotation_data->annotation_cairo_context);
        cairo_set_operator (annotation_data->annotation_cairo_context, CAIRO_OPERATOR_SOURCE);

        /* call for a redraw */
        gtk_widget_queue_draw(annotation_data->annotation_window ); // generate expose event

        /* Add the empty savepoint. */
        annotate_add_savepoint ();
    }
}

static void
create_annotation_data() {
    annotation_data = g_malloc ((gsize) sizeof (AnnotateData));
    gchar* color = g_strdup ("FF0000FF");

    /* Initialize the data structure. */
    annotation_data->is_background_visible = FALSE;
    annotation_data->is_text_editor_visible = FALSE;
    annotation_data->is_annotation_visible = TRUE;
    annotation_data->is_window_covering_toolbar = TRUE; // err on side of caution
    annotation_data->is_opaque = FALSE;

    annotation_data->annotation_cairo_context = (cairo_t *) NULL;
    annotation_data->savepoint_list = (GSList *) NULL;
    annotation_data->current_save_index = 0;
    annotation_data->cursor = (GdkCursor *) NULL;
    annotation_data->devdatatable = (GHashTable *) NULL;

    annotation_data->color = color;
    annotation_data->is_grabbed = FALSE;
    annotation_data->arrow = FALSE;
    annotation_data->rectify = FALSE;
    annotation_data->roundify = FALSE;
    annotation_data->old_paint_type = ANNOTATE_PEN;

    annotation_data->is_cursor_hidden = TRUE;

    annotation_data->debug = FALSE;
    annotation_data->default_pen = annotate_paint_context_new (ANNOTATE_PEN);
    annotation_data->default_eraser = annotate_paint_context_new (ANNOTATE_ERASER);
    annotation_data->default_filler = annotate_paint_context_new (ANNOTATE_FILLER);
    annotation_data->cur_context = annotation_data->default_pen;
    annotation_data->monitor = NULL;

    annotation_data->recordingstudio_window_gtk_builder = NULL;
    annotation_data->recordingstudio_window = NULL;
    annotation_data->recordingstudio_options = NULL;

    annotation_data->clapperboard_cairo_context = NULL;
    annotation_data->is_clapperboard_visible = FALSE;

    annotation_data->cursor_window_gtk_builder = NULL;
    annotation_data->cursor_window = NULL;
    annotation_data->is_cursor_visible = FALSE;
    annotation_data->cursor_timer = 0;
    annotation_data->cursor_step = 0;
    
    // we create background data objects at the same time
    // to be safe
    background_data = create_background_data();
}


/* Initialize the annotation. */
void
annotate_init                (gchar      *iwb_file,
                              gboolean    debug,
                              Monitor*    monitor)
{
  cursors_main ();

  // setup AnnotateData object
  create_annotation_data();

  /* Initialize the pen context. */
  annotation_data->debug = debug;
  annotation_data->monitor = monitor;

  setup_input_devices (annotation_data);
  allocate_invisible_cursor (&annotation_data->invisible_cursor);

  create_savepoint_dir ();

  if (iwb_file)
    {
      annotation_data->savepoint_list = load_iwb (iwb_file);
    }

}

gboolean
annotation_window_button_press(GdkEventButton* ev, AnnotateData* data) {
    if ( data->is_text_editor_visible ) {
        return FALSE;
    }
    GdkDevice *master = gdk_event_get_device ( (GdkEvent *) ev);

    /* Get the data for this device. */
    AnnotateDeviceData *masterdata = g_hash_table_lookup (annotation_data->devdatatable, master);

    gdouble pressure = 1.0;

    if (annotation_data->cur_context == annotation_data->default_filler)
      {
        return FALSE;
      }

    if (!annotation_data->is_grabbed)
      {
        //return FALSE;
        g_printf("on_button_press: initialising cairo\n");
        initialize_annotation_cairo_context (data);
        if (!annotation_data->is_grabbed)
          {
              g_printf("on_button_press: initialising cairo failed\n");
              return FALSE;
          }
      }

    if (!ev)
      {
        g_printerr ("Device '%s': Invalid event; I ungrab all\n",
                    gdk_device_get_name (master));
        annotate_release_grab ();
        return FALSE;
      }

    if (annotation_data->debug)
      {
        g_printerr ("Device '%s': Button %i Down at (x,y)= (%f : %f)\n",
                    gdk_device_get_name (master),
                    ev->button,
                    ev->x,
                    ev->y);
      }

  #ifdef _WIN32
    if (inside_bar_window (ev->x_root, ev->y_root))
      {
        /* The point is inside the ardesia bar then ungrab. */
        annotate_release_grab ();
        return FALSE;
      }
  #endif

    pressure = get_pressure ( (GdkEvent *) ev);

    if (pressure <= 0)
      {
        return FALSE;
      }

    annotate_unhide_cursor ();

    // acquires the grab capability
    initialize_annotation_cairo_context (data);

    annotate_configure_pen_options (data);

    annotate_coord_dev_list_free (masterdata);
    annotate_draw_point (masterdata, ev->x, ev->y, pressure);

    annotate_coord_list_prepend (masterdata,
                                 ev->x,
                                 ev->y,
                                 annotate_get_thickness (),
                                 pressure);
    gtk_widget_queue_draw(annotation_data->annotation_window);
    return TRUE;
}

gboolean
annotation_window_mouse_move( GdkEventMotion* ev, AnnotateData* data ) {
    if ( data->is_text_editor_visible ) {
        return FALSE;
    }

    GdkDevice *master = gdk_event_get_device ( (GdkEvent *) ev);
    GdkDevice *slave = gdk_event_get_source_device ( (GdkEvent *) ev);

    /* Get the data for this device. */
    AnnotateDeviceData *masterdata= g_hash_table_lookup (data->devdatatable, master);
    AnnotateDeviceData *slavedata = g_hash_table_lookup (data->devdatatable, slave);

     if (data->cur_context == data->default_filler)
      {
        return FALSE;
      }

    if (ev->state != masterdata->state ||
        ev->state != slavedata->state  ||
        masterdata->lastslave != slave)
      {
         annotate_select_tool (data, master, slave, ev->state);
      }

    gdouble selected_width = 0.0;
    gdouble pressure = 1.0;

    if (!data->is_grabbed)
      {
        return FALSE;
      }

    if (!ev)
      {
        g_printerr ("Device '%s': Invalid event; I ungrab all\n",
                    gdk_device_get_name (master));
        annotate_release_grab ();
        return FALSE;
      }

    if (data->debug)
      {
        g_printerr ("Device '%s': Move at (x,y)= (%f : %f)\n",
                    gdk_device_get_name (master),
                    ev->x,
                    ev->y);
      }

  #ifdef _WIN32
    if (inside_bar_window (ev->x_root, ev->y_root))
      {

        if (data->debug)
          {
            g_printerr ("Device '%s': Move on the bar then ungrab\n",
                         gdk_device_get_name (master));
          }

        /* The point is inside the ardesia bar then ungrab. */
        annotate_release_grab ();
        return FALSE;
      }
  #endif

    annotate_unhide_cursor ();

    /* Only the first 5 buttons allowed. */
    if(!(ev->state & (GDK_BUTTON1_MASK|
                      GDK_BUTTON2_MASK|
                      GDK_BUTTON3_MASK|
                      GDK_BUTTON4_MASK|
                      GDK_BUTTON5_MASK)))
      {
        return TRUE;
      }

    initialize_annotation_cairo_context (data);

    annotate_configure_pen_options (data);

    if (data->cur_context->type != ANNOTATE_ERASER)
      {
        pressure = get_pressure ( (GdkEvent *) ev);

        if (pressure <= 0)
          {
            return FALSE;
          }

        /* If the point is already selected and higher pressure then print else jump it. */
        if (masterdata->coord_list)
          {
            AnnotatePoint *last_point = (AnnotatePoint *) g_slist_nth_data (masterdata->coord_list, 0);
            gdouble tollerance = annotate_get_thickness ();

            if (get_distance (last_point->x, last_point->y, ev->x, ev->y)<tollerance)
              {
                /* Seems that you are uprising the pen. */
                if (pressure <= last_point->pressure)
                  {
                    /* Jump the point you are uprising the hand. */
                    return FALSE;
                  }
                else // pressure >= last_point->pressure
                  {
                    /* Seems that you are pressing the pen more. */
                    annotate_modify_color (masterdata, data, pressure);
                    annotate_draw_line (masterdata, ev->x, ev->y, TRUE);
                    /* Store the new pressure without allocate a new coordinate. */
                    last_point->pressure = pressure;
                    return TRUE;
                  }
              }
            annotate_modify_color (masterdata, data, pressure);
          }
      }

    annotate_draw_line (masterdata, ev->x, ev->y, TRUE);
    annotate_coord_list_prepend (masterdata, ev->x, ev->y, selected_width, pressure);

    gtk_widget_queue_draw(annotation_data->annotation_window);
    return TRUE;
}


gboolean
annotation_window_button_release( GdkEventButton* ev, AnnotateData* data) {
    if ( data->is_text_editor_visible ) {
        return FALSE;
    }
    GdkDevice *master = gdk_event_get_device ( (GdkEvent *) ev);

    /* Get the data for this device. */
    AnnotateDeviceData *masterdata= g_hash_table_lookup (data->devdatatable, master);

    guint length = g_slist_length (masterdata->coord_list);

    if (!data->is_grabbed)
      {
        return FALSE;
      }

    if (!ev)
      {
        g_printerr ("Device '%s': Invalid event; I ungrab all\n",
                    gdk_device_get_name (master));
        annotate_release_grab ();
        return FALSE;
      }

    if (data->debug)
      {
        g_printerr ("Device '%s': Button %i Up at (x,y)= (%.2f : %.2f)\n",
                    gdk_device_get_name (master),
                     ev->button, ev->x, ev->y);
      }

  #ifdef _WIN32
    if (inside_bar_window (ev->x_root, ev->y_root))
      /* Point is in the ardesia bar. */
      {
        /* The last point was outside the bar then ungrab. */
        annotate_release_grab ();
        return FALSE;
      }
    if (data->old_paint_type == ANNOTATE_PEN)
      {
         annotate_select_pen ();
      }
  #endif

    // this was required to stop if from permanently holding on the screen
    // over the ardesia bar
    if (inside_bar_window (ev->x_root, ev->y_root))
    /* Point is in the ardesia bar. */
    {
      /* The last point was outside the bar then ungrab. */
      annotate_release_grab ();
      return FALSE;
    }

    if (data->cur_context == data->default_filler)
      {
        annotate_fill (masterdata, data, ev->x, ev->y);
        return TRUE;
      }

    initialize_annotation_cairo_context (data);


    if (length > 2)
      {
        AnnotatePoint *first_point = (AnnotatePoint *) g_slist_nth_data (masterdata->coord_list, length-1);
        AnnotatePoint *last_point = (AnnotatePoint *) g_slist_nth_data (masterdata->coord_list, 0);

        gdouble distance = get_distance (ev->x, ev->y, first_point->x, first_point->y);

        /* This is the tolerance to force to close the path in a magnetic way. */
        gint score = 3;

        /* If is applied some handled drawing mode then the tool is more tollerant. */
        if ((data->rectify || data->roundify))
          {
            score = 6;
          }

        gdouble tollerance = annotate_get_thickness () * score;

        gdouble pressure = last_point->pressure;
        annotate_modify_color (masterdata, data, pressure);

        gboolean closed_path = FALSE;

        /* If the distance between two point lesser than tolerance they are the same point for me. */
        if (distance > tollerance)
          {
            /* Different point. */
            annotate_draw_line (masterdata, ev->x, ev->y, TRUE);
            annotate_coord_list_prepend (masterdata, ev->x, ev->y, annotate_get_thickness (), pressure);
          }
        else
          {
            /* Rounded to be the same point. */
            closed_path = TRUE; // this seems to be a closed path
            annotate_draw_line (masterdata, first_point->x, first_point->y, TRUE);
            annotate_coord_list_prepend (masterdata, first_point->x, first_point->y, annotate_get_thickness (), pressure);
          }

        if (data->cur_context->type != ANNOTATE_ERASER)
          {
            annotate_shape_recognize (masterdata, closed_path);

            /* If is selected an arrow type then I draw the arrow. */
            if (data->arrow)
              {
                /* Print arrow at the end of the path. */
                annotate_draw_arrow (masterdata, distance);
              }
          }
      }

    cairo_stroke (data->annotation_cairo_context);


    annotate_add_savepoint ();

    annotate_hide_cursor ();

    gtk_widget_queue_draw(annotation_data->annotation_window);
    return TRUE;
}


/** during the window configuration and resize callback */
void
annotation_window_change(int width, int height) {
    // if ( background_data->cr != NULL ) {
    //     cairo_destroy( background_data->cr );
    // }
    if ( background_data->cr == NULL ) {
        background_data->cr = create_new_context(width, height);
    }
}

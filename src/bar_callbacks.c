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

#include <gdk/gdk.h>
#include <utils.h>
#include <bar_callbacks.h>
#include <bar.h>
#include <annotation_window.h>
#include <background_window.h>
#include <text_window.h>
#include <color_selector.h>
#include <preference_dialog.h>
#include <info_dialog.h>
#include <iwb_saver.h>
#include <recorder.h>
#include <saver.h>
#include <pdf_saver.h>
#include <share_confirmation_dialog.h>


/* Timer used to up-rise the window. */
static gint timer = -1;

GtkStatusbar* getStatusbar() {
    GObject *g_object = gtk_builder_get_object (bar_gtk_builder, gettext("statusbar") );
    return GTK_STATUSBAR( g_object );
}

void
replace_status_message( gchar* message ) {
    GtkStatusbar* bar = getStatusbar();
    if ( bar != NULL ) {
        //guint contextID = gtk_statusbar_get_context_id( bar, gettext("context description"));
        gtk_statusbar_pop( bar, 0 );
        gtk_statusbar_push( bar, 0, message );
    }
}

void
setStatusbarLabel( gchar* message ) {
    GObject *g_object = gtk_builder_get_object (bar_gtk_builder, gettext("labelCurrentSelection") );
    GtkLabel* label = GTK_LABEL( g_object );
    gtk_label_set_label( label, gettext(message) );
}
/* Try to up-rise the window;
 * this is used for the window manager
 * that does not support the stay above directive.
 */
static gboolean
bar_to_top         (gpointer data)
{
  if (!gtk_widget_get_visible (GTK_WIDGET (data)))
    {
       gtk_window_present (GTK_WINDOW (data));
       gtk_widget_grab_focus (data);
       gdk_window_lower (gtk_widget_get_window (GTK_WIDGET (data)));
    }
  return TRUE;
}


/* Is the toggle tool button specified with name is active? */
static gboolean
is_toggle_tool_button_active      (gchar *toggle_tool_button_name)
{
  GObject *g_object = gtk_builder_get_object (bar_gtk_builder, toggle_tool_button_name);
  GtkToggleToolButton *toggle_tool_button = GTK_TOGGLE_TOOL_BUTTON (g_object);
  return gtk_toggle_tool_button_get_active (toggle_tool_button);
}

/* Get GtkImage object from builder
 * 2018-08-09: TM
 */
static GtkImage* get_image_from_builder(gchar *image_name) {
    GObject *g_object = gtk_builder_get_object (bar_gtk_builder, image_name);
    GtkImage *image = GTK_IMAGE (g_object);
    return image;
}

/* Is the show/hide toggle tool button active?
 * 2018-08-09 : Added by TM
 */
// static
// gboolean is_hide_toggle_tool_button_active    ()
// {
//   return is_toggle_tool_button_active ("buttonShowHide");
// }

/* Is the text toggle tool button active? */
static
gboolean is_text_toggle_tool_button_active       ()
{
  return is_toggle_tool_button_active ("buttonText");
}


/* Is the highlighter toggle tool button active? */
static
gboolean is_highlighter_toggle_tool_button_active          ()
{
  return is_toggle_tool_button_active ("buttonHighlighter");
}


/* Is the filler toggle tool button active? */
static
gboolean is_filler_toggle_tool_button_active          ()
{
  return is_toggle_tool_button_active ("buttonFiller");
}


/* Is the eraser toggle tool button active? */
static
gboolean is_eraser_toggle_tool_button_active     ()
{
  return is_toggle_tool_button_active ("buttonEraser");
}


/* Is the eraser toggle tool button active? */
static
gboolean is_pen_toggle_tool_button_active     ()
{
  return is_toggle_tool_button_active ("buttonPencil");
}


/* Is the pointer toggle tool button active? */
static
gboolean is_pointer_toggle_tool_button_active    ()
{
  return is_toggle_tool_button_active ("buttonPointer");
}


/* Is the pointer toggle tool button active? */
static
gboolean is_arrow_toggle_tool_button_active      ()
{
  return is_toggle_tool_button_active ("buttonArrow");
}


/* Add alpha channel to build the RGBA string. */
static void
add_alpha               (BarData *bar_data)
{
    assert(strlen(bar_data->color) == 8 );
  if (is_highlighter_toggle_tool_button_active ())
    {
      strncpy (&bar_data->color[6], SEMI_OPAQUE_ALPHA, 2);
    }
  else
    {
      strncpy (&bar_data->color[6], OPAQUE_ALPHA, 2);
    }
}


/* Select the pen tool. */
static void
take_pen_tool           ()
{
  GObject *pencil_obj = gtk_builder_get_object (bar_gtk_builder, "buttonPencil");
  GtkToggleToolButton *pencil_tool_button = GTK_TOGGLE_TOOL_BUTTON (pencil_obj);

  /* Select the pen as default tool. */
  if (is_eraser_toggle_tool_button_active ())
    {
      GObject *eraser_obj = gtk_builder_get_object (bar_gtk_builder, "buttonEraser");
      GtkToggleToolButton *eraser_tool_button = GTK_TOGGLE_TOOL_BUTTON (eraser_obj);
      gtk_toggle_tool_button_set_active (eraser_tool_button, FALSE);
      gtk_toggle_tool_button_set_active (pencil_tool_button, TRUE);
    }

  if (is_pointer_toggle_tool_button_active ())
    {
      GObject *pointer_obj = gtk_builder_get_object (bar_gtk_builder, "buttonPointer");
      GtkToggleToolButton *pointer_tool_button = GTK_TOGGLE_TOOL_BUTTON (pointer_obj);
      gtk_toggle_tool_button_set_active (pointer_tool_button, FALSE);
      gtk_toggle_tool_button_set_active (pencil_tool_button, TRUE);
    }

  if (is_filler_toggle_tool_button_active ())
    {
      GObject *filler_obj = gtk_builder_get_object (bar_gtk_builder, "buttonFiller");
      GtkToggleToolButton *filler_tool_button = GTK_TOGGLE_TOOL_BUTTON (filler_obj);
      gtk_toggle_tool_button_set_active (filler_tool_button, FALSE);
      gtk_toggle_tool_button_set_active (pencil_tool_button, TRUE);
    }
}


/* Release to lock the mouse */
static void
release_lock                 (BarData *bar_data)
{
  if (bar_data->grab)
    {
      /* Lock enabled. */
      bar_data->grab = FALSE;
      annotate_release_grab ();

      /* Try to up-rise the window. */
      timer = g_timeout_add (BAR_TO_TOP_TIMEOUT, bar_to_top, get_annotation_window());
#ifdef _WIN32 // WIN32
      if (gtk_window_get_opacity (GTK_WINDOW (get_annotation_window ()))!=0)
        {
          /*
           * @HACK This allow the mouse input go below the window putting
           * the opacity to 0; when will be found a better way to make
           * the window transparent to the the pointer input we might
           * remove the previous hack.
           * @TODO Transparent window to the pointer input in a better way.
           */
           gtk_window_set_opacity (GTK_WINDOW (get_annotation_window ()), 0);
        }
#endif

    }
}


/* Lock the mouse. */
static void
lock (BarData *bar_data)
{
  if (! bar_data->grab)
    {
      // Unlock
      bar_data->grab = TRUE;

      /* delete the old timer */
      if (timer!=-1)
        {
          g_source_remove (timer);
          timer = -1;
        }

#ifdef _WIN32 // WIN32

      /*
       * @HACK Deny the mouse input to go below the window putting the opacity greater than 0
       * @TODO remove the opacity hack when will be solved the next todo.
       */
      if (gtk_window_get_opacity (GTK_WINDOW (get_background_window ()))==0)
        {
          gtk_window_set_opacity (GTK_WINDOW (get_background_window ()), BACKGROUND_OPACITY);
        }
#endif
    }
}


/* Set color; this is called each time that the user want change color. */
static void
set_color                    (BarData  *bar_data,
                              gchar    *selected_color)
{
  take_pen_tool ();
  lock (bar_data);
  assert( strlen(selected_color) >= 6  );
  strncpy (bar_data->color, selected_color, 6);
  annotate_set_color (bar_data->color);
}


/* Pass the options to the annotation window. */
static void set_options      (BarData *bar_data)
{

  annotate_set_rectifier (bar_data->rectifier);

  annotate_set_rounder (bar_data->rounder);

  annotate_set_thickness (bar_data->thickness);

  annotate_set_arrow (is_arrow_toggle_tool_button_active ());

  if (is_pen_toggle_tool_button_active ()         ||
      is_highlighter_toggle_tool_button_active () ||
      is_arrow_toggle_tool_button_active ())
    {
      annotate_set_color (bar_data->color);
      annotate_select_pen ();
    }
  else if (is_eraser_toggle_tool_button_active ())
    {
      annotate_select_eraser ();
    }

}


/* Start to paint with the selected tool. */
static void
start_tool                   (BarData *bar_data)
{
  if (bar_data->grab)
    {
        annotate_release_grab (); // release the old cursor
        annotate_acquire_grab (); // grab the pointer again so that button release will respond

      if (is_text_toggle_tool_button_active ())
        {
          /* Text button then start the text widget. */
          start_text_widget (annotation_data->annotation_window,
                             bar_data->color,
                             bar_data->thickness);
        }
      else
        {
            // this call is required as the leave event for the bar occurs
            // when we enter the toolbar object
            stop_text_widget();
          /* Is an other tool for paint or erase. */
          set_options (bar_data);
        }

    }
}



/* Windows state event: this occurs when the windows state changes. */
G_MODULE_EXPORT gboolean
on_bar_window_state_event         (GtkWidget            *widget,
                                   GdkEventWindowState  *event,
                                   gpointer              func_data)
{
  BarData *bar_data = (BarData *) func_data;

  /* Track the minimized signals */
  if(gdk_window_get_state (gtk_widget_get_window (widget)) & GDK_WINDOW_STATE_ICONIFIED)
    {
      release_lock (bar_data);
    }

  return TRUE;
}

G_MODULE_EXPORT gboolean
on_bar_draw_event          (GtkWidget *widget,
                    cairo_t   *cr,
                    gpointer   user_data) {
    //g_print("bar draw event\n");
return FALSE;
}

void
on_bar_hide_event (GtkWidget *widget,
               gpointer   user_data) {
    g_print("bar hide event\n");

    if ( bar_data->screenshot_pending == TRUE ) {

    }
}


/* Configure events occurs. */
G_MODULE_EXPORT gboolean
on_bar_configure_event            (GtkWidget  *widget,
                                   GdkEvent   *event,
                                   gpointer   func_data)
{
    g_print("bar configure event (%d)\n", bar_data->screenshot_pending);

  if ( bar_data->screenshot_pending == TRUE ) {
      // we tried 2 methods:
      //  1. hide causing a hide event or a configure event
      //  2. moving window off screen first and then triggering the hide event
      // both appear to have occurred, but image still captures the window - perhaps
      // a double buffering artifact somewhere.
      //
      // we have to give it some time for the animation to play out
      // otherwise the window will still be visible in our snapshot
      // 2 seconds appears to work, 1 second sometimes works.
      sleep(2);
      g_printf("found a screenshot pending\n");

      GdkPixbuf* buffer = take_screenshot_now();
      bar_data->screenshot_pending = FALSE;
      gdk_window_move( gtk_widget_get_window(get_bar_widget()),
                      bar_data->screenshot_saved_location_x,
                      bar_data->screenshot_saved_location_y);
      bar_data->screenshot_saved_location_x =-1;
      bar_data->screenshot_saved_location_y = -1;
      gtk_widget_show( get_bar_widget() );
      bar_data->screenshot_callback( buffer );
      bar_data->screenshot_callback = NULL;
  } else {
      set_options (bar_data);
  }
  return TRUE;
}


/* Called when push the quit button or the window close action*/
G_MODULE_EXPORT gboolean
on_bar_quit                     (GtkToolButton   *toolbutton,
                                 gpointer         func_data)
{
    g_printf("on_bar_quit\n");
  BarData *bar_data = (BarData *) func_data;

  stop_recorder ();

  bar_data->grab = FALSE;
  /* Release grab. */
  annotate_release_grab ();

  export_iwb (get_iwb_filename ());
  quit_pdf_saver ();
  //start_share_dialog ();

  // handles removal of background and text data structures
  annotate_quit ();

  /* Quit the gtk engine. */
  gtk_main_quit ();
  return FALSE;
}


/* Called when push the info button. */
G_MODULE_EXPORT gboolean
on_bar_info                      (GtkToolButton   *toolbutton,
                                  gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  gboolean grab_value = bar_data->grab;
  bar_data->grab = FALSE;

  /* Release grab. */
  annotate_release_grab ();

  /* Start the info dialog. */
  start_info_dialog (toolbutton, GTK_WINDOW (get_bar_widget ()));

  bar_data->grab = grab_value;
  start_tool (bar_data);
  return TRUE;
}


/* Called when leave the window. */
G_MODULE_EXPORT gboolean
on_bar_leave_notify_event         (GtkWidget       *widget,
                                   GdkEvent        *event,
                                   gpointer func_data)
{
  BarData *bar_data = (BarData *) func_data;
  add_alpha (bar_data);
  start_tool (bar_data);
  return TRUE;
}


/* Called when enter the window. */
G_MODULE_EXPORT gboolean
on_bar_enter_notify_event         (GtkWidget       *widget,
                                   GdkEvent        *event,
                                   gpointer         func_data)
{
  if (is_text_toggle_tool_button_active ())
    {
      stop_text_widget ();
    }
  return TRUE;
}


/* Push pointer button. */
G_MODULE_EXPORT void
on_bar_pointer_activate           (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  release_lock (bar_data);
}



/* Push text button. */
G_MODULE_EXPORT void
on_bar_text_activate              (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  lock (bar_data);
  replace_status_message(gettext("Text tool selected"));
}


/* Push mode button. */
G_MODULE_EXPORT void
on_bar_mode_activate              (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  take_pen_tool ();
  if (!bar_data->rectifier)
    {
      if (!bar_data->rounder)
        {
          /* Select the rounder mode. */
          GObject *rounder_obj = gtk_builder_get_object (bar_gtk_builder, "rounder");
          gtk_tool_button_set_icon_widget (toolbutton, GTK_WIDGET (rounder_obj));
          bar_data->rounder = TRUE;
          bar_data->rectifier = FALSE;
          replace_status_message(gettext("Rounder mode selected"));
        }
      else
        {
          /* Select the rectifier mode. */
          GObject *rectifier_obj = gtk_builder_get_object (bar_gtk_builder, "rectifier");
          gtk_tool_button_set_icon_widget (toolbutton, GTK_WIDGET (rectifier_obj));
          bar_data->rectifier = TRUE;
          bar_data->rounder = FALSE;
          replace_status_message(gettext("Polygon mode selected"));
        }
    }
  else
    {
      /* Select the free hand writing mode. */
      GObject *hand_obj = gtk_builder_get_object (bar_gtk_builder, "hand");
      gtk_tool_button_set_icon_widget (toolbutton, GTK_WIDGET (hand_obj));
      bar_data->rectifier = FALSE;
      bar_data->rounder = FALSE;
      replace_status_message(gettext("Freehand mode selected"));
    }
}


/* Push thickness button. */
G_MODULE_EXPORT void
on_bar_thick_activate             (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;

  if (bar_data->thickness == MICRO_THICKNESS)
    {
    replace_status_message(gettext("Brush thickness set to thin"));
      /* Set the thin icon. */
      GObject *thin_obj = gtk_builder_get_object (bar_gtk_builder, "thin");
      gtk_tool_button_set_icon_widget (toolbutton, GTK_WIDGET (thin_obj));
      bar_data->thickness = THIN_THICKNESS;
    }
  else if (bar_data->thickness == THIN_THICKNESS)
    {
        replace_status_message(gettext("Brush thickness set to medium"));
      /* Set the medium icon. */
      GObject *medium_obj = gtk_builder_get_object (bar_gtk_builder, "medium");
      gtk_tool_button_set_icon_widget (toolbutton, GTK_WIDGET (medium_obj));
      bar_data->thickness = MEDIUM_THICKNESS;
    }
  else if (bar_data->thickness==MEDIUM_THICKNESS)
    {
    replace_status_message(gettext("Brush thickness set to thick"));
      /* Set the thick icon. */
      GObject *thick_obj = gtk_builder_get_object (bar_gtk_builder, "thick");
      gtk_tool_button_set_icon_widget (toolbutton, GTK_WIDGET (thick_obj));
      bar_data->thickness = THICK_THICKNESS;
    }
  else if (bar_data->thickness==THICK_THICKNESS)
    {
        replace_status_message(gettext("Brush thickness set to micro"));
      /* Set the micro icon. */
      GObject *micro_obj = gtk_builder_get_object (bar_gtk_builder, "micro");
      gtk_tool_button_set_icon_widget (toolbutton, GTK_WIDGET (micro_obj));
      bar_data->thickness = MICRO_THICKNESS;
    }

}


/* Push arrow button. */
G_MODULE_EXPORT void
on_bar_arrow_activate             (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{

  BarData *bar_data = (BarData *) func_data;
  lock (bar_data);
  set_color (bar_data, bar_data->color);
  replace_status_message(gettext("Arrow tool selected"));
}


/* Push pencil button. */
G_MODULE_EXPORT void
on_bar_pencil_activate            (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{

  BarData *bar_data = (BarData *) func_data;
  lock (bar_data);
  set_color (bar_data, bar_data->color);
  replace_status_message(gettext("Pencil tool selected"));
}


/* Push highlighter button. */
G_MODULE_EXPORT void
on_bar_highlighter_activate       (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  lock (bar_data);
  set_color (bar_data, bar_data->color);
  replace_status_message(gettext("Highlighter tool selected"));
}


/* Push filler button. */
G_MODULE_EXPORT void
on_bar_filler_activate            (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  lock (bar_data);
  annotate_select_filler ();
  replace_status_message(gettext("Filler tool selected"));
}


/* Push eraser button. */
G_MODULE_EXPORT void
on_bar_eraser_activate            (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  lock (bar_data);
  annotate_select_eraser();
  replace_status_message(gettext("Eraser tool selected"));
}


/* Push save (screen-shoot) button. */
G_MODULE_EXPORT void
on_bar_screenshot_activate	      (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  gboolean grab_value = bar_data->grab;
  bar_data->grab = FALSE;
  /* Release grab. */
  annotate_release_grab ();
  replace_status_message(gettext("Taking screenshot"));
  gdk_window_set_cursor (gtk_widget_get_window (get_annotation_window ()), (GdkCursor *) NULL);
  start_save_image_dialog();
  bar_data->grab = grab_value;
  start_tool (bar_data);
}


/* Add page to pdf. */
G_MODULE_EXPORT void
on_bar_add_pdf_activate	          (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  gboolean grab_value = bar_data->grab;
  bar_data->grab = FALSE;

  /* Release grab. */
  annotate_release_grab ();

replace_status_message(gettext("Exporting as PDF"));
  add_pdf_page (GTK_WINDOW (get_bar_widget ()));
  bar_data->grab = grab_value;
  start_tool (bar_data);
}

/* Hide state event: this occurs when the show/hide widget event happens
 * 2018-08-09 : Added by TM
 */
G_MODULE_EXPORT void
on_bar_showhide_activate     (GtkToolButton            *toolButton,
                                   gpointer              func_data)
{
  BarData *bar_data = (BarData *) func_data;
  gboolean annotation_is_visible = bar_data->annotation_is_visible;

  /* Release grab lock. */
  annotate_release_grab ();
  bar_data->grab = FALSE;

  if ( annotation_is_visible == TRUE ) {
      /** currently annotations are visible so icon is the hidden **/
      GtkWidget* window = get_annotation_window ();
      if ( window != NULL ) {
          replace_status_message(gettext("Annotations hidden"));
          gtk_widget_hide( window ); // @TODO loses its position so we need to save the position and loses image
          bar_data->annotation_is_visible = FALSE;


          /* Set the stop tool-tip. */
          gtk_tool_item_set_tooltip_text ( (GtkToolItem *) toolButton,
                                          gettext ("Show Annotations"));



          /* Put the show icon. */
          GtkImage* icon = get_image_from_builder( gettext("show") );
          gtk_tool_button_set_icon_widget (toolButton, (GtkWidget*) icon);
      }
  } else {
      /** currently annotations are hidden so icon is the showing icon */

      GtkWidget* window = get_annotation_window ();
      if ( window != NULL ) {
          replace_status_message( gettext("Annotations visible") );
          gtk_widget_show( window );
          gtk_window_set_keep_above (GTK_WINDOW (window), TRUE);
          // @TODO move back to position, restore last image

          bar_data->annotation_is_visible = TRUE;

          /* Set the stop tool-tip. */
          gtk_tool_item_set_tooltip_text ( (GtkToolItem *) toolButton,
                                          gettext ("Hide Annotations"));



          /* Put the hide icon. */
          GtkImage* icon = get_image_from_builder( gettext("hide") );
          gtk_tool_button_set_icon_widget (toolButton, (GtkWidget*) icon);

          annotate_acquire_grab ();
      }
  }
}

/* Push recorder button. */
G_MODULE_EXPORT void
on_bar_recorder_activate          (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  gboolean grab_value = bar_data->grab;

  /* Release grab. */
  annotate_release_grab ();


  bar_data->grab = FALSE;

  if (is_started ())
    {
      if (is_paused ())
        {
          resume_recorder ();
          /* Set the stop tool-tip. */
          gtk_tool_item_set_tooltip_text ( (GtkToolItem *) toolbutton, gettext ("Stop"));
          /* Put the stop icon. */
          gtk_tool_button_set_icon_name (toolbutton, "media-playback-stop");
        }
      else
        {
          pause_recorder ();
          /* Set the stop tool-tip. */
          gtk_tool_item_set_tooltip_text ( (GtkToolItem *) toolbutton,
                                          gettext ("Record"));

          /* Put the record icon. */
          gtk_tool_button_set_icon_name (toolbutton, "media-record");
          replace_status_message(gettext("Screen recorder stopped"));
        }
    }
  else
    {

      if (!is_recorder_available ())
        {
          /* Visualize a dialog that informs the user about the missing recorder tool. */
          GObject *recorder_obj = gtk_builder_get_object (bar_gtk_builder,
                                                          "media-recorder-unavailable");

          gdk_window_set_cursor (gtk_widget_get_window (get_annotation_window ()),
                                 (GdkCursor *) NULL);

          visualize_missing_recorder_program_dialog (GTK_WINDOW (get_bar_widget ()),
                gettext ("In order to record with Ardesia you must install the vlc program and add it to the PATH environment variable")
            );
          /* Put an icon that remember that the tool is not available. */
          gtk_tool_button_set_icon_widget (toolbutton, GTK_WIDGET (recorder_obj));
          bar_data->grab = grab_value;
          start_tool (bar_data);
          return;
        }

      gdk_window_set_cursor (gtk_widget_get_window (get_annotation_window ()),
                             (GdkCursor *) NULL);

      replace_status_message(gettext("Starting screen recorder"));
      /* The recording is not active. */
      gboolean status = start_save_video_dialog (toolbutton, GTK_WINDOW (get_bar_widget ()));
      if (status)
        {
          /* Set the stop tool-tip. */
          gtk_tool_item_set_tooltip_text ( (GtkToolItem *) toolbutton, gettext ("Stop"));
          /* Put the stop icon. */
          gtk_tool_button_set_icon_name(toolbutton, gettext("media-playback-stop") );

        }
    }
  bar_data->grab = grab_value;
  start_tool (bar_data);
}


/* Push preference button. */
G_MODULE_EXPORT void
on_bar_preferences_activate	      (GtkToolButton   *toolbutton,
                                       gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  gboolean grab_value = bar_data->grab;
  bar_data->grab = FALSE;
  /* Release grab. */
  annotate_release_grab ();

  gdk_window_set_cursor (gtk_widget_get_window (get_annotation_window ()), (GdkCursor *) NULL);
  start_preference_dialog (GTK_WINDOW (get_bar_widget ()));
  bar_data->grab = grab_value;
  start_tool (bar_data);
}


/* Push undo button. */
G_MODULE_EXPORT void
on_bar_undo_activate              (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  annotate_undo ();
}


/* Push redo button. */
G_MODULE_EXPORT void
on_bar_redo_activate              (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  annotate_redo ();
}


/* Push clear button. */
G_MODULE_EXPORT void
on_bar_clear_activate             (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
    replace_status_message(gettext("Screen has been cleared"));
  annotate_clear_screen ();
}


/* Push colour selector button. */
G_MODULE_EXPORT void
on_bar_color_activate	            (GtkToggleToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  gboolean grab_value = bar_data->grab;
  gchar* new_color = "";

  if (!gtk_toggle_tool_button_get_active (toolbutton))
    {
      return;
    }

  /* Release grab. */
  annotate_release_grab ();

  bar_data->grab = FALSE;
  gdk_window_set_cursor (gtk_widget_get_window (get_annotation_window ()), (GdkCursor *) NULL);
  new_color = start_color_selector_dialog (GTK_TOOL_BUTTON (toolbutton),
                                           GTK_WINDOW (get_bar_widget ()),
                                           bar_data->color);

  if (new_color)  // if it is a valid colour
    {
      set_color (bar_data, new_color);
      g_free (new_color);
    }

  bar_data->grab = grab_value;
  start_tool (bar_data);
}


/* Push blue colour button. */
G_MODULE_EXPORT void
on_bar_blue_activate              (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  set_color (bar_data, BLUE);
}


/* Push red colour button. */
G_MODULE_EXPORT void
on_bar_red_activate               (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  set_color (bar_data, RED);
}


/* Push green colour button. */
G_MODULE_EXPORT void
on_bar_green_activate             (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  set_color (bar_data, GREEN);
}


/* Push yellow colour button. */
G_MODULE_EXPORT void
on_bar_yellow_activate            (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  set_color (bar_data, YELLOW);
}


/* Push white colour button. */
G_MODULE_EXPORT void
on_bar_white_activate             (GtkToolButton   *toolbutton,
                                   gpointer         func_data)
{
  BarData *bar_data = (BarData *) func_data;
  set_color (bar_data, WHITE);
}

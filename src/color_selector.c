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

#include <color_selector.h>
#include <utils.h>
#include <keyboard.h>


/* old picked color in RGBA format */
static gchar *picked_color = NULL;


/*
 * Start the color selector dialog
 * it return the selected color.
 */
gchar *
start_color_selector_dialog       (GtkToolButton  *toolbutton,
                                   GtkWindow      *parent,
                                   gchar          *color)
{
  GtkToggleToolButton *button = GTK_TOGGLE_TOOL_BUTTON (toolbutton);
  gchar *ret_color = NULL;

  start_virtual_keyboard ();

  if (gtk_toggle_tool_button_get_active (button))
    {
      /* Open colour widget. */
      GtkWidget *color_widget = gtk_color_chooser_dialog_new (gettext ("Changing colour"), parent);
      GtkColorChooserDialog *color_dialog = GTK_COLOR_CHOOSER_DIALOG(color_widget);
      gtk_color_chooser_set_use_alpha( GTK_COLOR_CHOOSER(color_dialog), TRUE );

      gint result = -1;
      /* Colour initially selected. */
      GdkRGBA *gdkcolor;

      if (picked_color != NULL)
        {
          gdkcolor = rgba_to_gdkcolor (picked_color);
        }
      else
        {
          gdkcolor = rgba_to_gdkcolor (color);
        }

        gtk_color_chooser_set_rgba( GTK_COLOR_CHOOSER(color_dialog), gdkcolor );

      result = gtk_dialog_run (GTK_DIALOG (color_dialog));

      /* Wait for user to select OK or Cancel. */
      switch (result)
        {
          case GTK_RESPONSE_OK:
            gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER(color_dialog), gdkcolor);
            ret_color = gdkrgba_to_rgba(gdkcolor);

            // reset previously picked color
            g_free (picked_color);
            picked_color = gdkrgba_to_rgba(gdkcolor);
            break;

          default:
            break;
        }

      if (color_widget)
      {
        gtk_widget_destroy (color_widget);
        color_widget = NULL;
        color_dialog = NULL;
      }

      g_free (gdkcolor);

    }

  stop_virtual_keyboard ();
  return ret_color;
}

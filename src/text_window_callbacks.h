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


#ifndef __TEXT_WINDOW_CALLBACKS_H
#define __TEXT_WINDOW_CALLBACKS_H



void
destroy_text_properties( gpointer data );

/* On configure event. */
G_MODULE_EXPORT gboolean
on_text_window_configure     (GtkWidget       *widget,
                              GdkEventExpose  *event,
                              gpointer         user_data);

/* On screen changed. */
G_MODULE_EXPORT void
on_text_window_screen_changed     (GtkWidget  *widget,
                                   GdkScreen  *previous_screen,
                                   gpointer    user_data);


/* The windows has been exposed. */
G_MODULE_EXPORT gboolean
on_text_window_expose_event  (GtkWidget  *widget,
                              cairo_t    *cr,
                              gpointer    data);

/* This is called when the button is leased. */
gboolean
on_text_window_button_release( GtkWidget       *win,
                            GdkEventButton* ev,
                            TextData* data );


/* This shots when the text pointer is moving. */
G_MODULE_EXPORT gboolean
on_text_window_cursor_motion      (GtkWidget       *win,
                                   GdkEventMotion  *ev,
                                   gpointer         func_data);

G_MODULE_EXPORT gboolean
on_text_window_key_press_event (GtkWidget *widget,
                                  GdkEvent  *event,
                                  gpointer   user_data);


#endif

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


#include <text_window.h>
#include <text_window_callbacks.h>
#include <keyboard.h>
#include <utils.h>
#include <annotation_window.h>
#include <bar_callbacks.h>

static void print_text_properties( CharInfo* char_info ) ;


#ifdef _WIN32
/* Is the point (x,y) above the virtual keyboard? */
static gboolean
is_above_virtual_keyboard    (gint  x,
                              gint  y)
{
  RECT rect;
  HWND hwnd = FindWindow (VIRTUALKEYBOARD_WINDOW_NAME, NULL);
  if (!hwnd)
    {
      return FALSE;
    }
  if (!GetWindowRect (hwnd, &rect))
    {
      return FALSE;
    }
  if ( (rect.left<x)&& (x<rect.right)&& (rect.top<y)&& (y<rect.bottom))
    {
      return TRUE;
    }
  return FALSE;
}
#endif


// /* On configure event. */
// G_MODULE_EXPORT gboolean
// on_text_window_configure     (GtkWidget       *widget,
//                               GdkEventExpose  *event,
//                               gpointer         user_data)
// {
//     g_printf("on_text_window_configure\n");
//     init_text_widget (widget);
//     return TRUE;
// }

//
// /* On screen changed. */
// G_MODULE_EXPORT void
// on_text_window_screen_changed     (GtkWidget  *widget,
//                                    GdkScreen  *previous_screen,
//                                    gpointer    user_data)
// {
//   GdkScreen *screen = gtk_widget_get_screen(GTK_WIDGET (widget));
//   GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
//   if (visual == NULL)
//     {
//       visual = gdk_screen_get_system_visual (screen);
//     }
//
//   gtk_widget_set_visual (widget, visual);
// }
//

/* The windows has been exposed. Need Double Buffering to be activated for this to work properly*/
G_MODULE_EXPORT gboolean
on_text_window_expose_event  (GtkWidget  *widget,
                              cairo_t    *cr,
                              gpointer    data)
{
//     g_printf("on_text_window_expose_event: DRAW EVENT\n");
//
//   // this is the draw event and we should be doing all our drawing to the
//   // window in this event
//   if ( text_data->cr ) {
//       g_printf("on_text_window_expose_event: UPDATE WINDOW\n");
//       cairo_save( cr );
//       cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
//       // where we want to copy the image FROM
//       cairo_surface_t* source_surface = cairo_get_target (text_data->cr);
//
//       // position the source surface over the destination
//       cairo_set_source_surface (cr, source_surface, 0, 0);
//       // paint the image on to the window
//       cairo_paint(cr);
//       cairo_restore( cr );
//
//       // remove and create a fresh copy of the latest window
//       cairo_destroy( text_data->cr );
//       text_data->cr = NULL;
//   }
// //  text_data->cr = create_copy_of_text_window_context( cr );
//
//
  return FALSE;
}


/* This is called when the button is leased. */
// G_MODULE_EXPORT gboolean
// on_text_window_button_release     (GtkWidget       *win,
//                                    GdkEventButton  *ev,
//                                    gpointer         user_data)
// {
gboolean
on_text_window_button_release( GtkWidget       *win,
                            GdkEventButton* ev,
                            TextData* data ) {
    g_printf("on_text_window_button_release BEGIN\n");
  /* only button1 allowed */
  if (ev->button != 1)
    {
      return TRUE;
    }

#ifdef _WIN32
  gboolean above = is_above_virtual_keyboard (ev->x_root, ev->y_root);

  if (above)
    {
      /* You have lost the focus; re grab it. */
      grab_pointer (text_data->window, TEXT_MOUSE_EVENTS);
      /* Ignore the data; the event will be passed to the virtual keyboard. */
      return TRUE;
    }

#endif

  if ((text_data) && (text_data->pos))
    {
        g_printf("on_text_window_button_release MOVE CURSOR\n");
      save_text (); // @TODO is this required?
      g_printf("on_text_window_button_release: %f %f %f %f\n", ev->x, ev->y, ev->x_root, ev->y_root);
      text_data->pos->x = ev->x; // x_root
      text_data->pos->y = ev->y; // y_root
      text_config->start_x = ev->x;
      replace_status_message( g_strdup_printf("on_text_window_button_release: text pos: %f %f", text_data->pos->x, text_data->pos->y));

      /* This present the ardesia bar and the panels. */
      gtk_window_present (GTK_WINDOW (get_bar_widget ()));
      gtk_window_present (GTK_WINDOW (annotation_data->annotation_window));
      gdk_window_raise (gtk_widget_get_window  (annotation_data->annotation_window));

      stop_virtual_keyboard ();
      start_virtual_keyboard ();

      //text_data->timer = g_timeout_add (1000, blink_cursor, NULL);
      start_blink_cursor();
    }
g_printf("on_text_window_button_release END\n");
  return TRUE;
}


/* This shots when the text pointer is moving. */
G_MODULE_EXPORT gboolean
on_text_window_cursor_motion      (GtkWidget       *win,
                                   GdkEventMotion  *ev,
                                   gpointer         func_data)
{
#ifdef _WIN32
  if (inside_bar_window (ev->x_root, ev->y_root))
    {
      stop_text_widget ();
    }
#endif
  return TRUE;
}



static CharInfo*
make_new_character() {
    CharInfo *char_info = g_malloc ( (gsize) sizeof (CharInfo));
    if ( char_info == NULL ) {
        g_error("failed to create new character object\n");
        return NULL;
    }
    return char_info;
}

static void
draw_character( cairo_t* cr, CharInfo* char_info ) {

    //guint r,g,b,a;
    guint br,bg,bb,ba;
    gint weight = 1;

    if ( cr ) {
        if ( char_info->bold ) {
            weight *= 15;
        } else {
            weight *= 5;
        }

        //sscanf( char_info->color, "%02X%02X%02X%02X", &r, &g, &b, &a);
        if ( char_info->background_color != NULL ) {
            sscanf( char_info->background_color, "%02X%02X%02X%02X", &br, &bg, &bb, &ba);
        }

        cairo_save(cr);
        g_printf("[DRAW] Drawing character at %f %f %s %s\n", char_info->x, char_info->y, char_info->color, char_info->font_family);

        cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
        //cairo_set_source_rgba( cr, r, g, b, a);

        cairo_set_line_width (cr, char_info->pen_width);
        cairo_set_source_color_from_string (cr, char_info->color);
        // cairo_set_font_size (cr, char_info->font_size);

        /* Select the font */
        // cairo_select_font_face (cr, char_info->font_family,
        //                         CAIRO_FONT_SLANT_NORMAL,
        //                         CAIRO_FONT_WEIGHT_NORMAL);

        // uses the Pango Layout interface
        PangoLayout* layout = pango_cairo_create_layout( cr );
        pango_layout_set_font_description( layout, char_info->pango_font_description );
        pango_layout_set_text( layout, char_info->character, -1 );


        // match the pango layout with the cairo object and any transformation
        pango_cairo_update_layout( cr, layout );

        gint text_width, text_height;
        pango_layout_get_pixel_size( layout, &text_width, &text_height );
        gint baseline = pango_layout_get_baseline( layout );
        cairo_move_to (cr, char_info->x, char_info->y - (baseline / PANGO_SCALE) );

        // draw layout on cairo context
        pango_cairo_show_layout( cr, layout );
        char_info->text_width = text_width;
        char_info->text_height = text_height;
        char_info->baseline = baseline;
        //cairo_show_text (cr, char_info->character);
        //cairo_text_extents (cr, char_info->character, &char_info->extents); // gets the bounding box of the non-whitespace characters
        //cairo_stroke (cr);
        cairo_surface_flush( cairo_get_target(cr) );
        cairo_restore(cr);

        gtk_widget_queue_draw(annotation_data->annotation_window);

        print_text_properties( char_info );
    }
}

static gboolean
is_delete_char(int ch) {
    return (ch == GDK_KEY_BackSpace) ||
         (ch == GDK_KEY_Delete);
}

static gboolean
is_tab_char(int ch ) {
    return ch == GDK_KEY_Tab;
}

static gboolean
is_return_char(int ch) {
    return (ch == GDK_KEY_Return) ||
	    (ch == GDK_KEY_ISO_Enter) ||
	    (ch == GDK_KEY_KP_Enter);
}

static void
print_text_properties( CharInfo* char_info ) {
    g_printf("Character: %s\n", char_info->character);
    g_printf("Position: %f, %f\n", char_info->x, char_info->y);
    g_printf("Bearing: %f, %f\n", char_info->extents.x_bearing, char_info->extents.y_bearing);
    g_printf("Advance: %f, %f\n", char_info->extents.x_advance, char_info->extents.y_advance);
    g_printf("Pen Width: %d\n", char_info->pen_width);
    g_printf("Color: %s\n", char_info->color);
    g_printf("Font Family: %s\n", char_info->font_family);
    g_printf("Italics: %d\n", char_info->italics);
    g_printf("Weight: %d\n", char_info->font_weight);
    g_printf("Background Color: %s\n", char_info->background_color);
}

static void
assign_text_properties( CharInfo* char_info ) {
    char_info->x = text_data->pos->x;
    char_info->y = text_data->pos->y;
    char_info->extents.x_bearing = text_data->extents.x_bearing;
    char_info->extents.y_bearing = text_data->extents.y_bearing;
    char_info->pen_width = text_data->pen_width;
    char* copy = g_strdup_printf("%s", text_data->color);
    char_info->color = copy;
    char_info->italics = CAIRO_FONT_SLANT_NORMAL;
    char_info->font_weight = CAIRO_FONT_WEIGHT_NORMAL;
    char_info->background_color = NULL;

    if ( annotation_data->font == NULL ) {
        char_info->pango_font_description = NULL;
        copy = g_strdup_printf("%s", text_config->fontfamily);
        char_info->font_family = copy;
        char_info->font_size=32;
    } else {
        char_info->pango_font_description = annotation_data->font;
        char_info->font_family = g_strdup_printf("%s", pango_font_description_get_family( annotation_data->font ) );
        char_info->font_size = pango_font_description_get_size( annotation_data->font ) /  PANGO_SCALE;
        g_printf("font: %s %d\n", char_info->font_family, char_info->font_size );
    }
}

void
destroy_text_properties( gpointer data ) {
    CharInfo* char_info = (CharInfo*) data;
    g_free( char_info->color );
    g_free( char_info->font_family );
    g_free( char_info->background_color );
}

/* Delete the last character printed. */
static void
delete_character        ()
{
    if ( text_data->cr ) {
        CharInfo *char_info = (CharInfo *) g_slist_nth_data (text_data->letterlist, 0);
        if (char_info)
        {
            if ( g_strcmp0(char_info->character,"\n") == 0 ) {
                // when deleting return we just move the cursor back to where we started
            } else {
                cairo_save( text_data->cr );
                cairo_set_operator (text_data->cr, CAIRO_OPERATOR_CLEAR);
                if ( char_info->pango_font_description == NULL ) {
                    cairo_rectangle (text_data->cr,
                                    char_info->x + char_info->extents.x_bearing,
                                    char_info->y + char_info->extents.y_bearing,
                                    char_info->extents.width, //text_data->pos->x - char_info->x,
                                    char_info->extents.height);
                } else {
                    cairo_rectangle (text_data->cr,
                                    char_info->x, //+ char_info->text_width,
                                    char_info->y - (char_info->baseline / PANGO_SCALE), //+ char_info->text_height,
                                    char_info->text_width, //text_data->pos->x - char_info->x,
                                    char_info->text_height);
                }
                cairo_fill (text_data->cr); // fill inner piece of rectangle
                cairo_stroke( text_data->cr); // draw border
                cairo_restore( text_data->cr );
            }
            text_data->pos->x = char_info->x;
            text_data->pos->y = char_info->y;
            destroy_text_properties( char_info );
            text_data->letterlist = g_slist_remove (text_data->letterlist, char_info);
        }
    }
}

static void
handle_delete_char() {
    delete_character (); // undo the last character inserted
}

static void
handle_return_char() {
    /* select the x indentation */
    CharInfo* char_info = make_new_character();
    char_info->character = "\n";
    assign_text_properties( char_info );
    text_data->letterlist = g_slist_prepend (text_data->letterlist, char_info);
    CharInfo* last = NULL;
    if ( g_slist_length(text_data->letterlist) > 0 ) {
        last = (CharInfo*) ( g_slist_last(text_data->letterlist)->data );
    }
    // move down and to underneath where user started this bit of text
    text_data->pos->x = text_config->start_x + text_config->leftmargin;
    if ( last != NULL ) {
        text_data->pos->y += last->text_height + 5;
    } else {
        text_data->pos->y +=  text_data->max_font_height;
    }
}

static void
handle_tab_char(/* arguments */) {
    /* Simple Tab-Implementation */
    CharInfo* char_info = make_new_character();
    char_info->character = "\t";
    assign_text_properties( char_info );
    text_data->letterlist = g_slist_prepend (text_data->letterlist, char_info);

    // move cursor along by tab size
    text_data->pos->x += text_config->tabsize;
}

static void
handle_printable_char(char ch) {
    /* Is the character printable? */
    CharInfo* char_info = make_new_character();
    /* Postcondition: the character is printable. */
    char_info->character = g_strdup_printf ("%c", ch);
    assign_text_properties( char_info );
    text_data->letterlist = g_slist_prepend (text_data->letterlist, char_info);

    draw_character( text_data->cr, char_info );

    /* Move cursor to the x step */
    if ( char_info->pango_font_description == NULL ) {
        text_data->pos->x +=  char_info->extents.x_advance;
    } else {
        text_data->pos->x += char_info->text_width;
    }
}


G_MODULE_EXPORT gboolean
on_text_window_key_press_event (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   user_data) {
    GdkEventKey* keyEvent = (GdkEventKey*) event;
    g_printf("on key press event for text window %d\n", keyEvent->keyval);
    if ( annotation_data->font != NULL ) {
        g_printf("PANGO FONT SELECTED\n");
    }
    if (event->type != GDK_KEY_PRESS) {
        return TRUE;
    }

    stop_blink_cursor();
    gboolean closed_to_bar = inside_bar_window (text_data->pos->x + text_data->extents.x_advance,
                                                text_data->pos->y-text_data->max_font_height/2);
    int width = gtk_widget_get_allocated_width( GTK_WIDGET( annotation_data->annotation_window ) );
    // int height = gtk_widget_get_allocated_width(text_data->window);

    if ( is_delete_char(keyEvent->keyval) ) {
        handle_delete_char();
    }
    /* It is the end of the line or the letter is closed to the window bar. */
    else if ( (text_data->pos->x + text_data->extents.x_advance >= width) ||
	    (closed_to_bar) ||
        is_return_char(keyEvent->keyval) )
    {
      handle_return_char();
    }
    else if ( is_tab_char(keyEvent->keyval) ) {
        handle_tab_char();
    }
    else if ( isprint(keyEvent->keyval) ) {
        handle_printable_char(keyEvent->keyval);
    }

    replace_status_message( g_strdup_printf("text pos: %f %f", text_data->pos->x, text_data->pos->y));

    start_blink_cursor();
    return TRUE;
}

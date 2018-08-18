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

#include <cairo_functions.h>
#include <utils.h>

/**
 * Paint from one context to another
 * @param source the context on which we want to paint
 * @param dest   the context which has our pattern on it
 */
void
draw_cairo_context (cairo_t* dest, cairo_t* source) {
    if ( source && dest ) {
        cairo_save( dest );
        cairo_set_operator(dest, CAIRO_OPERATOR_OVER);
        // where we want to copy the image FROM
        cairo_surface_t* pattern = cairo_get_target (source);

        // position the source surface over the destination
        cairo_set_source_surface (dest, pattern, 0, 0);
        // paint the image on to the window
        cairo_paint(dest);
        cairo_restore( dest );
    }
}


cairo_surface_t*
scale_image( gchar* image, gint new_width, gint new_height ) {
    cairo_surface_t *surface = cairo_image_surface_create_from_png (image);
    cairo_t *cr = cairo_create (surface);
    cairo_surface_t *scaled_surface = scale_surface (surface, new_width, new_height );
    cairo_surface_destroy (surface);
    cairo_destroy (cr);
    return scaled_surface;
}


/* Load a file image in the window. */
void
load_file_onto_context(gchar* image_filename, cairo_t* cr)
{
    g_printf("attempting to load file\n");
    if (cr) {
        gint new_height = 0;
        gint new_width = 0;
        get_context_size (cr, &new_width, &new_height);
        cairo_surface_t* scaled_surface = scale_image( image_filename, new_width, new_height);
        cairo_set_source_surface (cr, scaled_surface, 0.0, 0.0);

        cairo_save( cr );
        cairo_paint (cr);
        cairo_stroke (cr);
        cairo_surface_flush( cairo_get_target(cr) );
        cairo_surface_destroy (scaled_surface);
        cairo_restore( cr );
    } else {
        g_printf("no background_window cairo context found\n");
    }
}




/* The windows has been exposed after the show_all request to change the background color. */
void
load_color_onto_context(gchar* hex_color, cairo_t* cr)
{
    g_printf("%s\n", hex_color);
    assert(hex_color);
    assert(cr);
    assert(strlen(hex_color) == 8 );

    g_printf("load_color\n");
  gint r = 0;
  gint g = 0;
  gint b = 0;
  gint a = 0;

  if (cr)
    {
        sscanf (hex_color, "%02X%02X%02X%02X", &r, &g, &b, &a);
        cairo_save( cr );
        cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);


// #ifdef WIN32
//       gdouble opacity = BACKGROUND_OPACITY;
//       cairo_set_source_rgb (cr, (gdouble) r/256, (gdouble) g/256, (gdouble) b/256);
//
//       /*
//        * @TODO Implement with a full opaque windows and use cairo_set_source_rgba
//        * function to paint.
//        * I set the opacity with alpha and I use cairo_set_source_rgb to workaround
//        * the problem on windows with rgba.
//        */
//        if (((gdouble) a/256) >  BACKGROUND_OPACITY)
//          {
//            opacity = (gdouble) a/256;
//          }
//       gtk_window_set_opacity (GTK_WINDOW (annotation_data->annotation_window), opacity);
// #else
      cairo_set_source_rgba (cr,
                             (gdouble) r/256,
                             (gdouble) g/256,
                             (gdouble) b/256,
                             (gdouble) a/256);
// #endif

      cairo_paint (cr);
      cairo_stroke (cr);
      cairo_restore( cr );

// save_cairo_context(cr, "/tmp", "debug_color", 0);
    }
}

cairo_t*
create_new_context(int width, int height) {
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height );
    return cairo_create(surface);
}

cairo_t*
create_copy_of_context(cairo_t* current_context) {
    if ( current_context == NULL ) {
        int width = 0;
        int height = 0;
        get_context_size( current_context, &width, &height );
        cairo_surface_t* dest_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height );
        cairo_surface_t* source_surface = cairo_get_target (current_context);
        cairo_t* dest_cr = cairo_create (dest_surface);
        cairo_set_operator (dest_cr, CAIRO_OPERATOR_SOURCE);
        cairo_set_source_surface (dest_cr, source_surface, 0, 0);
        cairo_paint (dest_cr);
        return dest_cr;
    }
    return NULL;
}

void
draw_test_text(cairo_t* cr, gchar* text) {
    // test text
    cairo_save(cr);
    // paint context white
    //

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    // write near black text
    cairo_select_font_face (cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, 32.0);
    cairo_set_source_rgb (cr, 0.1, 0.1, 0.1);
    cairo_move_to (cr, 10.0, 50.0);
    cairo_show_text (cr, text);
    cairo_restore(cr);
}

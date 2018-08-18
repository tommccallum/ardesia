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


#include <input.h>


/* Add input device. */
static void
add_input_mode_device   (AnnotateData    *data,
                         GdkDevice       *device,
                         GdkInputMode     mode)
{
  if (!data->devdatatable)
    {
      data->devdatatable = g_hash_table_new (NULL, NULL);
    }

  AnnotateDeviceData *devdata = (AnnotateDeviceData *) NULL;
  devdata  = g_malloc ((gsize) sizeof (AnnotateDeviceData));
  devdata->coord_list = (GSList *) NULL;
  g_hash_table_insert (data->devdatatable, device, devdata);

  if (!gdk_device_set_mode (device, mode))
    {
      g_warning ("Unable to set the device %s to the %d mode\n",
                  gdk_device_get_name (device),
                  mode);
    }

  g_printerr ("Enabled Device in mode %s. Device: %p: \"%s\" (Type: %d)\n",
               mode == GDK_MODE_SCREEN ? "SCREEN" : "WINDOW",
               device,
               gdk_device_get_name (device),
               gdk_device_get_source (device));
}


/* Set-up input device list. */
static void
setup_input_device_list (AnnotateData  *data,
                         GList         *devices)
{
  remove_input_devices (data);
  g_list_foreach (devices, (GFunc) add_input_device, data);
}


/* Select the preferred input mode depending on axis. */
static GdkInputMode
select_input_device_mode     (GdkDevice     *device)
{
  if (gdk_device_get_source (device) != GDK_SOURCE_KEYBOARD && gdk_device_get_n_axes (device) >= 2)
    {
      /* Choose screen mode. */
      g_printf("Selecting GDK_MODE_SCREEN (%d)\n",GDK_MODE_SCREEN);
      return GDK_MODE_SCREEN;
    }
  else
    {
      /* Choose window mode. */
      g_printf("Selecting GDK_MODE_WINDOW (%d)\n",GDK_MODE_WINDOW);
      return GDK_MODE_WINDOW;
    }
}


/* Remove all the devices . */
void
remove_input_devices    (AnnotateData  *data)
{
  if (data->devdatatable)
    {
      GList* list = (GList *) NULL;
      list = g_hash_table_get_keys (data->devdatatable);
      g_list_foreach (list, (GFunc) remove_input_device, data);
      data->devdatatable = (GHashTable *) NULL;
    }
}

int deviceIndex =0;
void
print_device_info( GdkDevice * device ) {
    g_printf("Device %d: Name : %s\n", deviceIndex, gdk_device_get_name(device) );
    if ( gdk_device_get_device_type(device) != GDK_DEVICE_TYPE_MASTER ) {
        g_printf("Device %d: Vendor ID : %s\n", deviceIndex, gdk_device_get_vendor_id(device));
        g_printf("Device %d: Product ID : %s\n", deviceIndex, gdk_device_get_product_id(device));
    }
    if ( gdk_device_get_source(device) != GDK_SOURCE_KEYBOARD ) {
        g_printf("Device %d: Number of Axes : %d\n", deviceIndex, gdk_device_get_n_axes (device));
    }
    g_printf("Device %d: Source : %d\n", deviceIndex, gdk_device_get_source(device) );
    switch ( gdk_device_get_source(device)) {
        case 0:
        g_printf("Device %d: Source Type : %s\n", deviceIndex, "Mouse" );
        break;
        case 4:
        g_printf("Device %d: Source Type : %s\n", deviceIndex, "Keyboard" );
        break;
        default:
        g_printf("Device %d: Source Type : %s\n", deviceIndex, "Unknown" );
        break;
    }

    deviceIndex++;
}

/* Set-up input devices.
 * Entry point from annotation_window::annotate_init
 */
void
setup_input_devices     (AnnotateData  *data)
{
  GList* devices = NULL;
  GdkSeat *seat = gdk_display_get_default_seat(gdk_display_get_default ());

  GdkDevice *master = gdk_seat_get_pointer(seat );
  devices = g_list_append( devices, master );
  GList *slavers = gdk_seat_get_slaves (seat,  GDK_SEAT_CAPABILITY_ALL_POINTING);
  devices = g_list_concat(devices, slavers);
  g_assert( g_list_length( devices ) > 0 );
  // write out the devices
  deviceIndex = 0;
  g_list_foreach( devices, (GFunc) print_device_info, NULL );

  setup_input_device_list (data, devices);
}


/* Add input device. */
void
add_input_device        (GdkDevice     *device,
                         AnnotateData  *data)
{
  /* only enable devices with 2 or more axes and exclude keyboards */
  if ((gdk_device_get_source(device) != GDK_SOURCE_KEYBOARD) &&
      ( gdk_device_get_n_axes (device) >= 2))
    {
      add_input_mode_device (data, device, select_input_device_mode (device));
    }
}


/* Remove input device. */
void
remove_input_device     (GdkDevice     *device,
                         AnnotateData  *data)
{
  if (data)
    {
      AnnotateDeviceData *devdata = g_hash_table_lookup (data->devdatatable, device);
      annotate_coord_dev_list_free (devdata);
      g_hash_table_remove (data->devdatatable, device);
    }
}


/* Grab pointer. */
void
grab_pointer       (GtkWidget           *widget,
                    GdkEventMask         eventmask)
{
  GdkGrabStatus result;
  GdkDisplay    *display = (GdkDisplay *) NULL;
  //GdkDevice     *pointer = (GdkDevice *) NULL;
  GdkSeat *device_manager = (GdkSeat *) NULL;

  display = gdk_display_get_default ();
  ungrab_pointer     (display);
  //device_manager = gdk_display_get_device_manager (display);
  //pointer = gdk_device_manager_get_client_pointer (device_manager);
  device_manager = gdk_display_get_default_seat (display);
  // pointer = gdk_seat_get_pointer (device_manager);

  gdk_error_trap_push ();

  result = gdk_seat_grab (device_manager,
                            gtk_widget_get_window(widget),
                            GDK_SEAT_CAPABILITY_ALL_POINTING,
                            TRUE,
                            NULL,
                            NULL,
                            NULL,
                            NULL);


                            // pointer,
                            // gtk_widget_get_window (widget),
                            // GDK_OWNERSHIP_WINDOW,
                            // TRUE,
                            // eventmask,
                            // NULL,
                            // GDK_CURRENT_TIME);

  gdk_flush ();
  if (gdk_error_trap_pop ())
    {
      g_printerr ("Grab pointer error\n");
    }

  switch (result)
    {
    case GDK_GRAB_SUCCESS:
      break;
    case GDK_GRAB_ALREADY_GRABBED:
      g_printerr ("Grab Pointer failed: AlreadyGrabbed\n");
      break;
    case GDK_GRAB_INVALID_TIME:
      g_printerr ("Grab Pointer failed: GrabInvalidTime\n");
      break;
    case GDK_GRAB_NOT_VIEWABLE:
      g_printerr ("Grab Pointer failed: GrabNotViewable\n");
      break;
    case GDK_GRAB_FROZEN:
      g_printerr ("Grab Pointer failed: GrabFrozen\n");
      break;
    default:
      g_printerr ("Grab Pointer failed: Unknown error\n");
    }

}


/* Ungrab pointer. */
void
ungrab_pointer     (GdkDisplay        *display)
{
  // GdkDevice     *pointer = (GdkDevice *) NULL;
  // GdkDeviceManager *device_manager = (GdkDeviceManager *) NULL;
  GdkSeat *seat = (GdkSeat *) NULL;

  display = gdk_display_get_default ();
  // device_manager = gdk_display_get_device_manager (display);
  // pointer = gdk_device_manager_get_client_pointer (device_manager);
  seat = gdk_display_get_default_seat (display);

  gdk_error_trap_push ();

  // gdk_device_ungrab (pointer, GDK_CURRENT_TIME);
  gdk_seat_ungrab( seat );
  gdk_flush ();
  if (gdk_error_trap_pop ())
    {
      /* this probably means the device table is outdated,
       * e.g. this device doesn't exist anymore.
       */
      g_printerr ("Ungrab pointer device error\n");
    }
}

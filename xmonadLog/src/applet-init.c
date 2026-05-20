/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


/* D-Bus related functions come from xmonad-log-applet
 *
 * Copyright (c) 2009 Adam Wick
 * Copyright (c) 2011-2012 Alexander Kojevnikov
 * Copyright (c) 2011 Dan Callaghan
 * Copyright (c) 2012 Ari Croock
 *
 *
 */


/*  Text rendering comes from
http://x11.gp2x.de/personal/google/

*/


#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"

#include <dbus/dbus-glib.h>


void rendertext(cairo_t *cr,const char* msg) {
	PangoLayout *layout;							// layout for a paragraph of text
	//PangoFontDescription *desc;						// this structure stores a description of the style of font you'd most like
        cairo_identity_matrix (cr);
        cairo_translate(cr,myConfig.iTextDx ,myConfig.iTextDy);	
        //printf("translating to %d,%d\n",myConfig.iTextDx ,myConfig.iTextDy);	
        // set the origin of cairo instance 'cr' to (10,20) (i.e. this is where
										// drawing will start from).
	layout = pango_cairo_create_layout(cr);					// init pango layout ready for use
	pango_layout_set_markup(layout, msg, -1);			// sets the text to be associated with the layout (final arg is length, -1
										// to calculate automatically when passing a nul-terminated string)
	//desc = pango_font_description_from_string("Sans Bold 8");		// specify the font that would be ideal for your particular use
	//pango_layout_set_font_description(layout, desc);			// assign the previous font description to the layout
	//pango_font_description_free(desc);					// free the description

	cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);				// set the colour to blue
	pango_cairo_update_layout(cr, layout);					// if the target surface or transformation properties of the cairo instance
										// have changed, update the pango layout to reflect this
	pango_cairo_show_layout(cr, layout);					// draw the pango layout onto the cairo surface

	g_object_unref(layout);							// free the layout
}





static void signal_handler(DBusGProxy *obj, const char *msg)
{
  CD_APPLET_START_DRAWING_MY_ICON_OR_RETURN_CAIRO ();

  //printf("received %s\n",msg);
  rendertext(myDrawContext,msg);
  
  CD_APPLET_FINISH_DRAWING_MY_ICON_CAIRO;
}



static void set_up_dbus_transfer()
{
    DBusGConnection *connection;
    DBusGProxy *proxy;
    GError *error= NULL;

    connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
    if(connection == NULL) {
      g_printerr("Failed to open connection: %s\n", error->message);
      g_error_free(error);
    }

    proxy = dbus_g_proxy_new_for_name(
        connection, "org.xmonad.Log", "/org/xmonad/Log", "org.xmonad.Log");
    error = NULL;

    dbus_g_proxy_add_signal(proxy, "Update", G_TYPE_STRING, G_TYPE_INVALID);
    dbus_g_proxy_connect_signal(proxy, "Update", (GCallback)signal_handler, NULL, NULL);
    //    printf("should be connected");
}



CD_APPLET_DEFINITION (N_("xmonadLog"),
	3, 3, 2,
	CAIRO_DOCK_CATEGORY_APPLET_ACCESSORY,
	N_("Xmonad D-Bus Log printer\n"
	"cairo-dock equivalent of xmonad-log-applet"),
	"fabiodl")


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
                CD_APPLET_ALLOW_NO_CLICKABLE_DESKLET;
	}
	
	
	/// To be continued ...
	
        set_up_dbus_transfer();
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	
	/// To be continued ...
	
	
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		
		/// To be continued ...
		
	}
CD_APPLET_RELOAD_END

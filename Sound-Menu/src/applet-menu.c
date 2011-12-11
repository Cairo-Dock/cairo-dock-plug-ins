/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* based on indicator-messages.c written by :
*  Ted Gould <ted@canonical.com>
*  Cody Russell <cody.russell@canonical.com>
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

#include <stdlib.h>
#include <string.h>

#include <gdk/gdkkeysyms.h>

#include "applet-struct.h"
#include "applet-menu.h"
#include "transport-widget.h"
#include "volume-widget.h"
#include "voip-input-widget.h"
#include "title-widget.h"
#include "metadata-widget.h"

#if (GTK_MAJOR_VERSION < 3)
#include <libdbusmenu-gtk/menuitem.h>
#include <libdbusmenu-gtk/menu.h>
#else
#include <libdbusmenu-gtk3/menuitem.h>
#include <libdbusmenu-gtk3/menu.h>
#endif

#if (GTK_MAJOR_VERSION > 2 || GTK_MINOR_VERSION > 20)
#include <gdk/gdkkeysyms-compat.h>
#endif

#include <libido/libido.h>

static gboolean
new_transport_widget (DbusmenuMenuitem * newitem,
                      DbusmenuMenuitem * parent,
                      DbusmenuClient * client,
                      gpointer user_data)
{
  g_debug("indicator-sound: new_transport_bar() called ");

  GtkWidget* bar = NULL;

  g_return_val_if_fail(DBUSMENU_IS_MENUITEM(newitem), FALSE);
  g_return_val_if_fail(DBUSMENU_IS_GTKCLIENT(client), FALSE);

  bar = transport_widget_new(newitem);
  myData.transport_widgets_list = g_list_append ( myData.transport_widgets_list, bar );

  GtkMenuItem *menu_transport_bar = GTK_MENU_ITEM(bar);

  gtk_widget_show_all(bar);
  dbusmenu_gtkclient_newitem_base (DBUSMENU_GTKCLIENT(client),
                                   newitem,
                                   menu_transport_bar,
                                   parent);
  return TRUE;
}

static gboolean
new_metadata_widget (DbusmenuMenuitem * newitem,
                     DbusmenuMenuitem * parent,
                     DbusmenuClient * client,
                     gpointer user_data)
{
  g_debug("indicator-sound: new_metadata_widget");

  GtkWidget* metadata = NULL;

  g_return_val_if_fail(DBUSMENU_IS_MENUITEM(newitem), FALSE);
  g_return_val_if_fail(DBUSMENU_IS_GTKCLIENT(client), FALSE);

  metadata = metadata_widget_new (newitem);
  GtkMenuItem *menu_metadata_widget = GTK_MENU_ITEM(metadata);

  gtk_widget_show_all(metadata);
  dbusmenu_gtkclient_newitem_base (DBUSMENU_GTKCLIENT(client),
                                   newitem, menu_metadata_widget, parent);
  return TRUE;
}

static gboolean
new_title_widget(DbusmenuMenuitem * newitem,
                 DbusmenuMenuitem * parent,
                 DbusmenuClient * client,
                 gpointer user_data)
{
  g_return_val_if_fail(DBUSMENU_IS_MENUITEM(newitem), FALSE);
  g_return_val_if_fail(DBUSMENU_IS_GTKCLIENT(client), FALSE);

  g_debug ("%s (\"%s\")", __func__, dbusmenu_menuitem_property_get(newitem, DBUSMENU_TITLE_MENUITEM_NAME));

  GtkWidget* title = NULL;

  title = title_widget_new (newitem);
  GtkMenuItem *menu_title_widget = GTK_MENU_ITEM(title);
  
  gtk_widget_show_all(title);

  dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client),
                                  newitem,
                                  menu_title_widget, parent); 
  return TRUE;
}

static gboolean
new_volume_slider_widget(DbusmenuMenuitem * newitem,
                         DbusmenuMenuitem * parent,
                         DbusmenuClient * client,
                         gpointer user_data)
{
  g_print ("++++ indicator-sound: new_volume_slider_widget\n");

  GtkWidget* volume_widget = NULL;

  g_return_val_if_fail(DBUSMENU_IS_MENUITEM(newitem), FALSE);
  g_return_val_if_fail(DBUSMENU_IS_GTKCLIENT(client), FALSE);

  if (myData.volume_widget != NULL){ 
    volume_widget_tidy_up (myData.volume_widget);
    gtk_widget_destroy (myData.volume_widget);
    myData.volume_widget = NULL;
  }
  volume_widget = volume_widget_new (newitem);
  myData.volume_widget = volume_widget;

  GtkWidget* ido_slider_widget = volume_widget_get_ido_slider(VOLUME_WIDGET(myData.volume_widget));

  gtk_widget_show_all(ido_slider_widget);
  // register the style callback on this widget with state manager's style change
  // handler (needs to remake the blocking animation for each style).
  /**g_signal_connect (ido_slider_widget, "style-set",
                    G_CALLBACK(sound_state_manager_style_changed_cb),
                    myApplet.state_manager);*/

  GtkMenuItem *menu_volume_item = GTK_MENU_ITEM(ido_slider_widget);
  dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client),
                                  newitem,
                                  menu_volume_item,
                                  parent);
  return TRUE;
}
/**
 * new_voip_slider_widget
 * Create the voip menu item widget, must of the time this widget will be hidden.
 * @param newitem
 * @param parent
 * @param client
 * @return
 */
static gboolean
new_voip_slider_widget (DbusmenuMenuitem * newitem,
                        DbusmenuMenuitem * parent,
                        DbusmenuClient * client,
                        gpointer user_data)
{
  g_debug("indicator-sound: new_voip_slider_widget");
  GtkWidget* voip_widget = NULL;

  g_return_val_if_fail(DBUSMENU_IS_MENUITEM(newitem), FALSE);
  g_return_val_if_fail(DBUSMENU_IS_GTKCLIENT(client), FALSE);

  if (myData.voip_widget != NULL){ 
    voip_input_widget_tidy_up (myData.voip_widget);
    gtk_widget_destroy (myData.voip_widget);
    myData.voip_widget = NULL;
  }

  voip_widget = voip_input_widget_new (newitem);
  myData.voip_widget = voip_widget;

  GtkWidget* ido_slider_widget = voip_input_widget_get_ido_slider(VOIP_INPUT_WIDGET(voip_widget));

  gtk_widget_show_all(ido_slider_widget);

  GtkMenuItem *menu_volume_item = GTK_MENU_ITEM(ido_slider_widget);
  dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client),
                                  newitem,
                                  menu_volume_item,
                                  parent);
  return TRUE;
}

/*******************************************************************/
//UI callbacks
/******************************************************************/

/**
key_press_cb:
**/
static gboolean
key_press_cb(GtkWidget* widget, GdkEventKey* event, CairoDockModuleInstance *myApplet)
{
  gboolean digested = FALSE;

  GtkWidget *menuitem;
  
	#if (GTK_MAJOR_VERSION < 3)
	menuitem = GTK_MENU_SHELL (widget)->active_menu_item;
	#else
	menuitem = gtk_menu_shell_get_selected_item (GTK_MENU_SHELL (widget));
	#endif

  if (IDO_IS_SCALE_MENU_ITEM(menuitem) == TRUE){
    gdouble current_value = 0;
    gdouble new_value = 0;
    const gdouble five_percent = 5;
    gboolean is_voip_slider = FALSE;

    if (g_ascii_strcasecmp (ido_scale_menu_item_get_primary_label (IDO_SCALE_MENU_ITEM(menuitem)), "VOLUME") == 0) {
      g_debug ("vOLUME SLIDER KEY PRESS");
      GtkWidget* slider_widget = volume_widget_get_ido_slider(VOLUME_WIDGET(myData.volume_widget));
      GtkWidget* slider = ido_scale_menu_item_get_scale((IdoScaleMenuItem*)slider_widget);
      GtkRange* range = (GtkRange*)slider;
      g_return_val_if_fail(GTK_IS_RANGE(range), FALSE);
      current_value = gtk_range_get_value(range);
      new_value = current_value;
    }
    else if (g_ascii_strcasecmp (ido_scale_menu_item_get_primary_label (IDO_SCALE_MENU_ITEM(menuitem)), "VOIP") == 0) {
      g_debug ("VOIP SLIDER KEY PRESS");
      GtkWidget* slider_widget = voip_input_widget_get_ido_slider(VOIP_INPUT_WIDGET(myData.voip_widget));
      GtkWidget* slider = ido_scale_menu_item_get_scale((IdoScaleMenuItem*)slider_widget);
      GtkRange* range = (GtkRange*)slider;
      g_return_val_if_fail(GTK_IS_RANGE(range), FALSE);
      current_value = gtk_range_get_value(range);
      new_value = current_value;
      is_voip_slider = TRUE;
    }

    switch (event->keyval) {
    case GDK_Right:
      digested = TRUE;
      new_value = current_value + five_percent;
      break;
    case GDK_Left:
      digested = TRUE;
      new_value = current_value - five_percent;
      break;
    case GDK_plus:
      digested = TRUE;
      new_value = current_value + five_percent;
      break;
    case GDK_minus:
      digested = TRUE;
      new_value = current_value - five_percent;
      break;
    default:
      break;
    }
    new_value = CLAMP(new_value, 0, 100);
    if (new_value != current_value){
      if (is_voip_slider == TRUE){
        voip_input_widget_update (VOIP_INPUT_WIDGET(myData.voip_widget), new_value);
      }
      else{
        volume_widget_update (VOLUME_WIDGET(myData.volume_widget), new_value, "keypress-update");
      }
    }
  }
  else if (IS_TRANSPORT_WIDGET(menuitem) == TRUE) {
    TransportWidget* transport_widget = NULL;
    GList* elem;

    for ( elem = myData.transport_widgets_list; elem; elem = elem->next ) {
      transport_widget = TRANSPORT_WIDGET ( elem->data );
      if ( transport_widget_is_selected( transport_widget ) ) 
        break;
    }

    switch (event->keyval) {
    case GDK_Right:
      transport_widget_react_to_key_press_event ( transport_widget,
                                                  TRANSPORT_ACTION_NEXT );
      digested = TRUE;         
      break;        
    case GDK_Left:
      transport_widget_react_to_key_press_event ( transport_widget,
                                                  TRANSPORT_ACTION_PREVIOUS );
      digested = TRUE;         
      break;                  
    case GDK_KEY_space:
      transport_widget_react_to_key_press_event ( transport_widget,
                                                  TRANSPORT_ACTION_PLAY_PAUSE );
      digested = TRUE;         
      break;
    case GDK_Up:
    case GDK_Down:
      digested = FALSE;     
      break;
    default:
      break;
    }
  } 
  return digested;
}


/**
key_release_cb:
**/
static gboolean
key_release_cb(GtkWidget* widget, GdkEventKey* event, CairoDockModuleInstance *myApplet)
{
  gboolean digested = FALSE;

  GtkWidget *menuitem;

	#if (GTK_MAJOR_VERSION < 3)
	menuitem = GTK_MENU_SHELL (widget)->active_menu_item;
	#else
	menuitem = gtk_menu_shell_get_selected_item (GTK_MENU_SHELL (widget));
	#endif

  if (IS_TRANSPORT_WIDGET(menuitem) == TRUE) {
    TransportWidget* transport_widget = NULL;
    GList* elem;

    for(elem = myData.transport_widgets_list; elem; elem = elem->next) {
      transport_widget = TRANSPORT_WIDGET (elem->data);
      if ( transport_widget_is_selected( transport_widget ) ) 
        break;
    }

    switch (event->keyval) {
    case GDK_Right:
      transport_widget_react_to_key_release_event ( transport_widget,
                                                    TRANSPORT_ACTION_NEXT );
      digested = TRUE;
      break;        
    case GDK_Left:
      transport_widget_react_to_key_release_event ( transport_widget,
                                                    TRANSPORT_ACTION_PREVIOUS );
      digested = TRUE;         
      break;                  
    case GDK_KEY_space:
      transport_widget_react_to_key_release_event ( transport_widget,
                                                    TRANSPORT_ACTION_PLAY_PAUSE );
      digested = TRUE;         
      break;
    case GDK_Up:
    case GDK_Down:
      digested = FALSE;     
      break;
    default:
      break;
    }
  } 
  return digested;
}




  ///////////////
 // MAKE MENU //
///////////////

void cd_sound_add_menu_handler (DbusmenuGtkClient * client)
{
	dbusmenu_client_add_type_handler (DBUSMENU_CLIENT(client),
		DBUSMENU_VOLUME_MENUITEM_TYPE,
		(DbusmenuClientTypeHandler) new_volume_slider_widget);
	dbusmenu_client_add_type_handler (DBUSMENU_CLIENT(client),
		DBUSMENU_VOIP_INPUT_MENUITEM_TYPE,
		(DbusmenuClientTypeHandler) new_voip_slider_widget);
	dbusmenu_client_add_type_handler (DBUSMENU_CLIENT(client),
		DBUSMENU_TRANSPORT_MENUITEM_TYPE,
		(DbusmenuClientTypeHandler) new_transport_widget);
	dbusmenu_client_add_type_handler (DBUSMENU_CLIENT(client),
		DBUSMENU_METADATA_MENUITEM_TYPE,
		(DbusmenuClientTypeHandler) new_metadata_widget);
	dbusmenu_client_add_type_handler (DBUSMENU_CLIENT(client),
		DBUSMENU_TITLE_MENUITEM_TYPE,
		(DbusmenuClientTypeHandler) new_title_widget);
	
	// Note: Not ideal but all key handling needs to be managed here and then 
	// delegated to the appropriate widget.
	g_signal_connect (myData.pIndicator->pMenu, "key-press-event", G_CALLBACK(key_press_cb), myApplet);
	g_signal_connect (myData.pIndicator->pMenu, "key-release-event", G_CALLBACK(key_release_cb), myApplet);
}

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

/// This file is essentially indicator-sound.c, adapted to Cairo-Dock.

#include <stdlib.h>
#include <string.h>

#include <gdk/gdkkeysyms.h>

#include "applet-struct.h"
#include "applet-backend-sound-menu.h"  // update_accessible_desc
#include "transport-widget.h"
#include "volume-widget.h"
#include "voip-input-widget.h"
#include "mute-widget.h"
#include "metadata-widget.h"
#include "applet-menu.h"

#if (GTK_MAJOR_VERSION < 3)
#include <libdbusmenu-gtk/menuitem.h>
#include <libdbusmenu-gtk/menu.h>
#else
#include <libdbusmenu-gtk3/menuitem.h>
#include <libdbusmenu-gtk3/menu.h>
#endif
#include <libido/idoscalemenuitem.h>

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
  ///IndicatorObject *io = NULL;

  g_return_val_if_fail(DBUSMENU_IS_MENUITEM(newitem), FALSE);
  g_return_val_if_fail(DBUSMENU_IS_GTKCLIENT(client), FALSE);

  bar = transport_widget_new(newitem);
  /**io = g_object_get_data (G_OBJECT (client), "indicator");
  IndicatorSoundPrivate* priv = INDICATOR_SOUND_GET_PRIVATE(INDICATOR_SOUND (io));
  priv->transport_widgets_list = g_list_append ( priv->transport_widgets_list, bar );*/
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
  
  g_debug ("%s (\"%s\")", __func__,
           dbusmenu_menuitem_property_get(newitem, DBUSMENU_METADATA_MENUITEM_PLAYER_NAME));
  
  GtkMenuItem *menu_metadata_widget = GTK_MENU_ITEM(metadata);

  gtk_widget_show_all(metadata);
  dbusmenu_gtkclient_newitem_base (DBUSMENU_GTKCLIENT(client),
                                   newitem,
                                   menu_metadata_widget,
                                   parent);
  return TRUE;
}

static gboolean
new_volume_slider_widget(DbusmenuMenuitem * newitem,
                         DbusmenuMenuitem * parent,
                         DbusmenuClient * client,
                         gpointer user_data)
{
  g_debug("indicator-sound: new_volume_slider_widget");

  GtkWidget* volume_widget = NULL;
  ///IndicatorObject *io = NULL;

  g_return_val_if_fail(DBUSMENU_IS_MENUITEM(newitem), FALSE);
  g_return_val_if_fail(DBUSMENU_IS_GTKCLIENT(client), FALSE);

  /**io = g_object_get_data (G_OBJECT (client), "indicator");
  IndicatorSoundPrivate* priv = INDICATOR_SOUND_GET_PRIVATE(INDICATOR_SOUND (io));*/
	AppletData *priv = myDataPtr;

  if (priv->volume_widget != NULL){ 
    ///volume_widget_tidy_up (priv->volume_widget);
    gtk_widget_destroy (priv->volume_widget);
    priv->volume_widget = NULL;
  }
  volume_widget = volume_widget_new (newitem/**, io*/);
  priv->volume_widget = volume_widget;
  // Don't forget to set the accessible desc.
  /// TODO: check that it's not needed...
  update_accessible_desc (-1/**io*/);
  

  GtkWidget* ido_slider_widget = volume_widget_get_ido_slider(VOLUME_WIDGET(priv->volume_widget));

  gtk_widget_show_all(ido_slider_widget);
  // register the style callback on this widget with state manager's style change
  // handler (needs to remake the blocking animation for each style).
  /**g_signal_connect (ido_slider_widget, "style-set",
                    G_CALLBACK(sound_state_manager_style_changed_cb),
                    priv->state_manager);*/

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
 * @param user_data
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
  ///IndicatorObject *io = NULL;

  g_return_val_if_fail(DBUSMENU_IS_MENUITEM(newitem), FALSE);
  g_return_val_if_fail(DBUSMENU_IS_GTKCLIENT(client), FALSE);

  /**io = g_object_get_data (G_OBJECT (client), "indicator");
  IndicatorSoundPrivate* priv = INDICATOR_SOUND_GET_PRIVATE(INDICATOR_SOUND (io));*/
	AppletData *priv = myDataPtr;

  if (priv->voip_widget != NULL){ 
    ///voip_input_widget_tidy_up (priv->voip_widget);
    gtk_widget_destroy (priv->voip_widget);
    priv->voip_widget = NULL;
  }

  voip_widget = voip_input_widget_new (newitem);
  priv->voip_widget = voip_widget;

  GtkWidget* ido_slider_widget = voip_input_widget_get_ido_slider(VOIP_INPUT_WIDGET(voip_widget));

  gtk_widget_show_all(ido_slider_widget);

  GtkMenuItem *menu_volume_item = GTK_MENU_ITEM(ido_slider_widget);
  dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client),
                                  newitem,
                                  menu_volume_item,
                                  parent);
  return TRUE;
}

static gboolean
new_mute_widget(DbusmenuMenuitem * newitem,
                DbusmenuMenuitem * parent,
                DbusmenuClient * client,
                gpointer user_data)
{
  ///IndicatorObject *io = NULL;

  g_return_val_if_fail(DBUSMENU_IS_MENUITEM(newitem), FALSE);
  g_return_val_if_fail(DBUSMENU_IS_GTKCLIENT(client), FALSE);

  /**io = g_object_get_data (G_OBJECT (client), "indicator");
  IndicatorSoundPrivate* priv = INDICATOR_SOUND_GET_PRIVATE(INDICATOR_SOUND (io));*/
	AppletData *priv = myDataPtr;
  
  if (priv->mute_widget != NULL){ 
    g_object_unref (priv->mute_widget);
    priv->mute_widget = NULL;
  }

  priv->mute_widget = mute_widget_new(newitem);
  GtkMenuItem *item = mute_widget_get_menu_item (priv->mute_widget);

  dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client),
                                  newitem,
                                  item,
                                  parent);

  return TRUE;
}


/*******************************************************************/
//UI callbacks
/******************************************************************/

/**static GtkWidget *
get_current_item (GtkContainer * container)
{
  GList *children = gtk_container_get_children (container);
  GList *iter;
  GtkWidget *rv = NULL;

  // Suprisingly, GTK+ doesn't really let us query "what is the currently
  // selected item?".  But it does note it internally by prelighting the
  // widget, so we watch for that.
  for (iter = children; iter; iter = iter->next) {
    if (gtk_widget_get_state (GTK_WIDGET (iter->data)) & GTK_STATE_PRELIGHT) {
      rv = GTK_WIDGET (iter->data);
      break;
    }
  }

  return rv;
}*/

/**
key_press_cb:
**/
static gboolean
key_press_cb(GtkWidget* widget, GdkEventKey* event, gpointer data)
{
  gboolean digested = FALSE;

  ///g_return_val_if_fail(IS_INDICATOR_SOUND(data), FALSE);

  ///IndicatorSound *indicator = INDICATOR_SOUND (data);

  ///IndicatorSoundPrivate* priv = INDICATOR_SOUND_GET_PRIVATE(indicator);
  AppletData *priv = myDataPtr;
  GtkWidget *menuitem;
  ///menuitem = get_current_item (GTK_CONTAINER (widget));
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
      GtkWidget* slider_widget = volume_widget_get_ido_slider(VOLUME_WIDGET(priv->volume_widget));
      GtkWidget* slider = ido_scale_menu_item_get_scale((IdoScaleMenuItem*)slider_widget);
      GtkRange* range = (GtkRange*)slider;
      g_return_val_if_fail(GTK_IS_RANGE(range), FALSE);
      current_value = gtk_range_get_value(range);
      new_value = current_value;
    }
    else if (g_ascii_strcasecmp (ido_scale_menu_item_get_primary_label (IDO_SCALE_MENU_ITEM(menuitem)), "VOIP") == 0) {
      g_debug ("VOIP SLIDER KEY PRESS");
      GtkWidget* slider_widget = voip_input_widget_get_ido_slider(VOIP_INPUT_WIDGET(priv->voip_widget));
      GtkWidget* slider = ido_scale_menu_item_get_scale((IdoScaleMenuItem*)slider_widget);
      GtkRange* range = (GtkRange*)slider;
      g_return_val_if_fail(GTK_IS_RANGE(range), FALSE);
      current_value = gtk_range_get_value(range);
      new_value = current_value;
      is_voip_slider = TRUE;
    }

    switch (event->keyval) {
    case GDK_KEY_Right:
      digested = TRUE;
      new_value = current_value + five_percent;
      break;
    case GDK_KEY_Left:
      digested = TRUE;
      new_value = current_value - five_percent;
      break;
    case GDK_KEY_plus:
      digested = TRUE;
      new_value = current_value + five_percent;
      break;
    case GDK_KEY_minus:
      digested = TRUE;
      new_value = current_value - five_percent;
      break;
    default:
      break;
    }
    new_value = CLAMP(new_value, 0, 100);
    if (new_value != current_value){
      if (is_voip_slider == TRUE){
        voip_input_widget_update (VOIP_INPUT_WIDGET(priv->voip_widget), new_value);
      }
      else{
        volume_widget_update (VOLUME_WIDGET(priv->volume_widget), new_value, "keypress-update");
      }
    }
  }
  else if (IS_TRANSPORT_WIDGET(menuitem) == TRUE) {
    TransportWidget* transport_widget = NULL;
    GList* elem;

    for ( elem = priv->transport_widgets_list; elem; elem = elem->next ) {
      transport_widget = TRANSPORT_WIDGET ( elem->data );
      if ( transport_widget_is_selected( transport_widget ) ) 
        break;
    }

    switch (event->keyval) {
    case GDK_KEY_Right:
      transport_widget_react_to_key_press_event ( transport_widget,
                                                  TRANSPORT_ACTION_NEXT );
      digested = TRUE;         
      break;        
    case GDK_KEY_Left:
      transport_widget_react_to_key_press_event ( transport_widget,
                                                  TRANSPORT_ACTION_PREVIOUS );
      digested = TRUE;         
      break;                  
    case GDK_KEY_space:
      transport_widget_react_to_key_press_event ( transport_widget,
                                                  TRANSPORT_ACTION_PLAY_PAUSE );
      digested = TRUE;         
      break;
    case GDK_KEY_Up:
    case GDK_KEY_Down:
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
key_release_cb(GtkWidget* widget, GdkEventKey* event, gpointer data)
{
  gboolean digested = FALSE;

  ///g_return_val_if_fail(IS_INDICATOR_SOUND(data), FALSE);

  ///IndicatorSound *indicator = INDICATOR_SOUND (data);

  ///IndicatorSoundPrivate* priv = INDICATOR_SOUND_GET_PRIVATE(indicator);
	AppletData *priv = myDataPtr;
  GtkWidget *menuitem;

  ///menuitem = get_current_item (GTK_CONTAINER (widget));
	#if (GTK_MAJOR_VERSION < 3)
	menuitem = GTK_MENU_SHELL (widget)->active_menu_item;
	#else
	menuitem = gtk_menu_shell_get_selected_item (GTK_MENU_SHELL (widget));
	#endif
  if (IS_TRANSPORT_WIDGET(menuitem) == TRUE) {
    TransportWidget* transport_widget = NULL;
    GList* elem;

    for(elem = priv->transport_widgets_list; elem; elem = elem->next) {
      transport_widget = TRANSPORT_WIDGET (elem->data);
      if ( transport_widget_is_selected( transport_widget ) ) 
        break;
    }

    switch (event->keyval) {
    case GDK_KEY_Right:
      transport_widget_react_to_key_release_event ( transport_widget,
                                                    TRANSPORT_ACTION_NEXT );
      digested = TRUE;
      break;        
    case GDK_KEY_Left:
      transport_widget_react_to_key_release_event ( transport_widget,
                                                    TRANSPORT_ACTION_PREVIOUS );
      digested = TRUE;         
      break;                  
    case GDK_KEY_space:
      transport_widget_react_to_key_release_event ( transport_widget,
                                                    TRANSPORT_ACTION_PLAY_PAUSE );
      digested = TRUE;         
      break;
    case GDK_KEY_Up:
    case GDK_KEY_Down:
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
                              	new_volume_slider_widget);
	dbusmenu_client_add_type_handler (DBUSMENU_CLIENT(client),
                              	DBUSMENU_VOIP_INPUT_MENUITEM_TYPE,
                              	new_voip_slider_widget);
	dbusmenu_client_add_type_handler (DBUSMENU_CLIENT(client),
                              	DBUSMENU_TRANSPORT_MENUITEM_TYPE,
                              	new_transport_widget);
	dbusmenu_client_add_type_handler (DBUSMENU_CLIENT(client),
                              	DBUSMENU_METADATA_MENUITEM_TYPE,
                              	new_metadata_widget);
	dbusmenu_client_add_type_handler (DBUSMENU_CLIENT(client),
                              	DBUSMENU_MUTE_MENUITEM_TYPE,
                              	new_mute_widget);
	
	// Note: Not ideal but all key handling needs to be managed here and then 
	// delegated to the appropriate widget.
	g_signal_connect (myData.pIndicator->pMenu, "key-press-event", G_CALLBACK(key_press_cb), myApplet);
	g_signal_connect (myData.pIndicator->pMenu, "key-release-event", G_CALLBACK(key_release_cb), myApplet);
}

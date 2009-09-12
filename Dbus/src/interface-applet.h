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


#ifndef __INTERFACE_APPLET__
#define  __INTERFACE_APPLET__

#include <cairo-dock.h>
#include "applet-struct.h"


dbusApplet *cd_dbus_create_remote_applet_object (CairoDockModuleInstance *pModuleInstance);

void cd_dbus_delete_remote_applet_object (CairoDockModuleInstance *pModuleInstance);

void cd_dbus_unregister_notifications (void);


gboolean cd_dbus_applet_is_used (const gchar *cModuleName);
void cd_dbus_launch_distant_applet_in_dir (const gchar *cModuleName, const gchar *cDirPath);
void cd_dbus_launch_distant_applet (const gchar *cModuleName);

void cd_dbus_action_on_init_module (CairoDockModuleInstance *pModuleInstance);
void cd_dbus_emit_on_init_module (CairoDockModuleInstance *pModuleInstance, GKeyFile *pKeyFile);

void cd_dbus_action_on_stop_module (CairoDockModuleInstance *pModuleInstance);
void cd_dbus_emit_on_stop_module (CairoDockModuleInstance *pModuleInstance);

gboolean cd_dbus_emit_on_reload_module (CairoDockModuleInstance *pModuleInstance, CairoContainer *pOldContainer, GKeyFile *pKeyFile);


gboolean cd_dbus_applet_set_quick_info (dbusApplet *pDbusApplet, const gchar *cQuickInfo, GError **error);

gboolean cd_dbus_applet_set_label (dbusApplet *pDbusApplet, const gchar *cLabel, GError **error);

gboolean cd_dbus_applet_set_icon (dbusApplet *pDbusApplet, const gchar *cImage, GError **error);

gboolean cd_dbus_applet_animate (dbusApplet *pDbusApplet, const gchar *cAnimation, gint iNbRounds, GError **error);

gboolean cd_dbus_applet_show_dialog (dbusApplet *pDbusApplet, const gchar *message, gint iDuration, GError **error);

gboolean cd_dbus_applet_populate_menu (dbusApplet *pDbusApplet, const gchar **pLabels, GError **error);

gboolean cd_dbus_applet_add_data_renderer (dbusApplet *pDbusApplet, const gchar *cType, gint iNbValues, const gchar *cTheme, GError **error);

gboolean cd_dbus_applet_render_values (dbusApplet *pDbusApplet, GArray *pValues, GError **error);

gboolean cd_dbus_applet_add_sub_icons (dbusApplet *pDbusApplet, const gchar **pIconFields, GError **error);

gboolean cd_dbus_applet_remove_sub_icon (dbusApplet *pDbusApplet, const gchar *cIconID, GError **error);


gboolean cd_dbus_applet_emit_on_click_icon (gpointer data, Icon *pClickedIcon, CairoContainer *pClickedContainer, guint iButtonState);

gboolean cd_dbus_applet_emit_on_middle_click_icon (gpointer data, Icon *pClickedIcon, CairoContainer *pClickedContainer);

gboolean cd_dbus_applet_emit_on_scroll_icon (gpointer data, Icon *pClickedIcon, CairoContainer *pClickedContainer, int iDirection);

gboolean cd_dbus_applet_emit_on_build_menu (gpointer data, Icon *pClickedIcon, CairoContainer *pClickedContainer, GtkWidget *pAppletMenu);

gboolean cd_dbus_applet_emit_on_drop_data (gpointer data, const gchar *cReceivedData, Icon *pClickedIcon, double fPosition, CairoContainer *pClickedContainer);


#endif

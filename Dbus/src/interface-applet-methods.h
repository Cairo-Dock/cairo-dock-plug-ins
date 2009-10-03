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


#ifndef __INTERFACE_APPLET_METHODS__
#define  __INTERFACE_APPLET_METHODS__

#include <cairo-dock.h>
#include "applet-struct.h"


/* sub applet interface methods */
gboolean cd_dbus_sub_applet_set_quick_info (dbusSubApplet *pDbusSubApplet, const gchar *cQuickInfo, const gchar *cIconID, GError **error);

gboolean cd_dbus_sub_applet_set_label (dbusSubApplet *pDbusSubApplet, const gchar *cLabel, const gchar *cIconID, GError **error);

gboolean cd_dbus_sub_applet_set_icon (dbusSubApplet *pDbusSubApplet, const gchar *cImage, const gchar *cIconID, GError **error);

gboolean cd_dbus_sub_applet_animate (dbusSubApplet *pDbusSubApplet, const gchar *cAnimation, gint iNbRounds, const gchar *cIconID, GError **error);

gboolean cd_dbus_sub_applet_show_dialog (dbusSubApplet *pDbusSubApplet, const gchar *message, gint iDuration, const gchar *cIconID, GError **error);

gboolean cd_dbus_sub_applet_add_sub_icons (dbusSubApplet *pDbusSubApplet, const gchar **pIconFields, GError **error);

gboolean cd_dbus_sub_applet_remove_sub_icon (dbusSubApplet *pDbusSubApplet, const gchar *cIconID, GError **error);


/* applet interface methods */
gboolean cd_dbus_applet_set_quick_info (dbusApplet *pDbusApplet, const gchar *cQuickInfo, GError **error);

gboolean cd_dbus_applet_set_label (dbusApplet *pDbusApplet, const gchar *cLabel, GError **error);

gboolean cd_dbus_applet_set_icon (dbusApplet *pDbusApplet, const gchar *cImage, GError **error);

gboolean cd_dbus_applet_animate (dbusApplet *pDbusApplet, const gchar *cAnimation, gint iNbRounds, GError **error);

gboolean cd_dbus_applet_show_dialog (dbusApplet *pDbusApplet, const gchar *message, gint iDuration, GError **error);

gboolean cd_dbus_applet_add_data_renderer (dbusApplet *pDbusApplet, const gchar *cType, gint iNbValues, const gchar *cTheme, GError **error);

gboolean cd_dbus_applet_render_values (dbusApplet *pDbusApplet, GArray *pValues, GError **error);

gboolean cd_dbus_applet_control_appli (dbusApplet *pDbusApplet, const gchar *cApplicationClass, GError **error);

gboolean cd_dbus_applet_populate_menu (dbusApplet *pDbusApplet, const gchar **pLabels, GError **error);


#endif

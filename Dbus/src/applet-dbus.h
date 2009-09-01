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


#ifndef __APPLET_DBUS__
#define  __APPLET_DBUS__

#include <cairo-dock.h>
#include "applet-struct.h"

void cd_dbus_launch_service (void);
void cd_dbus_stop_service (void);


gboolean cd_dbus_callback_show_desklet(dbusCallback *pDbusCallback, gboolean *widgetLayer, GError **error);

gboolean cd_dbus_callback_reboot(dbusCallback *pDbusCallback, GError **error);

gboolean cd_dbus_callback_quit (dbusCallback *pDbusCallback, GError **error);

gboolean cd_dbus_callback_show_dock (dbusCallback *pDbusCallback, gboolean bShow, GError **error);

gboolean cd_dbus_callback_reload_module (dbusCallback *pDbusCallback, const gchar *cModuleName, GError **error);

gboolean cd_dbus_callback_load_launcher_from_file (dbusCallback *pDbusCallback, const gchar *cDesktopFile, GError **error);

gboolean cd_dbus_callback_create_launcher_from_scratch (dbusCallback *pDbusCallback, const gchar *cIconFile, const gchar *cLabel, const gchar *cCommand, const gchar *cParentDockName, GError **error);

gboolean cd_dbus_callback_set_quick_info (dbusCallback *pDbusCallback, const gchar *cQuickInfo, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error);

gboolean cd_dbus_callback_set_label (dbusCallback *pDbusCallback, const gchar *cLabel, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error);

gboolean cd_dbus_callback_set_icon (dbusCallback *pDbusCallback, const gchar *cImage, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error);

gboolean cd_dbus_callback_animate (dbusCallback *pDbusCallback, const gchar *cAnimation, gint iNbRounds, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error);

gboolean cd_dbus_callback_show_dialog (dbusCallback *pDbusCallback, const gchar *message, gint iDuration, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error);


gboolean cd_dbus_callback_register_new_module (dbusCallback *pDbusCallback, const gchar *cModuleName, gint iCategory, const gchar *cDescription, const gchar *cShareDataDir, GError **error);

gboolean cd_dbus_callback_unregister_module (dbusCallback *pDbusCallback, const gchar *cModuleName, GError **error);


#endif

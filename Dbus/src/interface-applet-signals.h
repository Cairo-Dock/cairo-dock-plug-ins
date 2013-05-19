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

#ifndef __INTERFACE_APPLET_SIGNALS__
#define  __INTERFACE_APPLET_SIGNALS__

#include <cairo-dock.h>
#include "applet-struct.h"


void cd_dbus_applet_init_signals_once (dbusAppletClass *klass);
void cd_dbus_sub_applet_init_signals_once (dbusSubAppletClass *klass);


gboolean cd_dbus_applet_emit_on_click_icon (gpointer data, Icon *pClickedIcon, GldiContainer *pClickedContainer, guint iButtonState);

gboolean cd_dbus_applet_emit_on_middle_click_icon (gpointer data, Icon *pClickedIcon, GldiContainer *pClickedContainer);

gboolean cd_dbus_applet_emit_on_scroll_icon (gpointer data, Icon *pClickedIcon, GldiContainer *pClickedContainer, int iDirection);

gboolean cd_dbus_applet_emit_on_build_menu (gpointer data, Icon *pClickedIcon, GldiContainer *pClickedContainer, GtkWidget *pAppletMenu);

void cd_dbus_emit_on_menu_select (GtkMenuItem *pMenuItem, gpointer data);

gboolean cd_dbus_applet_emit_on_drop_data (gpointer data, const gchar *cReceivedData, Icon *pClickedIcon, double fPosition, GldiContainer *pClickedContainer);

gboolean cd_dbus_applet_emit_on_change_focus (gpointer data, GldiWindowActor *pNewActiveWindow);


void cd_dbus_applet_emit_on_answer_question (int iClickedButton, GtkWidget *pInteractiveWidget, dbusApplet *pDbusApplet, CairoDialog *pDialog);

void cd_dbus_applet_emit_on_answer_value (int iClickedButton, GtkWidget *pInteractiveWidget, dbusApplet *pDbusApplet, CairoDialog *pDialog);

void cd_dbus_applet_emit_on_answer_text (int iClickedButton, GtkWidget *pInteractiveWidget, dbusApplet *pDbusApplet, CairoDialog *pDialog);

void cd_dbus_applet_emit_on_answer_buttons (int iClickedButton, GtkWidget *pInteractiveWidget, dbusApplet *pDbusApplet, CairoDialog *pDialog);

void cd_dbus_applet_emit_on_answer_text_entry (int iClickedButton, GtkWidget *pInteractiveWidget, dbusApplet *pDbusApplet, CairoDialog *pDialog);

void cd_dbus_applet_emit_on_answer_text_view (int iClickedButton, GtkWidget *pInteractiveWidget, dbusApplet *pDbusApplet, CairoDialog *pDialog);

void cd_dbus_applet_emit_on_answer_scale (int iClickedButton, GtkWidget *pInteractiveWidget, dbusApplet *pDbusApplet, CairoDialog *pDialog);

void cd_dbus_applet_emit_on_answer_combo_entry (int iClickedButton, GtkWidget *pInteractiveWidget, dbusApplet *pDbusApplet, CairoDialog *pDialog);

void cd_dbus_applet_emit_on_answer_combo (int iClickedButton, GtkWidget *pInteractiveWidget, dbusApplet *pDbusApplet, CairoDialog *pDialog);

void cd_dbus_applet_emit_on_shortkey (const gchar *cShortkey, dbusApplet *pDbusApplet);


void cd_dbus_action_on_init_module (GldiModuleInstance *pModuleInstance);
gboolean cd_dbus_emit_init_module_delayed (dbusApplet *pDbusApplet);
void cd_dbus_emit_init_signal (GldiModuleInstance *pModuleInstance);
void cd_dbus_emit_on_init_module (GldiModuleInstance *pModuleInstance, GKeyFile *pKeyFile);

void cd_dbus_action_on_stop_module (GldiModuleInstance *pModuleInstance);
void cd_dbus_emit_on_stop_module (GldiModuleInstance *pModuleInstance);

gboolean cd_dbus_emit_on_reload_module (GldiModuleInstance *pModuleInstance, GldiContainer *pOldContainer, GKeyFile *pKeyFile);


#endif

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

void cd_dbus_add_applet_to_startup (const gchar *cModuleName);

gboolean cd_dbus_register_module_in_dir (const gchar *cModuleName, const gchar *cThirdPartyPath);

gboolean cd_dbus_register_new_module (const gchar *cModuleName, const gchar *cDescription, const gchar *cAuthor, const gchar *cVersion, gint iCategory, const gchar *cIconName, const gchar *cShareDataDir);


// Marshallers //

void cd_dbus_marshal_VOID__INT_STRING (GClosure *closure,
	GValue *return_value,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint,
	gpointer marshal_data);

void cd_dbus_marshal_VOID__BOOLEAN_STRING (GClosure *closure,
	GValue *return_value,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint,
	gpointer marshal_data);

void cd_dbus_marshal_VOID__STRING_STRING (GClosure *closure,
	GValue *return_value,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint,
	gpointer marshal_data);

void cd_dbus_marshal_VOID__VALUE (GClosure *closure,
	GValue *return_value,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint,
	gpointer marshal_data);

void cd_dbus_marshal_VOID__INT_VALUE (GClosure *closure,
	GValue *return_value,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint,
	gpointer marshal_data);

void cd_dbus_marshal_VOID__VALUE_STRING (GClosure *closure,
	GValue *return_value,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint,
	gpointer marshal_data);

#define cd_dbus_marshal_VOID__INT g_cclosure_marshal_VOID__INT
#define cd_dbus_marshal_VOID__BOOLEAN g_cclosure_marshal_VOID__BOOLEAN
#define cd_dbus_marshal_VOID__STRING g_cclosure_marshal_VOID__STRING
#define cd_dbus_marshal_VOID__VOID g_cclosure_marshal_VOID__VOID

#endif

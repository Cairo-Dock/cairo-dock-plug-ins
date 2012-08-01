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

#include <unistd.h>
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include "interface-main-methods.h"

#include "dbus-main-spec.h"
#include "applet-marshallers.h"

void cd_dbus_marshal_VOID__INT_STRING (GClosure *closure,
	GValue *return_value G_GNUC_UNUSED,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint G_GNUC_UNUSED,
	gpointer marshal_data)
{
	//g_print ("%s ()\n", __func__);
	typedef void (*GMarshalFunc_VOID__INT_STRING) (gpointer     data1,
												gint        arg_1,
												gchar 	   *arg_2,
												gpointer     data2);
	register GMarshalFunc_VOID__INT_STRING callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;
	g_return_if_fail (n_param_values == 3);  // return_value est NULL ici, car la callback ne renvoit rien.

	if (G_CCLOSURE_SWAP_DATA (closure))
	{
		data1 = closure->data;
		data2 = g_value_peek_pointer (param_values + 0);
	}
	else
	{
		data1 = g_value_peek_pointer (param_values + 0);
		data2 = closure->data;
	}
	callback = (GMarshalFunc_VOID__INT_STRING) (marshal_data ? marshal_data : cc->callback);

	callback (data1,
		g_marshal_value_peek_int (param_values + 1),
		g_marshal_value_peek_string (param_values + 2),
		data2);
}
void cd_dbus_marshal_VOID__BOOLEAN_STRING (GClosure *closure,
	GValue *return_value G_GNUC_UNUSED,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint G_GNUC_UNUSED,
	gpointer marshal_data)
{
	//g_print ("%s ()\n", __func__);
	typedef void (*GMarshalFunc_VOID__BOOLEAN_STRING) (gpointer     data1,
												gboolean    arg_1,
												gchar 	   *arg_2,
												gpointer     data2);
	register GMarshalFunc_VOID__BOOLEAN_STRING callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;
	g_return_if_fail (n_param_values == 3);  // return_value est NULL ici, car la callback ne renvoit rien.

	if (G_CCLOSURE_SWAP_DATA (closure))
	{
		data1 = closure->data;
		data2 = g_value_peek_pointer (param_values + 0);
	}
	else
	{
		data1 = g_value_peek_pointer (param_values + 0);
		data2 = closure->data;
	}
	callback = (GMarshalFunc_VOID__BOOLEAN_STRING) (marshal_data ? marshal_data : cc->callback);

	callback (data1,
		g_marshal_value_peek_boolean (param_values + 1),
		g_marshal_value_peek_string (param_values + 2),
		data2);
}
void cd_dbus_marshal_VOID__STRING_STRING (GClosure *closure,
	GValue *return_value G_GNUC_UNUSED,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint G_GNUC_UNUSED,
	gpointer marshal_data)
{
	//g_print ("%s ()\n", __func__);
	typedef void (*GMarshalFunc_VOID__STRING_STRING) (gpointer     data1,
												gchar      *arg_1,
												gchar 	   *arg_2,
												gpointer     data2);
	register GMarshalFunc_VOID__STRING_STRING callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;
	g_return_if_fail (n_param_values == 3);  // return_value est NULL ici, car la callback ne renvoit rien.

	if (G_CCLOSURE_SWAP_DATA (closure))
	{
		data1 = closure->data;
		data2 = g_value_peek_pointer (param_values + 0);
	}
	else
	{
		data1 = g_value_peek_pointer (param_values + 0);
		data2 = closure->data;
	}
	callback = (GMarshalFunc_VOID__STRING_STRING) (marshal_data ? marshal_data : cc->callback);

	callback (data1,
		g_marshal_value_peek_string (param_values + 1),
		g_marshal_value_peek_string (param_values + 2),
		data2);
}
void cd_dbus_marshal_VOID__VALUE (GClosure *closure,
	GValue *return_value G_GNUC_UNUSED,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint G_GNUC_UNUSED,
	gpointer marshal_data)
{
	//g_print ("%s ()\n", __func__);
	typedef void (*GMarshalFunc_VOID__VALUE) (gpointer     data1,
												GValue     *arg_1,
												gpointer     data2);
	register GMarshalFunc_VOID__VALUE callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;
	g_return_if_fail (n_param_values == 2);  // return_value est NULL ici, car la callback ne renvoit rien.

	if (G_CCLOSURE_SWAP_DATA (closure))
	{
		data1 = closure->data;
		data2 = g_value_peek_pointer (param_values + 0);
	}
	else
	{
		data1 = g_value_peek_pointer (param_values + 0);
		data2 = closure->data;
	}
	callback = (GMarshalFunc_VOID__VALUE) (marshal_data ? marshal_data : cc->callback);

	callback (data1,
		g_marshal_value_peek_pointer (param_values + 1),
		data2);
}
void cd_dbus_marshal_VOID__INT_VALUE (GClosure *closure,
	GValue *return_value G_GNUC_UNUSED,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint G_GNUC_UNUSED,
	gpointer marshal_data)
{
	//g_print ("%s ()\n", __func__);
	typedef void (*GMarshalFunc_VOID__INT_VALUE) (gpointer     data1,
												gint 		arg_1,
												GValue     *arg_2,
												gpointer     data2);
	register GMarshalFunc_VOID__INT_VALUE callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;
	g_return_if_fail (n_param_values == 3);  // return_value est NULL ici, car la callback ne renvoit rien.

	if (G_CCLOSURE_SWAP_DATA (closure))
	{
		data1 = closure->data;
		data2 = g_value_peek_pointer (param_values + 0);
	}
	else
	{
		data1 = g_value_peek_pointer (param_values + 0);
		data2 = closure->data;
	}
	callback = (GMarshalFunc_VOID__INT_VALUE) (marshal_data ? marshal_data : cc->callback);

	callback (data1,
		g_marshal_value_peek_int (param_values + 1),
		g_marshal_value_peek_pointer (param_values + 2),
		data2);
}
void cd_dbus_marshal_VOID__VALUE_STRING (GClosure *closure,
	GValue *return_value G_GNUC_UNUSED,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint G_GNUC_UNUSED,
	gpointer marshal_data)
{
	//g_print ("%s ()\n", __func__);
	typedef void (*GMarshalFunc_VOID__VALUE_STRING) (gpointer     data1,
												GValue     *arg_1,
												gchar 	   *arg_2,
												gpointer     data2);
	register GMarshalFunc_VOID__VALUE_STRING callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;
	g_return_if_fail (n_param_values == 3);  // return_value est NULL ici, car la callback ne renvoit rien.

	if (G_CCLOSURE_SWAP_DATA (closure))
	{
		data1 = closure->data;
		data2 = g_value_peek_pointer (param_values + 0);
	}
	else
	{
		data1 = g_value_peek_pointer (param_values + 0);
		data2 = closure->data;
	}
	callback = (GMarshalFunc_VOID__VALUE_STRING) (marshal_data ? marshal_data : cc->callback);

	callback (data1,
		g_marshal_value_peek_pointer (param_values + 1),
		g_marshal_value_peek_string (param_values + 2),
		data2);
}

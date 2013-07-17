/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
* Based on the work of Ted Gould <ted@canonical.com>
*  (applet-main.c from indicator-applet project)
* Adapted by Matthieu Baerts <matttbe@gmail.com>
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

#include <glib.h>

#include <cairo-dock.h>

#include "indicator-applet3.h"

#ifdef IS_INDICATOR_NG
#include <libindicator/indicator-ng.h>
#include <libido/libido.h>
static gboolean bIsIdoInit = FALSE;
#endif

IndicatorObject * cd_indicator3_load (const gchar *cName, CairoDockIndicator3Func entry_added, CairoDockIndicator3Func entry_removed, CairoDockIndicator3Func accessible_desc_update, CairoDockIndicator3FuncMenu menu_show, gpointer data)
{
	#ifdef IS_INDICATOR_NG
	if (! bIsIdoInit)
	{
		ido_init(); // need to be init ones...
		bIsIdoInit = TRUE;
	}
	#endif
	cd_debug ("Load: %s", cName);
	g_return_val_if_fail (cName != NULL, NULL);

	IndicatorObject *pIndicator;
	gchar *cFullPath;
	if (g_str_has_suffix (cName, G_MODULE_SUFFIX))
	{
		cFullPath = g_build_filename (INDICATOR_DIR, cName, NULL);
		cd_debug ("Load: %s (%s)", cName, cFullPath);
		pIndicator = indicator_object_new_from_file (cFullPath);
		g_free (cFullPath);
	}
	else
	{
		#ifdef IS_INDICATOR_NG
		GError *error = NULL;
		cFullPath = g_build_filename (INDICATOR_SERVICE_DIR, cName, NULL);
		pIndicator = INDICATOR_OBJECT (indicator_ng_new_for_profile (cFullPath, "desktop", &error));
		g_free (cFullPath);
		if (pIndicator == NULL)
		{
			cd_warning ("could not load indicator from '%s': %s", cName, error->message);
			g_error_free (error);
		}
		#else
		pIndicator = NULL;
		#endif
	}

	if (pIndicator == NULL)
	{
		cd_warning ("Unable to load %s", cName);
		return NULL;
	}

	// indicator_object_set_environment ???

	/* Connect to its signals */
	if (entry_added)
		g_signal_connect (G_OBJECT (pIndicator), INDICATOR_OBJECT_SIGNAL_ENTRY_ADDED,   G_CALLBACK (entry_added),   data);
	if (entry_removed)
		g_signal_connect (G_OBJECT (pIndicator), INDICATOR_OBJECT_SIGNAL_ENTRY_REMOVED, G_CALLBACK (entry_removed), data);
	if (menu_show)
		g_signal_connect (G_OBJECT (pIndicator), INDICATOR_OBJECT_SIGNAL_MENU_SHOW,     G_CALLBACK (menu_show),     data);
	if (accessible_desc_update)
		g_signal_connect (G_OBJECT (pIndicator), INDICATOR_OBJECT_SIGNAL_ACCESSIBLE_DESC_UPDATE, G_CALLBACK (accessible_desc_update), data);
	// other signals
	// INDICATOR_OBJECT_SIGNAL_ENTRY_MOVED
	// INDICATOR_OBJECT_SIGNAL_SECONDARY_ACTIVATE
	// INDICATOR_OBJECT_SIGNAL_SHOW_NOW_CHANGED

	if (entry_added)
	{
		GList *pList = indicator_object_get_entries (pIndicator);
		GList *pEntry = NULL;

		// we can receive more than one entry (e.g. AppMenu)
		for (pEntry = pList; pEntry != NULL; pEntry = g_list_next (pEntry))
		{
			IndicatorObjectEntry *pData = (IndicatorObjectEntry *) pEntry->data;
			entry_added (pIndicator, pData, data);
		}

		g_list_free (pList);
	}
	return pIndicator;
}

void cd_indicator3_unload (IndicatorObject *pIndicator, CairoDockIndicator3Func entry_added, CairoDockIndicator3Func entry_removed, CairoDockIndicator3Func accessible_desc_update, CairoDockIndicator3FuncMenu menu_show, gpointer data)
{
	if (entry_added)
		g_signal_handlers_disconnect_by_func (G_OBJECT (pIndicator), G_CALLBACK (entry_added), data);
	if (entry_removed)
		g_signal_handlers_disconnect_by_func (G_OBJECT (pIndicator), G_CALLBACK (entry_removed), data);
	if (menu_show)
		g_signal_handlers_disconnect_by_func (G_OBJECT (pIndicator), G_CALLBACK (menu_show), data);
	if (accessible_desc_update)
		g_signal_handlers_disconnect_by_func (G_OBJECT (pIndicator), G_CALLBACK (accessible_desc_update), data);
}

const gchar * cd_indicator3_get_label (IndicatorObjectEntry *pEntry)
{
	g_return_val_if_fail (pEntry != NULL, NULL);
	return gtk_label_get_text (pEntry->label);
}

GtkImage * cd_indicator3_get_image (IndicatorObjectEntry *pEntry)
{
	g_return_val_if_fail (pEntry != NULL, NULL);
	return pEntry->image;
}

GtkMenu * cd_indicator3_get_menu (IndicatorObjectEntry *pEntry)
{
	g_return_val_if_fail (pEntry != NULL, NULL);
	return pEntry->menu;
}

const gchar * cd_indicator3_get_accessible_desc (IndicatorObjectEntry *pEntry)
{
	g_return_val_if_fail (pEntry != NULL, NULL);
	return pEntry->accessible_desc;
}

const gchar * cd_indicator3_get_name_hint (IndicatorObjectEntry *pEntry)
{
	g_return_val_if_fail (pEntry != NULL, NULL);
	return pEntry->name_hint;
}

const gchar * cd_indicator3_get_directory_path (void)
{
	return INDICATOR_DIR;
}

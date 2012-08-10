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

#include "applet-struct.h"

#include "applet-backend-alsamixer.h"  // mixer_get_elements_list
#include "applet-notifications.h"
#include "applet-config.h"


CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.card_id = CD_CONFIG_GET_STRING ("Configuration", "card id");
	if (myConfig.card_id == NULL)
		myConfig.card_id = g_strdup ("default");
	
	gchar *cMixerElementName = CD_CONFIG_GET_STRING ("Configuration", "mixer element");
	gchar *cMixerElementName2 = CD_CONFIG_GET_STRING ("Configuration", "mixer element 2");
	if (cMixerElementName != NULL && cMixerElementName2 != NULL && strcmp (cMixerElementName, cMixerElementName2) == 0)
	{
		myConfig.cMixerElementName = g_strconcat (cMixerElementName, ",0", NULL);
		myConfig.cMixerElementName2 = g_strconcat (cMixerElementName, ",1", NULL);
		g_free (cMixerElementName);
	}
	else
	{
		myConfig.cMixerElementName = cMixerElementName;
		myConfig.cMixerElementName2 = cMixerElementName2;
	}
	
	myConfig.cShowAdvancedMixerCommand = CD_CONFIG_GET_STRING ("Configuration", "show mixer");
	
	myConfig.cShortcut = CD_CONFIG_GET_STRING ("Configuration", "shortkey");
	
	myConfig.iScrollVariation = CD_CONFIG_GET_INTEGER ("Configuration", "scroll variation");
	
	myConfig.bHideScaleOnLeave = CD_CONFIG_GET_BOOLEAN ("Configuration", "hide on leave");
	
	
	myConfig.iVolumeDisplay = CD_CONFIG_GET_INTEGER ("Configuration", "display volume");
	
	myConfig.iVolumeEffect = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "display icon", -1);
	if (myConfig.iVolumeEffect >= VOLUME_NB_EFFECTS)  // new option (iVolumeEffect is an unsigned int)
	{
		myConfig.iVolumeEffect = CD_CONFIG_GET_INTEGER ("Configuration", "effect");
		if (myConfig.iVolumeEffect == 1 || myConfig.iVolumeEffect == 2)  // old "zoom" and "transparency" effects
			myConfig.iVolumeEffect = VOLUME_EFFECT_BAR;
		else if (myConfig.iVolumeEffect >= 3)  // other options are offseted by 2
			myConfig.iVolumeEffect -= 2;
		g_key_file_set_integer (CD_APPLET_MY_KEY_FILE, "Configuration", "display icon", myConfig.iVolumeEffect);
	}
	
	if (myConfig.iVolumeEffect == VOLUME_EFFECT_GAUGE)
	{
		myConfig.cGThemePath = CD_CONFIG_GET_GAUGE_THEME ("Configuration", "theme");
		myConfig.iRotateTheme = CD_CONFIG_GET_INTEGER ("Configuration", "rotate theme");
	}
	
	myConfig.cDefaultIcon = CD_CONFIG_GET_STRING ("Configuration", "default icon");
	myConfig.cBrokenIcon = CD_CONFIG_GET_STRING ("Configuration", "broken icon");
	myConfig.cMuteIcon = CD_CONFIG_GET_STRING ("Configuration", "mute icon");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.card_id);
	g_free (myConfig.cMixerElementName);
	g_free (myConfig.cMixerElementName2);
	g_free (myConfig.cShowAdvancedMixerCommand);
	g_free (myConfig.cShortcut);
	g_free (myConfig.cDefaultIcon);
	g_free (myConfig.cBrokenIcon);
	g_free (myConfig.cMuteIcon);
	g_free (myConfig.cGThemePath);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	if (myData.pScale != NULL)
	{
		gtk_widget_destroy (myData.pScale);
		myData.pScale = NULL;
	}
	cairo_dock_dialog_unreference (myData.pDialog);
CD_APPLET_RESET_DATA_END


void cd_mixer_load_custom_widget (CairoDockModuleInstance *myApplet, GKeyFile* pKeyFile, GSList *pWidgetList)
{
	cd_debug ("%s (%s)\n", __func__, myIcon->cName);
	//\____________ On construit la liste des canaux a controler.
	GList *pList = mixer_get_elements_list ();
	
	//\____________ On recupere la combo.
	CairoDockGroupKeyWidget *pGroupKeyWidget = cairo_dock_gui_find_group_key_widget_in_list (pWidgetList, "Configuration", "mixer element");
	
	GtkWidget *pCombo = cairo_dock_gui_get_first_widget (pGroupKeyWidget);
	g_return_if_fail (pCombo != NULL);
	cairo_dock_fill_combo_with_list (pCombo, pList, myConfig.cMixerElementName);
	
	//\____________ Idem pour la 2eme, avec une entree vide au debut.
	pGroupKeyWidget = cairo_dock_gui_find_group_key_widget_in_list (pWidgetList, "Configuration", "mixer element 2");
	pCombo = cairo_dock_gui_get_first_widget (pGroupKeyWidget);
	g_return_if_fail (pCombo != NULL);
	pList = g_list_prepend (pList, (gpointer)"");  // on peut caster ici car tous les elements sont des const pour nous.
	cairo_dock_fill_combo_with_list (pCombo, pList, myConfig.cMixerElementName2);
	
	g_list_free (pList);  // les elements appartiennent au mixer_handle.
}

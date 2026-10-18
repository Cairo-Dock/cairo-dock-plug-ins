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

#include "applet-init.h"

static void _load_custom_widget (GldiModuleInstance *myApplet, GKeyFile* pKeyFile, GSList *pWidgetList);

CD_APPLET_DEFINE2_BEGIN ("Desktop integration",
	CAIRO_DOCK_MODULE_DEFAULT_FLAGS,
	CAIRO_DOCK_CATEGORY_APPLET_SYSTEM,
	N_("This applet provides functions for a better integration into your desktop environnement.\n"
	"It is auto-activated, so you don't need to activate it."),
	"Fabounet (Fabrice Rey) &amp; Tofe (Christophe Chapuis) &amp; Daniel Kondor")
	pInterface->initModule = CD_APPLET_INIT_FUNC;
	// no stop function -> make this plug-in auto-loaded (also, no need to stop it)
	pInterface->reloadModule = CD_APPLET_RELOAD_FUNC;
	pInterface->read_conf_file = CD_APPLET_READ_CONFIG_FUNC;
	pInterface->load_custom_widget = _load_custom_widget;
	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_IS_PLUGIN);
CD_APPLET_DEFINE2_END

static void _start_backend (void)
{
	myData.iCurrentEnv = myConfig.bEnvOverride ? myConfig.iEnvRequested : g_iDesktopEnv;
	switch (myData.iCurrentEnv)
	{
		case CAIRO_DOCK_GNOME:
		case CAIRO_DOCK_CINNAMON:
		case CAIRO_DOCK_MATE:
			cd_gnome_integration_init ();
			break;
		case CAIRO_DOCK_KDE:
			cd_kde_integration_init ();
			break;
		case CAIRO_DOCK_XFCE:
			cd_xfce_integration_init ();
			break;
		case CAIRO_DOCK_COSMIC:
			cd_cosmic_integration_init ();
			break;
		default:
			// de-register in case called from the reload function
			cairo_dock_fm_register_de_integration_override_backend (NULL);
			if (!myConfig.bEnvOverride)
				cd_message ("Unknown or unsupported desktop environment");
			break;
	}
}

CD_APPLET_INIT_BEGIN
	_start_backend ();
CD_APPLET_INIT_END


static void _stop_backend (void)
{
	switch (myData.iCurrentEnv)
	{
		case CAIRO_DOCK_GNOME:
		case CAIRO_DOCK_CINNAMON:
		case CAIRO_DOCK_MATE:
			cd_gnome_integration_stop ();
			break;
		case CAIRO_DOCK_XFCE:
			cd_xfce_integration_stop ();
		default:
			break;
	}
}

static gboolean s_bReloading = FALSE;

CD_APPLET_RELOAD_BEGIN
	_stop_backend ();
	_start_backend ();
	s_bReloading = TRUE;
	CD_APPLET_RELOAD_CONFIG_PANEL;
	s_bReloading = FALSE;
CD_APPLET_RELOAD_END

CD_APPLET_GET_CONFIG_BEGIN
	myConfig.bEnvOverride  = CD_CONFIG_GET_BOOLEAN ("Configuration", "use_de_override");
	myConfig.iEnvRequested = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "de_override", 0);
CD_APPLET_GET_CONFIG_END

static void _load_custom_widget (GldiModuleInstance *myApplet, GKeyFile* pKeyFile, GSList *pWidgetList)  // myApplet can be NULL
{
	CairoDockGroupKeyWidget *pGroupKeyWidget = cairo_dock_gui_find_group_key_widget_in_list (pWidgetList, "Configuration", "de_detected");
	if (!pGroupKeyWidget) return; // cd_warning()?
	
	GString *sLabel = g_string_new (NULL);
	const gchar *cStatus = NULL;
	if (!s_bReloading) switch (myData.iCurrentEnv)
	{
		case CAIRO_DOCK_GNOME:
		case CAIRO_DOCK_CINNAMON:
		case CAIRO_DOCK_MATE:
			cStatus = cd_gnome_integration_get_status ();
			break;
		case CAIRO_DOCK_XFCE:
			cStatus = cd_xfce_integration_get_status ();
		default:
			break;
	}
	g_string_append_printf (sLabel, "%s %s", D_("Detected desktop environment:"), cairo_dock_fm_get_desktop_name ());
	if (myData.iCurrentEnv != g_iDesktopEnv)
	{
		const char *cOverride;
		if (myData.iCurrentEnv != CAIRO_DOCK_UNKNOWN_ENV)
		{
			//!! TODO: create a proper function for this
			CairoDockDesktopEnv tmp = g_iDesktopEnv;
			g_iDesktopEnv = myData.iCurrentEnv;
			cOverride = cairo_dock_fm_get_desktop_name ();
			g_iDesktopEnv = tmp;
		}
		else cOverride = D_("Disabled");
		
		g_string_append_printf (sLabel, "\n%s %s", D_("User-defined override:"), cOverride);
	}
	if (cStatus) g_string_append_printf (sLabel, "\n%s", cStatus);
	
	GtkWidget *pLabel = gtk_label_new (sLabel->str);
	// gtk_label_set_justify (GTK_LABEL (pLabel), GTK_JUSTIFY_RIGHT);
	gtk_box_pack_start (GTK_BOX (pGroupKeyWidget->pKeyBox), pLabel, FALSE, FALSE, 0);
	g_string_free (sLabel, TRUE);
}



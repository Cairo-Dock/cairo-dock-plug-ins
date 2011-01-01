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

/******************************************************************************
exemples : 
----------

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.CreateLauncherFromScratch string:gimp.png string:"fake gimp" string:gimp string:none

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.SetLabel string:new_label string:icon_name string:any string:none

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.SetQuickInfo string:123 string:none string:none string:dustbin

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.Animate string:default int32:2 string:any string:firefox string:none

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.SetIcon string:firefox-3.0 string:any string:nautilus string:none

******************************************************************************/

#include <unistd.h>
#include <glib.h>

#include "interface-main-methods.h"

#define nullify_argument(string) do {\
	if (string != NULL && (*string == '\0' || strcmp (string, "any") == 0 || strcmp (string, "none") == 0))\
		string = NULL; } while (0)

static gboolean dbus_deskletVisible = FALSE;
static guint dbus_xLastActiveWindow;

gboolean cd_dbus_main_reboot(dbusMainObject *pDbusCallback, GError **error)
{
	if (! myConfig.bEnableReboot)
		return FALSE;
	cairo_dock_load_current_theme ();
	return TRUE;
}

gboolean cd_dbus_main_quit (dbusMainObject *pDbusCallback, GError **error)
{
	if (! myConfig.bEnableQuit)
		return FALSE;
	gtk_main_quit ();
	return TRUE;
}

gboolean cd_dbus_main_reload_module (dbusMainObject *pDbusCallback, const gchar *cModuleName, GError **error)
{
	if (! myConfig.bEnableReloadModule)
		return FALSE;
	CairoDockModule *pModule = cairo_dock_find_module_from_name (cModuleName);
	if (pModule != NULL)
	{
		cairo_dock_reload_module (pModule, TRUE);  // TRUE <=> reload module conf file.
	}
	else
	{
		//CairoDockInternalModule *pInternalModule = cairo_dock_find_internal_module_from_name (cModuleName);
		GldiManager *pManager = gldi_get_manager (cModuleName);
		if (pManager != NULL)
		{
			//cairo_dock_reload_internal_module (pInternalModule, g_cConfFile);
			gldi_reload_manager (pManager, g_cConfFile);
		}
		else
		{
			cd_warning ("no module named '%s'", cModuleName);
			return FALSE;
		}
	}
	return TRUE;
}

gboolean cd_dbus_main_activate_module (dbusMainObject *pDbusCallback, const gchar *cModuleName, gboolean bActivate, GError **error)
{
	if (! myConfig.bEnableActivateModule)
		return FALSE;
	
	CairoDockModule *pModule = cairo_dock_find_module_from_name (cModuleName);
	if (pModule == NULL)
	{
		/*if (cairo_dock_find_internal_module_from_name (cModuleName) != NULL)
			cd_warning ("Internal modules can't be (de)activated.");
		else*/
			cd_warning ("no such module (%s)", cModuleName);
		return FALSE;
	}
	
	if (bActivate)
		cairo_dock_activate_module_and_load (cModuleName);
	else
		cairo_dock_deactivate_module_and_unload (cModuleName);
	return TRUE;
}

gboolean cd_dbus_main_show_desklet(dbusMainObject *pDbusCallback, gboolean *widgetLayer, GError **error)
{
	if (! myConfig.bEnableDesklets)
		return FALSE;
	if (dbus_deskletVisible)
	{
		cairo_dock_set_desklets_visibility_to_default ();
		cairo_dock_show_xwindow (dbus_xLastActiveWindow);
	}
	else
	{
		dbus_xLastActiveWindow = cairo_dock_get_current_active_window ();
		cairo_dock_set_all_desklets_visible (widgetLayer != NULL ? *widgetLayer : FALSE);
	}
	dbus_deskletVisible = !dbus_deskletVisible;
	return TRUE;
}

static void _show_hide_one_dock (const gchar *cDockName, CairoDock *pDock, gpointer data)
{
	if (pDock->iRefCount != 0)
		return ;
	gboolean bShow = GPOINTER_TO_INT (data);
	if (bShow)
	{
		///cairo_dock_pop_up (pDock);
		if (pDock->bAutoHide)
			cairo_dock_emit_enter_signal (CAIRO_CONTAINER (pDock));
	}
	else
	{
		///cairo_dock_pop_down (pDock);  // ne fait rien s'il n'etait pas "popped".
		if (pDock->bAutoHide)
			cairo_dock_emit_leave_signal (CAIRO_CONTAINER (pDock));
	}
}
gboolean cd_dbus_main_show_dock (dbusMainObject *pDbusCallback, gboolean bShow, GError **error)
{
	if (! myConfig.bEnableShowDock)
		return FALSE;
	
	if (bShow)
		cairo_dock_stop_quick_hide ();
	
	cairo_dock_foreach_docks ((GHFunc) _show_hide_one_dock, GINT_TO_POINTER (bShow));
	
	if (! bShow)
		cairo_dock_quick_hide_all_docks ();
	
	return TRUE;
}

gboolean cd_dbus_main_create_launcher_from_scratch (dbusMainObject *pDbusCallback, const gchar *cIconFile, const gchar *cLabel, const gchar *cCommand, const gchar *cParentDockName, GError **error)
{
	if (! myConfig.bEnableCreateLauncher)
		return FALSE;
	
	nullify_argument (cParentDockName);
	if (cParentDockName == NULL)
		cParentDockName = CAIRO_DOCK_MAIN_DOCK_NAME;
	
	CairoDock *pParentDock = cairo_dock_search_dock_from_name (cParentDockName);
	if (pParentDock == NULL)
	{
		cd_message ("le dock parent (%s) n'existe pas, on le cree", cParentDockName);
		pParentDock = cairo_dock_create_dock (cParentDockName, NULL);
	}
	
	Icon *pIcon = cairo_dock_create_dummy_launcher (g_strdup (cLabel),
		g_strdup (cIconFile),
		g_strdup (cCommand),
		NULL,
		CAIRO_DOCK_LAST_ORDER);
	pIcon->cParentDockName = g_strdup (cParentDockName);
	cairo_dock_set_launcher_class (pIcon, NULL);
	
	cairo_dock_load_icon_buffers (pIcon, CAIRO_CONTAINER (pParentDock));
	
	cairo_dock_insert_icon_in_dock (pIcon, pParentDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON);
	cairo_dock_launch_animation (CAIRO_CONTAINER (pParentDock));
	
	if (pIcon->cClass != NULL)
	{
		cairo_dock_inhibite_class (pIcon->cClass, pIcon);
	}
	
	return TRUE;
}

gboolean cd_dbus_main_load_launcher_from_file (dbusMainObject *pDbusCallback, const gchar *cDesktopFile, GError **error)  // pris de cairo_dock_add_new_launcher_by_uri().
{
	if (! myConfig.bEnableTweakingLauncher)
		return FALSE;
	g_return_val_if_fail (cDesktopFile != NULL, FALSE);
	
	Icon *pIcon = cairo_dock_create_icon_from_desktop_file (cDesktopFile);
	
	if (pIcon == NULL)
	{
		cd_warning ("the icon couldn't be created, check that the file '%s' exists and is a valid and readable .desktop file\n", cDesktopFile);
		return FALSE;
	}
	
	CairoDock * pParentDock = cairo_dock_search_dock_from_name (pIcon->cParentDockName);
	if (pParentDock != NULL)  // a priori toujours vrai puisqu'il est cree au besoin. En fait c'est probablement le main dock pour un .desktop de base.
	{
		cairo_dock_insert_icon_in_dock (pIcon, pParentDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON);
		cairo_dock_start_icon_animation (pIcon, pParentDock);
	}
	cd_debug (" => cDesktopFileName : %s\n", pIcon->cDesktopFileName);
	
	return TRUE;
}

static void _find_launcher_in_dock (Icon *pIcon, CairoDock *pDock, gpointer *data)
{
	gchar *cDesktopFile = data[0];
	Icon **pFoundIcon = data[1];
	if (pIcon->cDesktopFileName && g_ascii_strncasecmp (cDesktopFile, pIcon->cDesktopFileName, strlen (cDesktopFile)) == 0)
	{
		*pFoundIcon = pIcon;
	}
}
Icon *cd_dbus_find_launcher (const gchar *cDesktopFile)
{
	Icon *pIcon = NULL;
	gpointer data[2];
	data[0] = (gpointer) cDesktopFile;
	data[1] = &pIcon;
	cairo_dock_foreach_icons_in_docks ((CairoDockForeachIconFunc) _find_launcher_in_dock, data);
	return pIcon;
}

gboolean cd_dbus_main_reload_launcher (dbusMainObject *pDbusCallback, const gchar *cDesktopFile, GError **error)
{
	if (! myConfig.bEnableTweakingLauncher)
		return FALSE;
	
	nullify_argument (cDesktopFile);
	g_return_val_if_fail (cDesktopFile != NULL, FALSE);
	
	Icon *pIcon = cd_dbus_find_launcher (cDesktopFile);
	if (pIcon == NULL)
		return FALSE;
	
	cairo_dock_reload_launcher (pIcon);
	
	return TRUE;
}

gboolean cd_dbus_main_remove_launcher (dbusMainObject *pDbusCallback, const gchar *cDesktopFile, GError **error)
{
	if (! myConfig.bEnableTweakingLauncher)
		return FALSE;
	
	nullify_argument (cDesktopFile);
	g_return_val_if_fail (cDesktopFile != NULL, FALSE);
	
	Icon *pIcon = cd_dbus_find_launcher (cDesktopFile);
	if (pIcon == NULL)
		return FALSE;
	
	if (pIcon->pSubDock != NULL)  // on detruit le sous-dock et ce qu'il contient.
	{
		cairo_dock_destroy_dock (pIcon->pSubDock, (pIcon->cClass != NULL ? pIcon->cClass : pIcon->cName));
		pIcon->pSubDock = NULL;
	}
	
	cairo_dock_trigger_icon_removal_from_dock (pIcon);
	
	return TRUE;
}




static void _find_icon_in_dock (Icon *pIcon, CairoDock *pDock, gpointer *data)
{
	gchar *cIconName = data[0];
	gchar *cIconCommand = data[1];
	Icon **pFoundIcon = data[2];
	gchar *cName = (pIcon->cInitialName != NULL ? pIcon->cInitialName : pIcon->cName);
	//g_print ("%s (%s/%s, %s/%s)\n", __func__, cName, cIconName, pIcon->cCommand, cIconCommand);
	if ((cIconName == NULL || (cName && g_ascii_strncasecmp (cIconName, cName, strlen (cIconName)) == 0)) &&
		(cIconCommand == NULL || (pIcon->cCommand && g_ascii_strncasecmp (cIconCommand, pIcon->cCommand, strlen (cIconCommand)) == 0) || (pIcon->cClass && g_ascii_strncasecmp (cIconCommand, pIcon->cClass, strlen (cIconCommand)) == 0)))  // si un des parametre est non nul, alors on exige qu'il corresponde. 'cIconCommand' peut correspondre soit avec la commande, soit avec la classe.
	{
		Icon *icon = *pFoundIcon;
		if (icon != NULL)  // on avait deja trouve une icone avant.
		{
			cName = (pIcon->cInitialName != NULL ? pIcon->cInitialName : pIcon->cName);
			if ((cIconName && cName && g_ascii_strcasecmp (cIconName, cName) == 0) ||
				(cIconCommand && pIcon->cCommand && g_ascii_strcasecmp (cIconCommand, pIcon->cCommand)))  // elle satisfait entierement aux criteres, donc on la garde.
				return ;
		}
		*pFoundIcon = pIcon;
	}
}
Icon *cd_dbus_find_icon (const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName)
{
	Icon *pIcon = NULL;
	if (cModuleName != NULL)  // c'est une icone d'un des modules.
	{
		CairoDockModule *pModule = cairo_dock_find_module_from_name (cModuleName);
		g_return_val_if_fail (pModule != NULL, FALSE);
		
		if (pModule->pInstancesList != NULL)
		{
			CairoDockModuleInstance *pModuleInstance = pModule->pInstancesList->data;
			if (pModuleInstance != NULL)
				pIcon = pModuleInstance->pIcon;
		}
	}
	else if (cIconName || cIconCommand) // on cherche une icone de lanceur.
	{
		gpointer data[3];
		data[0] = (gpointer) cIconName;
		data[1] = (gpointer) cIconCommand;
		data[2] = &pIcon;
		cairo_dock_foreach_icons_in_docks ((CairoDockForeachIconFunc) _find_icon_in_dock, data);
	}
	return pIcon;
}
gboolean cd_dbus_main_set_quick_info (dbusMainObject *pDbusCallback, const gchar *cQuickInfo, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error)
{
	if (! myConfig.bEnableSetQuickInfo)
		return FALSE;
	
	nullify_argument (cIconName);
	nullify_argument (cIconCommand);
	nullify_argument (cModuleName);
	nullify_argument (cQuickInfo);
	
	Icon *pIcon = cd_dbus_find_icon (cIconName, cIconCommand, cModuleName);
	if (pIcon == NULL)
		return FALSE;
	
	CairoContainer *pContainer = cairo_dock_search_container_from_icon (pIcon);
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	cairo_dock_set_quick_info (pIcon, pContainer, cQuickInfo);
	cairo_dock_redraw_icon (pIcon, pContainer);
	return TRUE;
}

gboolean cd_dbus_main_set_label (dbusMainObject *pDbusCallback, const gchar *cLabel, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error)
{
	if (! myConfig.bEnableSetLabel)
		return FALSE;
	
	nullify_argument (cIconName);
	nullify_argument (cIconCommand);
	nullify_argument (cModuleName);
	nullify_argument (cLabel);
	
	Icon *pIcon = cd_dbus_find_icon (cIconName, cIconCommand, cModuleName);
	if (pIcon == NULL)
		return FALSE;
	
	CairoContainer *pContainer = cairo_dock_search_container_from_icon (pIcon);
	g_return_val_if_fail (pContainer != NULL, FALSE);
	cairo_dock_set_icon_name (cLabel, pIcon, pContainer);
	return TRUE;
}

gboolean cd_dbus_main_set_icon (dbusMainObject *pDbusCallback, const gchar *cImage, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error)
{
	if (! myConfig.bEnableSetIcon)
		return FALSE;
	
	nullify_argument (cIconName);
	nullify_argument (cIconCommand);
	nullify_argument (cModuleName);
	
	Icon *pIcon = cd_dbus_find_icon (cIconName, cIconCommand, cModuleName);
	if (pIcon == NULL)
		return FALSE;
	
	CairoContainer *pContainer = cairo_dock_search_container_from_icon (pIcon);
	g_return_val_if_fail (pContainer != NULL, FALSE);
	g_return_val_if_fail (pIcon->pIconBuffer != NULL, FALSE);
	cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
	cairo_dock_set_image_on_icon (pIconContext, cImage, pIcon, pContainer);
	cairo_destroy (pIconContext);
	cairo_dock_redraw_icon (pIcon, pContainer);
	return TRUE;
}

gboolean cd_dbus_main_set_emblem (dbusMainObject *pDbusCallback, const gchar *cImage, gint iPosition, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error)
{
	if (! myConfig.bEnableSetIcon)
		return FALSE;
	
	nullify_argument (cIconName);
	nullify_argument (cIconCommand);
	nullify_argument (cModuleName);
	
	Icon *pIcon = cd_dbus_find_icon (cIconName, cIconCommand, cModuleName);
	if (pIcon == NULL)
		return FALSE;
	
	CairoContainer *pContainer = cairo_dock_search_container_from_icon (pIcon);
	g_return_val_if_fail (pContainer != NULL, FALSE);
	g_return_val_if_fail (pIcon->pIconBuffer != NULL, FALSE);
	cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
	
	CairoEmblem *pEmblem = cairo_dock_make_emblem (cImage, pIcon, pContainer);
	pEmblem->iPosition = iPosition;
	cairo_dock_draw_emblem_on_icon (pEmblem, pIcon, pContainer);
	cairo_dock_free_emblem (pEmblem);
	
	cairo_destroy (pIconContext);
	cairo_dock_redraw_icon (pIcon, pContainer);
	return TRUE;
}

gboolean cd_dbus_main_animate (dbusMainObject *pDbusCallback, const gchar *cAnimation, gint iNbRounds, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error)
{
	if (! myConfig.bEnableAnimateIcon || cAnimation == NULL)
		return FALSE;
	
	nullify_argument (cIconName);
	nullify_argument (cIconCommand);
	nullify_argument (cModuleName);
	
	Icon *pIcon = cd_dbus_find_icon (cIconName, cIconCommand, cModuleName);
	if (pIcon == NULL)
		return FALSE;
	
	CairoContainer *pContainer = cairo_dock_search_container_from_icon (pIcon);
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	if (! CAIRO_DOCK_IS_DOCK (pContainer))
		return FALSE;
	
	cairo_dock_request_icon_animation (pIcon, CAIRO_DOCK (pContainer), cAnimation, iNbRounds);
	return TRUE;
}

gboolean cd_dbus_main_show_dialog (dbusMainObject *pDbusCallback, const gchar *message, gint iDuration, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error)
{
	if (! myConfig.bEnablePopUp)
		return FALSE;
	g_return_val_if_fail (message != NULL, FALSE);
	
	nullify_argument (cIconName);
	nullify_argument (cIconCommand);
	nullify_argument (cModuleName);
	
	Icon *pIcon = cd_dbus_find_icon (cIconName, cIconCommand, cModuleName);
	if (pIcon != NULL)
	{
		CairoContainer *pContainer = cairo_dock_search_container_from_icon (pIcon);
		if (pContainer != NULL)
		{
			cairo_dock_show_temporary_dialog_with_icon (message, pIcon, pContainer, 1000 * iDuration, "same icon");
			return TRUE;
		}
	}
	
	cairo_dock_show_general_message (message, 1000 * iDuration);
	return TRUE;
}

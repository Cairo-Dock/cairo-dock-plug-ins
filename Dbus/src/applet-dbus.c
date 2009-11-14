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

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.CreateLauncherFromScratch string:inkscape string:yep string:rien string:none

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.SetLabel string:new_label string:icon_name string:any string:none

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.SetQuickInfo string:123 string:none string:none string:dustbin

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.Animate string:default int32:2 string:any string:firefox string:none

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.SetIcon string:firefox-3.0 string:any string:nautilus string:none

******************************************************************************/

#include <unistd.h>
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include "applet-dbus.h"
#include "dbus-main-spec.h"
#include "interface-applet-object.h"
#include "interface-applet-signals.h"

#define nullify_argument(string) do {\
	if (string != NULL && (*string == '\0' || strcmp (string, "any") == 0 || strcmp (string, "none") == 0))\
		string = NULL; } while (0)

static gboolean dbus_deskletVisible = FALSE;
static guint dbus_xLastActiveWindow;

G_DEFINE_TYPE(dbusMainObject, cd_dbus_main, G_TYPE_OBJECT);


static void cd_dbus_main_class_init(dbusMainObjectClass *klass)
{
	cd_message("");
}
static void cd_dbus_main_init (dbusMainObject *pMainObject)
{
	cd_message("");
	
	// Initialise the DBus connection
	pMainObject->connection = cairo_dock_get_session_connection ();
	
	dbus_g_object_type_install_info(cd_dbus_main_get_type(), &dbus_glib_cd_dbus_main_object_info);
	
	// Register DBUS path
	dbus_g_connection_register_g_object(pMainObject->connection, "/org/cairodock/CairoDock", G_OBJECT(pMainObject));
}

static void _cd_dbus_launch_third_party_applets (const gchar *cDirPath)
{
	GError *erreur = NULL;
	const gchar *cFileName;
	gchar *cThirdPartyPath = g_strdup_printf ("%s/%s", cDirPath, "third-party");
	GDir *dir = g_dir_open (cThirdPartyPath, 0, &erreur);
	if (erreur != NULL)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	GString *sPath = g_string_new ("");
	do
	{
		cFileName = g_dir_read_name (dir);
		if (cFileName == NULL)
			break ;
		g_string_printf (sPath, "%s/%s", cThirdPartyPath, cFileName);
		cd_dbus_launch_distant_applet_in_dir (cFileName, sPath->str);
	}
	while (1);
	g_dir_close (dir);
	g_string_free (sPath, TRUE);
	g_free (cThirdPartyPath);
}

void cd_dbus_launch_service (void)
{
	g_return_if_fail (myData.pMainObject == NULL);
	g_type_init();
	
	// on cree l'objet distant principal.
	cd_message("dbus : Lancement du service");
	myData.pMainObject = g_object_new (cd_dbus_main_get_type(), NULL);  // appelle cd_dbus_main_class_init() et cd_dbus_main_init().
	
	// Register the service name
	cairo_dock_register_service_name ("org.cairodock.CairoDock");
	
	// on lance les applets distantes.
	_cd_dbus_launch_third_party_applets (MY_APPLET_SHARE_DATA_DIR);
	
	_cd_dbus_launch_third_party_applets (g_cCairoDockDataDir);
}

void cd_dbus_stop_service (void)
{
	// on vire tous les modules distants.
	myData.bServiceIsStopping = TRUE;  // on stoppe les applets distantes differemment suivant que c'est l'utilisateur qui la decoche ou pas.
	dbusApplet *pDbusApplet;
	GList *a;
	for (a = myData.pAppletList; a != NULL; a = a->next)
	{
		pDbusApplet = a->data;
		cairo_dock_unregister_module (pDbusApplet->cModuleName);  // stoppe le module (toutes les instances), ce qui appelle stop_module, qui emet le signal 'stop' vers l'applet distante (qui du coup quitte), et enleve le module de la table. l'objet distant n'est pas detruit, puisque de toute facon on vide toute la liste.
		g_object_unref (pDbusApplet);
	}
	g_list_free (myData.pAppletList);
	myData.pAppletList = NULL;
	
	// on se desabonne de toutes les notifications.
	cd_dbus_unregister_notifications ();
	
	// on vire le main object.
	if (myData.pMainObject != NULL)
		g_object_unref (myData.pMainObject);
	myData.pMainObject = NULL;
	myData.bServiceIsStopping = FALSE;
}



gboolean cd_dbus_main_reboot(dbusMainObject *pDbusCallback, GError **error)
{
	if (! myConfig.bEnableReboot)
		return FALSE;
	cairo_dock_read_conf_file (g_cConfFile, g_pMainDock);
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
		CairoDockInternalModule *pInternalModule = cairo_dock_find_internal_module_from_name (cModuleName);
		if (pInternalModule != NULL)
		{
			cairo_dock_reload_internal_module (pInternalModule, g_cConfFile);
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
		if (cairo_dock_find_internal_module_from_name (cModuleName) != NULL)
			cd_warning ("Internal modules can't be (de)activated.");
		else
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
		cairo_dock_pop_up (pDock);
		if (pDock->bAutoHide)
			cairo_dock_emit_enter_signal (pDock);
	}
	else
	{
		cairo_dock_pop_down (pDock);  // ne fait rien s'il n'etait pas "popped".
		if (pDock->bAutoHide)
			cairo_dock_emit_leave_signal (pDock);
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
		pParentDock = cairo_dock_create_new_dock (cParentDockName, NULL);
	}
	
	Icon *pIcon = g_new0 (Icon, 1);
	pIcon->iType = CAIRO_DOCK_LAUNCHER;
	pIcon->cFileName = g_strdup (cIconFile);
	pIcon->cName = g_strdup (cLabel);
	pIcon->cCommand = g_strdup (cCommand);
	pIcon->cParentDockName = g_strdup (cParentDockName);
	pIcon->cDesktopFileName = g_strdup ("none");
	pIcon->fOrder = CAIRO_DOCK_LAST_ORDER;
	
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pParentDock));
	cairo_dock_fill_icon_buffers_for_dock (pIcon, pCairoContext, pParentDock);
	cairo_destroy (pCairoContext);
	
	cairo_dock_insert_icon_in_dock (pIcon, pParentDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON);
	
	return TRUE;
}

gboolean cd_dbus_main_load_launcher_from_file (dbusMainObject *pDbusCallback, const gchar *cDesktopFile, GError **error)  // pris de cairo_dock_add_new_launcher_by_uri().
{
	if (! myConfig.bEnableTweakingLauncher)
		return FALSE;
	g_return_val_if_fail (cDesktopFile != NULL, FALSE);
	
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (g_pMainDock));
	Icon *pIcon = cairo_dock_create_icon_from_desktop_file (cDesktopFile, pCairoContext);
	cairo_destroy (pCairoContext);
	
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
	g_print (" => cDesktopFileName : %s\n", pIcon->cDesktopFileName);
	
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
		cairo_dock_destroy_dock (pIcon->pSubDock, (pIcon->cClass != NULL ? pIcon->cClass : pIcon->cName), NULL, NULL);
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
		(cIconCommand == NULL || (pIcon->cCommand && g_ascii_strncasecmp (cIconCommand, pIcon->cCommand, strlen (cIconCommand)) == 0)))
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
	
	Icon *pIcon = cd_dbus_find_icon (cIconName, cIconCommand, cModuleName);
	if (pIcon == NULL)
		return FALSE;
	
	CairoContainer *pContainer = cairo_dock_search_container_from_icon (pIcon);
	g_return_val_if_fail (pContainer != NULL, FALSE);
	double fMaxScale = cairo_dock_get_max_scale (pContainer);
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pContainer);
	cairo_dock_set_quick_info (pCairoContext, cQuickInfo, pIcon, fMaxScale);
	cairo_destroy (pCairoContext);
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
	
	Icon *pIcon = cd_dbus_find_icon (cIconName, cIconCommand, cModuleName);
	if (pIcon == NULL)
		return FALSE;
	
	CairoContainer *pContainer = cairo_dock_search_container_from_icon (pIcon);
	g_return_val_if_fail (pContainer != NULL, FALSE);
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pContainer);
	cairo_dock_set_icon_name (pCairoContext, cLabel, pIcon, pContainer);
	cairo_destroy (pCairoContext);
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



static gboolean _emit_init_module_delayed (CairoDockModuleInstance *pInstance)
{
	cd_dbus_emit_init_signal (pInstance);
	return FALSE;
}
gboolean cd_dbus_main_register_new_module (dbusMainObject *pDbusCallback, const gchar *cModuleName, const gchar *cDescription, const gchar *cAuthor, const gchar *cVersion, gint iCategory, const gchar *cShareDataDir, GError **error)
{
	if (! myConfig.bEnableNewModule)
		return FALSE;
	
	g_print ("%s (%s)\n", __func__, cModuleName);
	// on cree et on enregistre un nouveau module s'il n'existe pas deja.
	CairoDockModule *pModule = cairo_dock_find_module_from_name (cModuleName);
	if (pModule != NULL)
	{
		g_print ("this module (%s) is already registered\n", cModuleName);
		if (pModule->cSoFilePath != NULL)
		{
			cd_warning ("an installed module already exists with this name (%s).", cModuleName);
			return FALSE;  // eventuellement on pourrait prendre le controle d'une applet comme ca !
		}
	}
	else
	{
		pModule = g_new0 (CairoDockModule, 1);
		CairoDockVisitCard *pVisitCard = g_new0 (CairoDockVisitCard, 1);
		pModule->pVisitCard = pVisitCard;
		pVisitCard->cModuleName = g_strdup (cModuleName);
		pVisitCard->iMajorVersionNeeded = 2;
		pVisitCard->iMinorVersionNeeded = 1;
		pVisitCard->iMicroVersionNeeded = 0;
		pVisitCard->cPreviewFilePath = cShareDataDir ? g_strdup_printf ("%s/preview", cShareDataDir) : NULL;
		pVisitCard->cGettextDomain = g_strdup_printf ("cd-%s", cModuleName);
		pVisitCard->cUserDataDir = g_strdup (cModuleName);
		pVisitCard->cShareDataDir = g_strdup (cShareDataDir);
		pVisitCard->cConfFileName = g_strdup_printf ("%s.conf", cModuleName);
		pVisitCard->cModuleVersion = g_strdup (cVersion);
		pVisitCard->cAuthor = g_strdup (cAuthor);
		pVisitCard->iCategory = iCategory;
		pVisitCard->cIconFilePath = cShareDataDir ? g_strdup_printf ("%s/icon", cShareDataDir) : NULL;
		pVisitCard->iSizeOfConfig = 4;  // au cas ou ...
		pVisitCard->iSizeOfData = 4;  // au cas ou ...
		pVisitCard->cDescription = g_strdup (cDescription);
		//pVisitCard->bMultiInstance = TRUE;
		pModule->pInterface = g_new0 (CairoDockModuleInterface, 1);
		pModule->pInterface->initModule = cd_dbus_emit_on_init_module;
		pModule->pInterface->stopModule = cd_dbus_emit_on_stop_module;
		pModule->pInterface->reloadModule = cd_dbus_emit_on_reload_module;
		cairo_dock_load_manual_module (pModule);
		
		if (pModule->pVisitCard->cDockVersionOnCompilation == NULL)
		{
			cairo_dock_free_module (pModule);
			cd_warning ("registration of '%s' has failed.", cModuleName);
			return FALSE;
		}
	}
	
	// si l'applet ne doit pas etre utilisee, on en reste la.
	gboolean bAppletIsUsed = cd_dbus_applet_is_used (cModuleName);
	if (! bAppletIsUsed)
	{
		g_print ("applet %s has been registered, but is not wanted by the user.\n", cModuleName);
		return TRUE;
	}
	
	// sinon on active le module.
	pModule->fLastLoadingTime = -1;  // indique a la fonction d'init que c'est l'applet qui demande a s'activer (activation automatique ou manuelle), plutot que l'utilisateur (activation par panneau de conf).
	GError *tmp_erreur = NULL;
	gboolean bAlreadyInstanciated = FALSE;
	cairo_dock_activate_module (pModule, &tmp_erreur);
	if (tmp_erreur != NULL)
	{
		cd_warning ("%s (maybe the applet didn't stop correctly before)", tmp_erreur->message);
		g_error_free (tmp_erreur);
		tmp_erreur = NULL;
		bAlreadyInstanciated = TRUE;  // donc on n'est pas passe dans l'init, ce qui n'est pas grave puisque de toute facon on le lance avec un delai, et l'objet distant existait deja.
	}
	g_return_val_if_fail (pModule->pInstancesList != NULL, FALSE);
	
	// on retarde l'emission du signal 'init'.
	CairoDockModuleInstance *pInstance = pModule->pInstancesList->data;
	if (! bAlreadyInstanciated)
	{
		if (pInstance->pDock)
		{
			cairo_dock_update_dock_size (pInstance->pDock);
			cairo_dock_redraw_container (pInstance->pContainer);
		}
	}
	else  // cas ou l'applet etait deja instanciee, on simule un stop/init pour repartir sur de bonnes bases.
	{
		g_print ("applet already instanciated before, reset it\n");
		cd_dbus_action_on_stop_module (pInstance);
		cd_dbus_action_on_init_module (pInstance);
	}
	g_timeout_add (500, (GSourceFunc)_emit_init_module_delayed, pInstance);  // petit hack car l'applet est en train d'attendre le retour de cette fonction. Elle ne peut donc pas recuperer l'objet maintenant. On laisse une tempo pour cela.
	
	g_print ("applet has been successfully instanciated, will be initialized in 500ms...\n");
	///pModule->fLastLoadingTime = time (NULL) + 1e6;  // pour ne pas qu'il soit desactive lors d'un reload general, car il n'est pas dans la liste des modules actifs du fichier de conf. => a priori maintenant il l'est.
	return TRUE;
}

gboolean cd_dbus_main_unregister_module (dbusMainObject *pDbusCallback, const gchar *cModuleName, GError **error)
{
	CairoDockModule *pModule = cairo_dock_find_module_from_name (cModuleName);
	g_return_val_if_fail (pModule != NULL, FALSE);
	
	if (pModule->cSoFilePath != NULL)
	{
		cd_warning ("can't unregister installed modules, only distant modules can");
		return FALSE;
	}
	
	if (pModule->pInstancesList != NULL)  // on le fait maintenant pour ne pas lancer le signal 'stop' (en effet c'est l'applet elle-meme qui envoie ce signal lorsqu'elle se termine), et aussi car l'instance est encore disponible.
	{
		CairoDockModuleInstance *pInstance = pModule->pInstancesList->data;
		cd_dbus_delete_remote_applet_object (pInstance);
	}
	
	cairo_dock_unregister_module (cModuleName);  // stoppe le module (toutes les instances) et l'enleve de la table.
	
	return TRUE;
}

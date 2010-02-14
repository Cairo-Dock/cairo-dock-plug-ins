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

#include "interface-main-methods.h"
#include "dbus-main-spec.h"
#include "interface-applet-object.h"
#include "interface-applet-signals.h"
#include "applet-dbus.h"

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

gboolean cd_dbus_register_module_in_dir (const gchar *cModuleName, const gchar *cThirdPartyPath)
{
	cd_debug ("%s (%s, %s)", __func__, cModuleName, cThirdPartyPath);
	gchar *cFilePath = g_strdup_printf ("%s/%s/auto-load.conf", cThirdPartyPath, cModuleName);
	GKeyFile *pKeyFile = cairo_dock_open_key_file (cFilePath);
	g_return_val_if_fail (pKeyFile != NULL, FALSE);
	
	GError *error = NULL;
	
	gchar *cDescription = g_key_file_get_string (pKeyFile, "Register", "description", &error);
	if (error != NULL)
	{
		cd_warning (error->message);
		g_error_free (error);
		error = NULL;
	}
	gchar *cAuthor = g_key_file_get_string (pKeyFile, "Register", "author", &error);
	if (error != NULL)
	{
		cd_warning (error->message);
		g_error_free (error);
		error = NULL;
	}
	gchar *cVersion = g_key_file_get_string (pKeyFile, "Register", "version", &error);
	if (error != NULL)
	{
		cd_warning (error->message);
		g_error_free (error);
		error = NULL;
	}
	int iCategory = g_key_file_get_integer (pKeyFile, "Register", "category", &error);
	if (error != NULL)
	{
		cd_warning (error->message);
		g_error_free (error);
		error = NULL;
	}
	
	gchar *cShareDataDir = g_strdup_printf ("%s/%s", cThirdPartyPath, cModuleName);
	
	g_key_file_free (pKeyFile);
	
	gboolean bActivationOk = cd_dbus_register_new_module (cModuleName, cDescription, cAuthor, cVersion, iCategory, cShareDataDir);
	g_free (cDescription);
	g_free (cAuthor);
	g_free (cVersion);
	g_free (cShareDataDir);
	g_free (cFilePath);
	return bActivationOk;
}

static void _cd_dbus_launch_third_party_applets_in_dir (const gchar *cDirPath)
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
	
	do
	{
		cFileName = g_dir_read_name (dir);
		if (cFileName == NULL)
			break ;
		
		cd_dbus_register_module_in_dir (cFileName, cThirdPartyPath);
	}
	while (1);
	g_dir_close (dir);
	g_free (cThirdPartyPath);
}

static gboolean _cd_dbus_launch_third_party_applets (gpointer data)
{
	_cd_dbus_launch_third_party_applets_in_dir (MY_APPLET_SHARE_DATA_DIR);
	
	_cd_dbus_launch_third_party_applets_in_dir (g_cCairoDockDataDir);
	return FALSE;
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
	g_idle_add ((GSourceFunc) _cd_dbus_launch_third_party_applets, NULL);  // on les lance avec un delai, car si l'applet DBus est lancee au demarrage du dock, on est dans la fonction cairo_dock_activate_modules_from_list(), et donc si on enregistre des applets pendant ce temps, le dock risque de recharger certaines des applets distantes (celles qui seront actives en conf et qui viendront apres DBus).
}

void cd_dbus_stop_service (void)
{
	// on vire tous les modules distants.
	myData.bServiceIsStopping = TRUE;  // on stoppe les applets distantes differemment suivant que c'est l'utilisateur qui la decoche ou pas.
	dbusApplet *pDbusApplet;
	Icon *pIcon;
	CairoContainer *pContainer;
	GList *a;
	for (a = myData.pAppletList; a != NULL; a = a->next)
	{
		pDbusApplet = a->data;
		pIcon = (pDbusApplet->pModuleInstance ? pDbusApplet->pModuleInstance->pIcon : NULL);
		pContainer = (pDbusApplet->pModuleInstance ? pDbusApplet->pModuleInstance->pContainer : NULL);
		
		cairo_dock_unregister_module (pDbusApplet->cModuleName);  // stoppe le module (toutes les instances), ce qui appelle stop_module, qui emet le signal 'stop' vers l'applet distante (qui du coup quitte), et enleve le module de la table. l'objet distant n'est pas detruit, puisque de toute facon on vide toute la liste.
		
		if (pIcon != NULL && pContainer != NULL)  // par contre les icones restent, mais ne sont plus des applets (elles ont perdu leur module). On fait donc le menage.
		{
			if (CAIRO_DOCK_IS_DOCK (pContainer))
			{
				cairo_dock_detach_icon_from_dock (pIcon, CAIRO_DOCK (pContainer), myIcons.iSeparateIcons);
				cairo_dock_free_icon (pIcon);
				cairo_dock_update_dock_size (CAIRO_DOCK (pContainer));
				cairo_dock_redraw_container (pContainer);
			}
		}
		g_object_unref (pDbusApplet);
		if (pDbusApplet->pSubApplet != NULL)
			g_object_unref (pDbusApplet->pSubApplet);
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


void cd_dbus_add_applet_to_startup (const gchar *cModuleName)
{
	if (! cd_dbus_applet_is_used (cModuleName))  // precaution pour eviter de le rajouter 2 fois.
	{
		gchar *str = myData.cActiveModules;
		if (myData.cActiveModules)
			myData.cActiveModules = g_strdup_printf ("%s;%s", myData.cActiveModules, cModuleName);
		else
			myData.cActiveModules = g_strdup (cModuleName);
		g_free (str);
		cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE,
			G_TYPE_STRING, "Configuration", "modules", myData.cActiveModules,
			G_TYPE_INVALID);
	}
}
static void _on_init_module (CairoDockModuleInstance *pModuleInstance, GKeyFile *pKeyFile)
{
	cd_debug ("%s (%d)", __func__, (int)pModuleInstance->pModule->fLastLoadingTime);
	
	//\_____________ On initialise l'icone.
	cd_dbus_action_on_init_module (pModuleInstance);
	
	//\_____________ On se souvient qu'il faut lancer cette applet au demarrage.
	cd_dbus_add_applet_to_startup (pModuleInstance->pModule->pVisitCard->cModuleName);
	
	//\_____________ On cree l'objet sur le bus.
	dbusApplet *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pModuleInstance);
	if (pDbusApplet == NULL)
		pDbusApplet = cd_dbus_create_remote_applet_object (pModuleInstance);
	g_return_if_fail (pDbusApplet != NULL);
	
	//\____________ On met a jour le fichier de conf si necessaire, c'est plus simple pour les applets.
	if (pKeyFile != NULL)
	{
		if (cairo_dock_conf_file_needs_update (pKeyFile, pModuleInstance->pModule->pVisitCard->cModuleVersion))
			cairo_dock_flush_conf_file (pKeyFile, pModuleInstance->cConfFilePath, pModuleInstance->pModule->pVisitCard->cShareDataDir, pModuleInstance->pModule->pVisitCard->cConfFileName);
	}
	
	//\_____________ On (re)lance le script distant.
	cd_dbus_launch_distant_applet_in_dir (pModuleInstance->pModule->pVisitCard->cModuleName, pModuleInstance->pModule->pVisitCard->cShareDataDir);
}
gboolean cd_dbus_register_new_module (const gchar *cModuleName, const gchar *cDescription, const gchar *cAuthor, const gchar *cVersion, gint iCategory, const gchar *cShareDataDir)
{
	if (! myConfig.bEnableNewModule)
		return FALSE;
	cd_debug ("%s (%s)", __func__, cModuleName);
	
	//\____________ on cree et on enregistre un nouveau module s'il n'existe pas deja.
	CairoDockModule *pModule = cairo_dock_find_module_from_name (cModuleName);
	if (pModule != NULL)  // le module existe deja, rien a faire.
	{
		cd_warning ("this module (%s) is already registered", cModuleName);
		if (pModule->cSoFilePath != NULL)
		{
			cd_warning ("an installed module already exists with this name (%s).", cModuleName);
			return FALSE;
		}
	}
	else  // on enregistre ce nouveau module.
	{
		pModule = g_new0 (CairoDockModule, 1);
		CairoDockVisitCard *pVisitCard = g_new0 (CairoDockVisitCard, 1);
		pModule->pVisitCard = pVisitCard;
		pVisitCard->cModuleName = g_strdup (cModuleName);
		pVisitCard->iMajorVersionNeeded = 2;
		pVisitCard->iMinorVersionNeeded = 1;
		pVisitCard->iMicroVersionNeeded = 1;
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
		pVisitCard->cTitle = g_strdup (dgettext (pVisitCard->cGettextDomain, cModuleName));
		//pVisitCard->bMultiInstance = TRUE;
		pModule->pInterface = g_new0 (CairoDockModuleInterface, 1);
		pModule->pInterface->initModule = _on_init_module;
		pModule->pInterface->stopModule = cd_dbus_emit_on_stop_module;
		pModule->pInterface->reloadModule = cd_dbus_emit_on_reload_module;
		if (! cairo_dock_register_module (pModule))
		{
			cairo_dock_free_module (pModule);
			cd_warning ("registration of '%s' has failed.", cModuleName);
			return FALSE;
		}
	}
	
	//\____________ si l'applet n'est pas activee en conf, on en reste la.
	gboolean bAppletIsUsed = cd_dbus_applet_is_used (cModuleName);
	if (! bAppletIsUsed)
	{
		cd_debug ("applet %s has been registered, but is not wanted by the user.", cModuleName);
		return TRUE;
	}
	
	//\____________ sinon on active le module.
	GError *tmp_erreur = NULL;
	cairo_dock_activate_module (pModule, &tmp_erreur);
	if (tmp_erreur != NULL)
	{
		cd_warning (tmp_erreur->message);
		g_error_free (tmp_erreur);
		return FALSE;
	}
	
	CairoDockModuleInstance *pInstance = pModule->pInstancesList->data;
	g_return_val_if_fail (pModule->pInstancesList != NULL, FALSE);
	if (pInstance->pDock)  // la fonction cairo_dock_activate_module() ne fait pas ca.
	{
		cairo_dock_update_dock_size (pInstance->pDock);
		cairo_dock_redraw_container (pInstance->pContainer);
	}
	
	cd_debug ("applet has been successfully instanciated");
	return TRUE;
}

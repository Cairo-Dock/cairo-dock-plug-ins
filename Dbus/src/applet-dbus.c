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

#include <fcntl.h>  // O_RDONLY
#include <unistd.h>  // open, read, close
#define __USE_POSIX
#include <signal.h>  // kill
#include <glib.h>
#include <glib/gstdio.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include "interface-main-methods.h"
#include "dbus-main-spec.h"
#include "interface-applet-object.h"
#include "interface-applet-signals.h"
#include "applet-dbus.h"

#define DISTANT_DIR "3.0"
#define GETTEXT_NAME_EXTRAS "cairo-dock-plugins-extra"
#define LOCALE_DIR_NAME "locale"

  ///////////////////
 /// MAIN OBJECT ///
///////////////////

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
	dbus_g_connection_register_g_object(pMainObject->connection, myData.cBasePath, G_OBJECT(pMainObject));
}


  //////////////////////////
 /// MODULE REGISTERING ///
//////////////////////////

static void _on_init_module (CairoDockModuleInstance *pModuleInstance, GKeyFile *pKeyFile)
{
	cd_debug ("%s ()", __func__);
	
	//\_____________ On initialise l'icone.
	cd_dbus_action_on_init_module (pModuleInstance);
	//\_____________ On cree l'objet sur le bus.
	dbusApplet *pDbusApplet = cd_dbus_create_remote_applet_object (pModuleInstance);
	g_return_if_fail (pDbusApplet != NULL);
	
	//\____________ On met a jour le fichier de conf si necessaire, c'est plus simple pour les applets.
	if (pKeyFile != NULL)
	{
		if (cairo_dock_conf_file_needs_update (pKeyFile, pModuleInstance->pModule->pVisitCard->cModuleVersion))
		{
			///cairo_dock_flush_conf_file (pKeyFile, pModuleInstance->cConfFilePath, pModuleInstance->pModule->pVisitCard->cShareDataDir, pModuleInstance->pModule->pVisitCard->cConfFileName);
			gchar *cTemplate = g_strdup_printf ("%s/%s", pModuleInstance->pModule->pVisitCard->cShareDataDir, pModuleInstance->pModule->pVisitCard->cConfFileName);
			cairo_dock_upgrade_conf_file (pModuleInstance->cConfFilePath, pKeyFile, cTemplate);
			g_free (cTemplate);
		}
	}
	
	//\_____________ On (re)lance l'executable de l'applet.
	cd_dbus_launch_applet_process (pModuleInstance, pDbusApplet);
}
static gboolean _cd_dbus_register_new_module (const gchar *cModuleName, const gchar *cDescription, const gchar *cAuthor, const gchar *cVersion, gint iCategory, const gchar *cIconName, const gchar *cShareDataDir, gboolean bMultiInstance, gboolean bActAsLauncher)
{
	cd_message ("%s (%s)", __func__, cModuleName);
	
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
		pVisitCard->cGettextDomain = g_strdup (GETTEXT_NAME_EXTRAS);
		pVisitCard->cUserDataDir = g_strdup (cModuleName);
		pVisitCard->cShareDataDir = g_strdup (cShareDataDir);
		pVisitCard->cConfFileName = g_strdup_printf ("%s.conf", cModuleName);
		pVisitCard->cModuleVersion = g_strdup (cVersion);
		pVisitCard->cAuthor = g_strdup (cAuthor);
		pVisitCard->iCategory = iCategory;
		if (cIconName != NULL)
			pVisitCard->cIconFilePath = cairo_dock_search_icon_s_path (cIconName);
		if (pVisitCard->cIconFilePath == NULL)
			pVisitCard->cIconFilePath = (cShareDataDir ? g_strdup_printf ("%s/icon", cShareDataDir) : NULL);
		pVisitCard->iSizeOfConfig = 4;  // au cas ou ...
		pVisitCard->iSizeOfData = 4;  // au cas ou ...
		pVisitCard->cDescription = g_strdup (cDescription);
		pVisitCard->cTitle = g_strdup (dgettext (pVisitCard->cGettextDomain, cModuleName));
		pVisitCard->iContainerType = CAIRO_DOCK_MODULE_CAN_DOCK | CAIRO_DOCK_MODULE_CAN_DESKLET;
		pVisitCard->bMultiInstance = bMultiInstance;
		pVisitCard->bActAsLauncher = bActAsLauncher;  // ex.: XChat controls xchat/xchat-gnome, but it does that only after initializing; we need to know if it's a launcher before the taskbar is loaded, hence this parameter.
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
	return TRUE;
}

gboolean cd_dbus_register_module_in_dir (const gchar *cModuleName, const gchar *cThirdPartyPath)
{
	cd_debug ("%s (%s, %s)", __func__, cModuleName, cThirdPartyPath);
	gchar *cFilePath = g_strdup_printf ("%s/%s/auto-load.conf", cThirdPartyPath, cModuleName);
	GKeyFile *pKeyFile = cairo_dock_open_key_file (cFilePath);
	if (pKeyFile == NULL)
	{
		cd_warning ("file %s should not be here", cFilePath);
		g_free (cFilePath);
		return FALSE;
	}
	
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
		iCategory = CAIRO_DOCK_CATEGORY_APPLET_ACCESSORY;
	}
	gchar *cIconName = g_key_file_get_string (pKeyFile, "Register", "icon", NULL);  // NULL if not specified, in which case we use the "icon" file.
	if (cIconName && *cIconName == '\0')
	{
		g_free (cIconName);
		cIconName = NULL;
	}
	
	gboolean bMultiInstance = g_key_file_get_boolean (pKeyFile, "Register", "multi-instance", NULL);  // false if not specified
	
	gboolean bActAsLauncher = g_key_file_get_boolean (pKeyFile, "Register", "act as launcher", NULL);  // false if not specified
	
	gchar *cShareDataDir = g_strdup_printf ("%s/%s", cThirdPartyPath, cModuleName);
	
	g_key_file_free (pKeyFile);
	
	gboolean bActivationOk = _cd_dbus_register_new_module (cModuleName, cDescription, cAuthor, cVersion, iCategory, cIconName, cShareDataDir, bMultiInstance, bActAsLauncher);
	g_free (cDescription);
	g_free (cAuthor);
	g_free (cVersion);
	g_free (cIconName);
	g_free (cShareDataDir);
	g_free (cFilePath);
	return bActivationOk;
}

static gboolean _cd_dbus_register_all_applets_in_dir (const gchar *cDirPath)
{
	gboolean bAppletRegistered = FALSE;
	const gchar *cFileName;
	gchar *cThirdPartyPath = g_strdup_printf ("%s/%s", cDirPath, CD_DBUS_APPLETS_FOLDER);
	
	GDir *dir = g_dir_open (cThirdPartyPath, 0, NULL);  // si le repertoire n'existe pas, on ne veut pas de warning.
	if (dir == NULL)
	{
		g_free (cThirdPartyPath);
		return bAppletRegistered;
	}

	do
	{
		cFileName = g_dir_read_name (dir);
		if (cFileName == NULL)
			break ;
		
		if (strcmp (cFileName, LOCALE_DIR_NAME) != 0)
			bAppletRegistered |= cd_dbus_register_module_in_dir (cFileName, cThirdPartyPath);
	}
	while (1);
	g_dir_close (dir);
	g_free (cThirdPartyPath);
	return bAppletRegistered;
}


  /////////////////////
 /// MODULE UPDATE ///
/////////////////////

static void _get_package_path (gchar *cModuleName)
{
	const gchar *cSharePackagesDir = MY_APPLET_SHARE_DATA_DIR"/"CD_DBUS_APPLETS_FOLDER;
	gchar *cUserPackagesDir = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, CD_DBUS_APPLETS_FOLDER);
	///gchar *cDistantPackagesDir = g_strdup_printf ("%s/%d.%d.%d", CD_DBUS_APPLETS_FOLDER, g_iMajorVersion, g_iMinorVersion, g_iMicroVersion);
	const gchar *cDistantPackagesDir = CD_DBUS_APPLETS_FOLDER"/"DISTANT_DIR;
	gchar *cPath = cairo_dock_get_package_path (cModuleName,
		cSharePackagesDir,
		cUserPackagesDir,
		cDistantPackagesDir,
		CAIRO_DOCK_UPDATED_PACKAGE);
	cd_debug ("*** update of the applet '%s' -> got '%s'\n", cModuleName, cPath);
	g_free (cPath);
	g_free (cUserPackagesDir);
	//g_free (cDistantPackagesDir);
}
static gboolean _apply_package_update (gchar *cModuleName)
{
	CairoDockModule *pModule = cairo_dock_find_module_from_name (cModuleName);
	
	if (pModule && pModule->pInstancesList != NULL)  // applet active => reload it (pModule can be NULL in case of "locale").
	{
		cd_debug ("*** applet '%s' is active, reload it", cModuleName);
		CairoDockModuleInstance *pModuleInstance = pModule->pInstancesList->data;
		Icon *pIcon = pModuleInstance->pIcon;
		CairoContainer *pContainer = pModuleInstance->pContainer;
		
		// unregister the module, since anything can have changed, including its definition.
		cairo_dock_unregister_module (cModuleName);  // stoppe le module (toutes les instances), ce qui appelle stop_module, qui emet le signal 'stop' vers l'applet distante (qui du coup quitte), et enleve le module de la table. L'objet distant est detruit durant le stop.
		
		// clean the dock from the remaining icon.
		if (pIcon != NULL && pContainer != NULL)  // par contre les icones restent, mais ne sont plus des applets (elles ont perdu leur module). On fait donc le menage.
		{
			if (CAIRO_DOCK_IS_DOCK (pContainer))
			{
				cairo_dock_detach_icon_from_dock (pIcon, CAIRO_DOCK (pContainer));
				cairo_dock_free_icon (pIcon);
				cairo_dock_update_dock_size (CAIRO_DOCK (pContainer));
				cairo_dock_redraw_container (pContainer);
			}
		}
		
		// now register again the updated module.
		gchar *cThirdPartyPath = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, CD_DBUS_APPLETS_FOLDER);
		cd_dbus_register_module_in_dir (cModuleName, cThirdPartyPath);
		g_free (cThirdPartyPath);
		
		// and activate it again.
		pModule = cairo_dock_find_module_from_name (cModuleName);  // the module has been destroyed previously, so grab it again.
		g_return_val_if_fail (pModule != NULL, TRUE);
		cairo_dock_activate_module (pModule, NULL);
	}
	
	// get corresponding task and free it.
	CairoDockTask *pUpdateTask = NULL;
	GList *t;
	for (t = myData.pUpdateTasksList; t != NULL; t = t->next)
	{
		pUpdateTask = t->data;
		if (pUpdateTask->pSharedMemory && strcmp (pUpdateTask->pSharedMemory, cModuleName) == 0)
		{
			myData.pUpdateTasksList = g_list_remove_link (myData.pUpdateTasksList, t);  /// g_list_delete_link ?...
			cairo_dock_discard_task (pUpdateTask);
			break;
		}
	}
	return TRUE;
}
static void _check_update_package (const gchar *cModuleName, CairoDockPackage *pPackage, gpointer data)
{
	cd_message ("*** %s (%s, %d)", __func__, cModuleName, pPackage->iType);
	if (pPackage->iType == CAIRO_DOCK_UPDATED_PACKAGE)
	{
		gchar *cUserDirPath = g_strdup_printf ("%s/%s/%s", g_cCairoDockDataDir, CD_DBUS_APPLETS_FOLDER, cModuleName);
		if (g_file_test (cUserDirPath, G_FILE_TEST_EXISTS))
		{
			cd_message ("*** the applet '%s' needs to be updated", cModuleName);
			CairoDockTask *pUpdateTask = cairo_dock_new_task_full (0,
				(CairoDockGetDataAsyncFunc) _get_package_path,
				(CairoDockUpdateSyncFunc) _apply_package_update,
				(GFreeFunc) g_free,
				g_strdup (cModuleName));
			myData.pUpdateTasksList = g_list_prepend (myData.pUpdateTasksList, pUpdateTask);
			cairo_dock_launch_task (pUpdateTask);
		}
	}
}
static void _on_got_list (GHashTable *pPackagesTable, gpointer data)
{
	if (pPackagesTable != NULL)
	{
		g_hash_table_foreach (pPackagesTable, (GHFunc) _check_update_package, NULL);
	}
	cairo_dock_discard_task (myData.pGetListTask);
	myData.pGetListTask = NULL;
}


  ///////////////
 /// SERVICE ///
///////////////

void cd_dbus_clean_up_processes (gboolean bAll)
{
	static gchar cFilePathBuffer[23+1];  // /proc/12345/cmdline + 4octets de marge.
	static gchar cContent[512+1];
	gboolean bIsRunning = FALSE;
	
	GError *erreur = NULL;
	GDir *dir = g_dir_open ("/proc", 0, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Dbus : %s", erreur->message);
		g_error_free (erreur);
		return;
	}
	
	int iPid, iPPid;
	gchar *str, *sp;
	const gchar *cPid;
	while ((cPid = g_dir_read_name (dir)) != NULL)
	{
		// get the command line that spawned the process.
		if (! g_ascii_isdigit (*cPid))
			continue;
		
		snprintf (cFilePathBuffer, 23, "/proc/%s/cmdline", cPid);
		int pipe = open (cFilePathBuffer, O_RDONLY);
		if (pipe <= 0)
			continue ;
		
		int iNbBytesRead;
		if ((iNbBytesRead = read (pipe, cContent, sizeof (cContent))) <= 1)  // ends with a nul char.
		{
			close (pipe);
			continue;
		}
		close (pipe);
		
		// search for the pattern "... progname 123", where spaces are nul char.
		const gchar *str = cContent + iNbBytesRead - 2;  // last non-nul char.
		while (*str != '\0' && str != cContent)  // search the previous nul char.
			str --;
		if (str == cContent)  // not found
			continue;
		iPPid = atoi (str+1);
		if (iPPid == 0)  // not a pid.
			continue;
		
		str --;  // skip the nul char.
		while (*str != '\0' && str != cContent)  // search the previous nul char.
			str --;
		if (str == cContent)  // not found
			continue;
		if (strcmp (str+1, myData.cProgName) != 0)  // not our program name.
			continue;
		
		// found, kill the applet process.
		gchar *cProcFile = g_strdup_printf ("/proc/%d", iPPid);
		if (bAll || ! g_file_test (cProcFile, G_FILE_TEST_EXISTS))  // old process
		{
			cd_message ("this applet (%s %s) is linked to an old gldi process (%d), kill it.", cContent, cPid, iPPid);
			iPid = atoi (cPid);
			kill (iPid, SIGKILL);  // SIGTERM sometimes lets the process alive.
		}
	}
	g_dir_close (dir);
}

void cd_dbus_launch_service (void)
{
	g_return_if_fail (myData.pMainObject == NULL);
	g_type_init();
	cd_message ("dbus : launching service...");
	
	//\____________ define the base path on the bus. So each program built on gldi has its own path on the gldi bus, and will place its applets under its own path.
	const gchar *cProgName = g_get_prgname ();
	g_return_if_fail (cProgName != NULL);
	int n = strlen (cProgName);
	gchar *cName1 = g_new0 (gchar, n+1);  // prog name without '-' and '_'
	gchar *cName2 = g_new0 (gchar, n+1);  // same but with upper char after the '-' and '_'
	int i, k=0;
	for (i = 0; i < n; i ++)
	{
		if (cProgName[i] == '-' || cProgName[i] == '_')
			continue;
		cName1[k] = g_ascii_tolower (cProgName[i]);
		if (i == 0 || cProgName[i-1] == '-' || cProgName[i-1] == '_')
			cName2[k] = g_ascii_toupper (cProgName[i]);
		else
			cName2[k] = cName1[k];
		k ++;
	}
	myData.cProgName = cProgName;
	myData.cBasePath = g_strdup_printf ("/org/%s/%s", cName1, cName2);
	g_free (cName1);
	g_free (cName2);
	
	//\____________ kill all the orphean applets (for instance if the dock has crashed, or if it was interrupted by a CTRL+C, or if it stopped and the applet was busy and didn't receive the stop event (dbus-timeout)).
	cd_dbus_clean_up_processes (FALSE);  // FALSE <=> from old gldi instances
	
	//\____________ Register the service name (the service name is registerd once by the first gldi instance).
	cairo_dock_register_service_name ("org.cairodock.CairoDock");  /// what happens if the gldi instance that had registered the name quits while a 2nd instance remains ? do we need to queue ?...
	
	//\____________ create the main object on the bus.
	myData.pMainObject = g_object_new (cd_dbus_main_get_type(), NULL);  // call cd_dbus_main_class_init() and cd_dbus_main_init().
	
	//\____________ register the applets installed in the default folders.
	gboolean bAppletRegistered = FALSE;
	bAppletRegistered |= _cd_dbus_register_all_applets_in_dir (MY_APPLET_SHARE_DATA_DIR);
	
	bAppletRegistered |= _cd_dbus_register_all_applets_in_dir (g_cCairoDockDataDir);
	
	//\____________ internationalize the applets.
	gchar *cLocaleDir = g_strdup_printf ("%s/"CD_DBUS_APPLETS_FOLDER"/"LOCALE_DIR_NAME, g_cCairoDockDataDir);  // user version of /usr/share/locale
	if (! g_file_test (cLocaleDir, G_FILE_TEST_EXISTS))  // translations not downloaded yet.
	{
		gchar *cUserAppletsFolder = g_strdup_printf ("%s/"CD_DBUS_APPLETS_FOLDER, g_cCairoDockDataDir);
		if (! g_file_test (cUserAppletsFolder, G_FILE_TEST_EXISTS))
		{
			if (g_mkdir (cUserAppletsFolder, 7*8*8+7*8+5) != 0)  // create an empty folder; since there is no date file, the "locale" package will be seen as "to be updated" by the package manager, and will therefore download it.
				cd_warning ("couldn't create '%s'; third-party applets can't be added", cUserAppletsFolder);
		}
		g_free (cUserAppletsFolder);
		
		if (g_mkdir (cLocaleDir, 7*8*8+7*8+5) != 0)  // create an empty folder; since there is no date file, the "locale" package will be seen as "to be updated" by the package manager, and will therefore download it.
			cd_warning ("couldn't create '%s'; applets won't be translated", cLocaleDir);
	}
	bindtextdomain (GETTEXT_NAME_EXTRAS, cLocaleDir);  // bind the applets' domain to the user locale folder.
	bind_textdomain_codeset (GETTEXT_NAME_EXTRAS, "UTF-8");
	g_free (cLocaleDir);
	
	//\____________ download in background the list of existing applets.
	if (bAppletRegistered)  // only if some third-party applets are present on the disk.
	{
		const gchar *cSharePackagesDir = NULL;  // no share data dir, since we can't write in /usr
		gchar *cUserPackagesDir = g_strdup_printf ("%s/%s", g_cCairoDockDataDir, CD_DBUS_APPLETS_FOLDER);
		const gchar *cDistantPackagesDir = CD_DBUS_APPLETS_FOLDER"/"DISTANT_DIR;
		myData.pGetListTask = cairo_dock_list_packages_async (cSharePackagesDir,
			cUserPackagesDir,
			cDistantPackagesDir,
			(CairoDockGetPackagesFunc) _on_got_list,
			NULL,  // data
			NULL);  // table
		g_free (cUserPackagesDir);
	}
}


  ///////////////////
 /// MARSHALLERS ///
///////////////////
// must be in this file, otherwise we get include hell because of the generated code in *-spec.h

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

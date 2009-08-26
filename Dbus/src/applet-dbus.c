/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Necropotame & Fabounet (for any bug report, please mail me to adrien.pilleboue@gmail.com)

exemples : 
----------

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.CreateLauncherFromScratch string:inkscape string:yep string:rien string:none

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.SetLabel string:new_label string:icon_name string:any string:none

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.SetQuickInfo string:123 string:none string:none string:dustbin

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.Animate string:deault int32:2 string:any string:firefox string:none

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.SetIcon string:firefox-3.0 string:any string:nautilus string:none

 
******************************************************************************/
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include "applet-dbus.h"
#include "applet-dbus-spec.h"

#define nullify_argument(string) do {\
	if (string != NULL && (*string == '\0' || strcmp (string, "any") == 0 || strcmp (string, "none") == 0))\
		string = NULL; } while (0)

static gboolean dbus_deskletVisible = FALSE;
static guint dbus_xLastActiveWindow;

G_DEFINE_TYPE(dbusCallback, cd_dbus_callback, G_TYPE_OBJECT);

static void g_cclosure_marshal_VOID__STRING_INT (GClosure *closure,
	GValue *return_value,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint,
	gpointer marshal_data);
static void g_cclosure_marshal_VOID__STRING_BOOLEAN (GClosure *closure,
	GValue *return_value,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint,
	gpointer marshal_data);
static void g_cclosure_marshal_VOID__STRING_STRING (GClosure *closure,
	GValue *return_value,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint,
	gpointer marshal_data);


static void g_cclosure_marshal_VOID__STRING_INT (GClosure *closure,
	GValue *return_value,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint,
	gpointer marshal_data)
{
	g_print ("%s ()\n", __func__);
}

static void g_cclosure_marshal_VOID__STRING_BOOLEAN (GClosure *closure,
	GValue *return_value,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint,
	gpointer marshal_data)
{
	g_print ("%s ()\n", __func__);
}

static void g_cclosure_marshal_VOID__STRING_STRING (GClosure *closure,
	GValue *return_value,
	guint n_param_values,
	const GValue *param_values,
	gpointer invocation_hint,
	gpointer marshal_data)
{
	g_print ("%s ()\n", __func__);
}

static void cd_dbus_callback_class_init(dbusCallbackClass *klass)
{
	cd_message("");
	
	// Enregistrement des marshaller specifique aux signaux.
	dbus_g_object_register_marshaller(g_cclosure_marshal_VOID__STRING_INT,
		G_TYPE_NONE, G_TYPE_STRING, G_TYPE_INT ,G_TYPE_INVALID);
	dbus_g_object_register_marshaller(g_cclosure_marshal_VOID__STRING_BOOLEAN,
		G_TYPE_NONE, G_TYPE_STRING, G_TYPE_BOOLEAN ,G_TYPE_INVALID);
	dbus_g_object_register_marshaller(g_cclosure_marshal_VOID__STRING_STRING,
		G_TYPE_NONE, G_TYPE_STRING, G_TYPE_STRING ,G_TYPE_INVALID);
	
	// on definit les signaux dont on aura besoin.
	myData.iSidOnClickIcon =
		g_signal_new("on_click_icon",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				g_cclosure_marshal_VOID__STRING_INT,
				G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_INT);
	myData.iSidOnMiddleClickIcon =
		g_signal_new("on_middle_click_icon",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				g_cclosure_marshal_VOID__STRING,
				G_TYPE_NONE, 1, G_TYPE_STRING);
	myData.iSidOnScrollIcon =
		g_signal_new("on_scroll_icon",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				g_cclosure_marshal_VOID__STRING_BOOLEAN,
				G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_BOOLEAN);
	myData.iSidOnBuildMenu =
		g_signal_new("on_build_menu",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				g_cclosure_marshal_VOID__STRING,
				G_TYPE_NONE, 1, G_TYPE_STRING);
	myData.iSidOnDropData =
		g_signal_new("on_drop_data",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				g_cclosure_marshal_VOID__STRING_STRING,
				G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);
	myData.iSidOnReloadModule =
		g_signal_new("on_reload_module",
			G_OBJECT_CLASS_TYPE(klass),
				G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
				0,
				NULL, NULL,
				g_cclosure_marshal_VOID__STRING_BOOLEAN,
				G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_BOOLEAN);
}
static void cd_dbus_callback_init (dbusCallback *server)
{
	cd_message("");
	g_return_if_fail (server->connection == NULL);
	
	// Initialise the DBus connection
	server->connection = cairo_dock_get_session_connection ();
	
	dbus_g_object_type_install_info(cd_dbus_callback_get_type(), &dbus_glib_cd_dbus_callback_object_info);
	
	// Register DBUS path
	dbus_g_connection_register_g_object(server->connection, "/org/cairodock/CairoDock", G_OBJECT(server));
	
	// Add signals
	DBusGProxy *pProxy = cairo_dock_get_main_proxy ();
	server->proxy = pProxy;
    if (pProxy != NULL)
    {
		dbus_g_proxy_add_signal(pProxy, "on_click_icon",
			G_TYPE_STRING, G_TYPE_INT, G_TYPE_INVALID);
		dbus_g_proxy_add_signal(pProxy, "on_middle_click_icon",
			G_TYPE_STRING, G_TYPE_INVALID);
		dbus_g_proxy_add_signal(pProxy, "on_scroll_icon",
			G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_INVALID);
		dbus_g_proxy_add_signal(pProxy, "on_build_menu",
			G_TYPE_STRING, G_TYPE_INVALID);
		dbus_g_proxy_add_signal(pProxy, "on_drop_data",
			G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INVALID);
	}
	// Register the service name
	cairo_dock_register_service_name ("org.cairodock.CairoDock");
}
void cd_dbus_launch_service (void)
{
	g_return_if_fail (myData.server == NULL);
	g_type_init();
	
	cd_message("dbus : Lancement du service");
	myData.server = g_object_new(cd_dbus_callback_get_type(), NULL);  // appelle cd_dbus_callback_class_init() et cd_dbus_callback_init().
	
	// on s'abonne aux notifications qu'on voudra propager sur le bus.
	if (myData.server->proxy != NULL)
	{
		cairo_dock_register_notification (CAIRO_DOCK_CLICK_ICON,
			(CairoDockNotificationFunc) cd_dbus_emit_on_click_icon,
			CAIRO_DOCK_RUN_AFTER,
			myApplet);
		cairo_dock_register_notification (CAIRO_DOCK_MIDDLE_CLICK_ICON,
			(CairoDockNotificationFunc) cd_dbus_emit_on_middle_click_icon,
			CAIRO_DOCK_RUN_AFTER,
			myApplet);
		cairo_dock_register_notification (CAIRO_DOCK_SCROLL_ICON,
			(CairoDockNotificationFunc) cd_dbus_emit_on_scroll_icon,
			CAIRO_DOCK_RUN_AFTER,
			myApplet);
		cairo_dock_register_notification (CAIRO_DOCK_BUILD_MENU,
			(CairoDockNotificationFunc) cd_dbus_emit_on_build_menu,
			CAIRO_DOCK_RUN_AFTER,
			myApplet);
		cairo_dock_register_notification (CAIRO_DOCK_DROP_DATA,
			(CairoDockNotificationFunc) cd_dbus_emit_on_drop_data,
			CAIRO_DOCK_RUN_AFTER,
			myApplet);
	}
}

void cd_dbus_stop_service (void)
{
	// on se desabonne.
	cairo_dock_remove_notification_func (CAIRO_DOCK_CLICK_ICON,
		(CairoDockNotificationFunc) cd_dbus_emit_on_click_icon,
		myApplet);
	cairo_dock_remove_notification_func (CAIRO_DOCK_MIDDLE_CLICK_ICON,
		(CairoDockNotificationFunc) cd_dbus_emit_on_middle_click_icon,
		myApplet);
	cairo_dock_remove_notification_func (CAIRO_DOCK_SCROLL_ICON,
		(CairoDockNotificationFunc) cd_dbus_emit_on_scroll_icon,
		myApplet);
	cairo_dock_remove_notification_func (CAIRO_DOCK_BUILD_MENU,
		(CairoDockNotificationFunc) cd_dbus_emit_on_build_menu,
		myApplet);
	cairo_dock_remove_notification_func (CAIRO_DOCK_DROP_DATA,
		(CairoDockNotificationFunc) cd_dbus_emit_on_drop_data,
		myApplet);
	
	/// virer tous les modules manuels ...
	
	if (myData.server != NULL)
		g_object_unref (myData.server);
	myData.server = NULL;
}



gboolean cd_dbus_callback_reboot(dbusCallback *pDbusCallback, GError **error)
{
	if (! myConfig.bEnableReboot)
		return FALSE;
	cairo_dock_read_conf_file (g_cConfFile, g_pMainDock);
	return TRUE;
}

gboolean cd_dbus_callback_quit (dbusCallback *pDbusCallback, GError **error)
{
	if (! myConfig.bEnableQuit)
		return FALSE;
	gtk_main_quit ();
	return TRUE;
}

gboolean cd_dbus_callback_reload_module (dbusCallback *pDbusCallback, const gchar *cModuleName, GError **error)
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

gboolean cd_dbus_callback_show_desklet(dbusCallback *pDbusCallback, gboolean *widgetLayer, GError **error)
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
gboolean cd_dbus_callback_show_dock (dbusCallback *pDbusCallback, gboolean bShow, GError **error)
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

gboolean cd_dbus_callback_load_launcher_from_file (dbusCallback *pDbusCallback, const gchar *cDesktopFile, GError **error)
{
	if (! myConfig.bEnableLoadLauncher)
		return FALSE;
	g_return_val_if_fail (cDesktopFile != NULL, FALSE);
	
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (g_pMainDock));
	Icon *pIcon = cairo_dock_create_icon_from_desktop_file (cDesktopFile, pCairoContext);
	cairo_destroy (pCairoContext);
	
	CairoDock * pParentDock = cairo_dock_search_dock_from_name (pIcon->cParentDockName);
	if (pParentDock != NULL)  // a priori toujours vrai.
	{
		cairo_dock_insert_icon_in_dock (pIcon, pParentDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON);
		cairo_dock_start_icon_animation (pIcon, pParentDock);
	}
	
	return TRUE;
}

gboolean cd_dbus_callback_create_launcher_from_scratch (dbusCallback *pDbusCallback, const gchar *cIconFile, const gchar *cLabel, const gchar *cCommand, const gchar *cParentDockName, GError **error)
{
	if (! myConfig.bEnableCreateLauncher)
		return FALSE;
	
	nullify_argument (cParentDockName);
	if (cParentDockName == NULL)
		cParentDockName = CAIRO_DOCK_MAIN_DOCK_NAME;
	
	CairoDock *pParentDock = cairo_dock_search_dock_from_name (cParentDockName);
	if (pParentDock == NULL)
		return FALSE;
	
	Icon *pIcon = g_new0 (Icon, 1);
	pIcon->iType = CAIRO_DOCK_LAUNCHER;
	pIcon->acFileName = g_strdup (cIconFile);
	pIcon->acName = g_strdup (cLabel);
	pIcon->acCommand = g_strdup (cCommand);
	pIcon->cParentDockName = g_strdup (cParentDockName);
	pIcon->acDesktopFileName = g_strdup ("none");
	pIcon->fOrder = CAIRO_DOCK_LAST_ORDER;
	
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pParentDock));
	cairo_dock_fill_icon_buffers_for_dock (pIcon, pCairoContext, pParentDock);
	cairo_destroy (pCairoContext);
	
	cairo_dock_insert_icon_in_dock (pIcon, pParentDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON);
	
	return TRUE;
}



static void _find_icon_in_dock (Icon *pIcon, CairoDock *pDock, gpointer *data)
{
	gchar *cIconName = data[0];
	gchar *cIconCommand = data[1];
	Icon **pFoundIcon = data[2];
	gchar *cName = (pIcon->cInitialName != NULL ? pIcon->cInitialName : pIcon->acName);
	//g_print ("%s (%s/%s, %s/%s)\n", __func__, cName, cIconName, pIcon->acCommand, cIconCommand);
	if ((cIconName == NULL || (cName && g_ascii_strcasecmp (cIconName, cName) == 0)) &&
		(cIconCommand == NULL || (pIcon->acCommand && g_ascii_strcasecmp (cIconCommand, pIcon->acCommand) == 0)))
	{
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
	else  // on cherche une icone de lanceur.
	{
		gpointer data[3];
		data[0] = (gpointer) cIconName;
		data[1] = (gpointer) cIconCommand;
		data[2] = &pIcon;
		cairo_dock_foreach_icons_in_docks ((CairoDockForeachIconFunc) _find_icon_in_dock, data);
	}
	return pIcon;
}
gboolean cd_dbus_callback_set_quick_info (dbusCallback *pDbusCallback, const gchar *cQuickInfo, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error)
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
	if (pContainer == NULL)
		return FALSE;
	double fMaxScale = cairo_dock_get_max_scale (pContainer);
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pContainer);
	cairo_dock_set_quick_info (pCairoContext, cQuickInfo, pIcon, fMaxScale);
	cairo_destroy (pCairoContext);
	cairo_dock_redraw_icon (pIcon, pContainer);
	return TRUE;
}

gboolean cd_dbus_callback_set_label (dbusCallback *pDbusCallback, const gchar *cLabel, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error)
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
	if (pContainer == NULL)
		return FALSE;
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pContainer);
	cairo_dock_set_icon_name (pCairoContext, cLabel, pIcon, pContainer);
	cairo_destroy (pCairoContext);
	return TRUE;
}

gboolean cd_dbus_callback_set_icon (dbusCallback *pDbusCallback, const gchar *cImage, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error)
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
	if (pContainer == NULL)
		return FALSE;
	
	g_return_val_if_fail (pIcon->pIconBuffer != NULL, FALSE);
	cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
	cairo_dock_set_image_on_icon (pIconContext, cImage, pIcon, pContainer);
	cairo_destroy (pIconContext);
	cairo_dock_redraw_icon (pIcon, pContainer);
	return TRUE;
}

gboolean cd_dbus_callback_animate (dbusCallback *pDbusCallback, const gchar *cAnimation, gint iNbRounds, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error)
{
	if (! myConfig.bEnableAnimateIcon)
		return FALSE;
	
	nullify_argument (cIconName);
	nullify_argument (cIconCommand);
	nullify_argument (cModuleName);
	
	Icon *pIcon = cd_dbus_find_icon (cIconName, cIconCommand, cModuleName);
	if (pIcon == NULL)
		return FALSE;
	
	CairoContainer *pContainer = cairo_dock_search_container_from_icon (pIcon);
	if (pContainer == NULL)
		return FALSE;
	
	if (CAIRO_DOCK_IS_DOCK (pContainer) && cAnimation != NULL)
	{
		cairo_dock_request_icon_animation (pIcon, CAIRO_DOCK (pContainer), cAnimation, iNbRounds);
		return TRUE;
	}
	return FALSE;
}

gboolean cd_dbus_callback_show_dialog (dbusCallback *pDbusCallback, const gchar *message, gint iDuration, const gchar *cIconName, const gchar *cIconCommand, const gchar *cModuleName, GError **error)
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


static gboolean cd_dbus_emit_on_reload_module (CairoDockModuleInstance *pModuleInstance, CairoContainer *pOldContainer, GKeyFile *pKeyFile)
{
	g_print ("%s ()\n", __func__);
	g_signal_emit (myData.server, myData.iSidOnReloadModule, 0, pModuleInstance->pModule->pVisitCard->cModuleName, pKeyFile != NULL);
}

gboolean cd_dbus_callback_register_new_module (dbusCallback *pDbusCallback, const gchar *cModuleName, gint iCategory, const gchar *cDescription, const gchar *cShareDataDir, GError **error)
{
	if (! myConfig.bEnableNewModule)
		return FALSE;
	
	CairoDockModule *pModule = g_new0 (CairoDockModule, 1);
	CairoDockVisitCard *pVisitCard = g_new0 (CairoDockVisitCard, 1);
	pModule->pVisitCard = pVisitCard;
	pVisitCard->cModuleName = g_strdup (cModuleName);
	pVisitCard->iMajorVersionNeeded = 2;
	pVisitCard->iMinorVersionNeeded = 1;
	pVisitCard->iMicroVersionNeeded = 0;
	pVisitCard->cPreviewFilePath = cShareDataDir ? g_strdup_printf ("%s/preview", cShareDataDir) : NULL;
	pVisitCard->cGettextDomain = g_strdup_printf ("cd-%s", cModuleName);
	pVisitCard->cUserDataDir = g_strdup ("cModuleName");
	pVisitCard->cShareDataDir = g_strdup (cShareDataDir);
	pVisitCard->cConfFileName = g_strdup_printf ("%s.conf", cModuleName);
	pVisitCard->cModuleVersion = g_strdup ("0.0.1");
	pVisitCard->iCategory = iCategory;
	pVisitCard->cIconFilePath = cShareDataDir ? g_strdup_printf ("%s/%s", cShareDataDir, "icon.png") : NULL;
	pVisitCard->iSizeOfConfig = 4;  // au cas ou ...
	pVisitCard->iSizeOfData = 4;  // au cas ou ...
	pVisitCard->cDescription = g_strdup (cDescription);
	pModule->pInterface = g_new0 (CairoDockModuleInterface, 1);
	//pModule->pInterface->initModule = _init;
	//pModule->pInterface->stopModule = _stop;
	pModule->pInterface->reloadModule = cd_dbus_emit_on_reload_module;
	cairo_dock_load_manual_module (pModule);
	
	GError *tmp_erreur = NULL;
	cairo_dock_activate_module (pModule, &tmp_erreur);  // cairo_dock_activate_module_and_load le rajoute en conf.
	if (tmp_erreur != NULL)
	{
		g_propagate_error (error, tmp_erreur);
		cairo_dock_free_module (pModule);
		return FALSE;
	}
	
	if (pModule->pInstancesList != NULL)
	{
		CairoDockModuleInstance *pInstance = pModule->pInstancesList->data;
		if (pInstance->pDock)
		{
			cairo_dock_update_dock_size (pInstance->pDock);
			gtk_widget_queue_draw (pInstance->pDock->pWidget);
		}
	}
	
	pModule->fLastLoadingTime = time (NULL) + 1e6;  // pour ne pas qu'il soit desactive lors d'un reload general, car il n'est pas dans la liste des modules actifs du fichier de conf.
	return TRUE;
}


#define CAIRO_DOCK_IS_MANUAL_APPLET(pIcon) (CAIRO_DOCK_IS_APPLET (pIcon) && pIcon->pModuleInstance->pModule->cSoFilePath == NULL)


static void _exec_menu_entry (GtkMenuShell *menu, gpointer data)
{
	int iNumEntry = GPOINTER_TO_INT (data);
	g_print ("%s (%s, %d)\n", __func__, myData.cCurrentMenuModule, iNumEntry);
	
}
gboolean cd_dbus_callback_populate_menu (dbusCallback *pDbusCallback, const gchar *cModuleName, const gchar **pLabels, GError **error)
{
	if (! myConfig.bEnableNewModule)
		return FALSE;
	
	if (myData.pModuleSubMenu == NULL)
		return FALSE;
	
	int i;
	for (i = 0; pLabels[i] != NULL; i ++)
	{
		cairo_dock_add_in_menu_with_stock_and_data (pLabels[i],
			NULL,
			_exec_menu_entry,
			myData.pModuleSubMenu,
			GINT_TO_POINTER (i));
	}
	g_free (myData.cCurrentMenuModule);
	myData.cCurrentMenuModule = g_strdup (cModuleName);
	
	return TRUE;
}

gboolean cd_dbus_emit_on_click_icon (CairoDockModuleInstance *myApplet, Icon *pClickedIcon, CairoContainer *pClickedContainer, guint iButtonState)
{
	if (! CAIRO_DOCK_IS_MANUAL_APPLET (pClickedIcon))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	g_print ("%s (%s, %d)\n", __func__, pClickedIcon->pModuleInstance->pModule->pVisitCard->cModuleName, iButtonState);
	g_signal_emit (myData.server, myData.iSidOnClickIcon, 0, pClickedIcon->pModuleInstance->pModule->pVisitCard->cModuleName, iButtonState);
	return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
}


gboolean cd_dbus_emit_on_middle_click_icon (CairoDockModuleInstance *myApplet, Icon *pClickedIcon, CairoContainer *pClickedContainer)
{
	if (! CAIRO_DOCK_IS_MANUAL_APPLET (pClickedIcon))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	g_print ("%s (%s)\n", __func__, pClickedIcon->pModuleInstance->pModule->pVisitCard->cModuleName);
	g_signal_emit (myData.server, myData.iSidOnMiddleClickIcon, 0, pClickedIcon->pModuleInstance->pModule->pVisitCard->cModuleName);
	return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
}

gboolean cd_dbus_emit_on_scroll_icon (CairoDockModuleInstance *myApplet, Icon *pClickedIcon, CairoContainer *pClickedContainer, int iDirection)
{
	if (! CAIRO_DOCK_IS_MANUAL_APPLET (pClickedIcon))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	g_signal_emit (myData.server, myData.iSidOnScrollIcon, 0, pClickedIcon->pModuleInstance->pModule->pVisitCard->cModuleName, CD_APPLET_SCROLL_UP);
	return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
}

static void _delete_menu (GtkMenuShell *menu, CairoDockModuleInstance *myApplet)
{
	myData.pModuleSubMenu = NULL;
}
gboolean cd_dbus_emit_on_build_menu (CairoDockModuleInstance *myApplet, Icon *pClickedIcon, CairoContainer *pClickedContainer, GtkWidget *pAppletMenu)
{
	if (! CAIRO_DOCK_IS_MANUAL_APPLET (pClickedIcon))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	myData.pModuleSubMenu = cairo_dock_create_sub_menu (pClickedIcon->pModuleInstance->pModule->pVisitCard->cModuleName,
		pAppletMenu,
		pClickedIcon->pModuleInstance->pModule->pVisitCard->cIconFilePath);
	
	cairo_dock_add_in_menu_with_stock_and_data (_("About this applet"),
		GTK_STOCK_ABOUT,
		cairo_dock_pop_up_about_applet,
		pAppletMenu,
		pClickedIcon->pModuleInstance);
	
	g_signal_connect (G_OBJECT (pAppletMenu),
		"deactivate",
		G_CALLBACK (_delete_menu),
		myApplet);
	
	g_print ("%s (%s)\n", __func__, pClickedIcon->pModuleInstance->pModule->pVisitCard->cModuleName);
	g_signal_emit (myData.server, myData.iSidOnBuildMenu, 0, pClickedIcon->pModuleInstance->pModule->pVisitCard->cModuleName);
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

gboolean cd_dbus_emit_on_drop_data (CairoDockModuleInstance *myApplet, const gchar *cReceivedData, Icon *pClickedIcon, double fPosition, CairoContainer *pClickedContainer)
{
	if (! CAIRO_DOCK_IS_MANUAL_APPLET (pClickedIcon))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	cd_message (" %s --> sur le bus !", cReceivedData);
	g_signal_emit (myData.server, myData.iSidOnDropData, 0, pClickedIcon->pModuleInstance->pModule->pVisitCard->cModuleName, cReceivedData);
	return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
}

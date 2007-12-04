
#include "stdlib.h"

#include "logout-config.h"
#include "logout-menu-functions.h"
#include "logout-init.h"


#define MY_APPLET_CONF_FILE "logout.conf"
#define MY_APPLET_USER_DATA_DIR "logout"


Icon *my_logout_pIcon = NULL;
CairoDock *my_logout_pDock = NULL;

CairoDockDesktopEnv my_logout_iDesktopEnv;


CairoDockVisitCard *pre_init (void)
{
	CairoDockVisitCard *pVisitCard = g_new0 (CairoDockVisitCard, 1);
	pVisitCard->cModuleName = g_strdup ("logout");
	pVisitCard->cReadmeFilePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, MY_APPLET_README_FILE);
	pVisitCard->iMajorVersionNeeded = 1;
	pVisitCard->iMinorVersionNeeded = 4;
	pVisitCard->iMicroVersionNeeded = 5;
	return pVisitCard;
}


Icon *init (CairoDock *pDock, gchar **cConfFilePath, GError **erreur)
{
	//g_print ("%s ()\n", __func__);
	//\_______________ On verifie la presence des fichiers necessaires.
	*cConfFilePath = cairo_dock_check_conf_file_exists (MY_APPLET_USER_DATA_DIR, MY_APPLET_SHARE_DATA_DIR, MY_APPLET_CONF_FILE);
	
	my_logout_iDesktopEnv = cairo_dock_guess_environment ();
	if (my_logout_iDesktopEnv == CAIRO_DOCK_UNKNOWN_ENV)
	{
		 g_set_error (erreur, 1, 1, "couldn't guess desktop environment, this module will not be active");
		return NULL;
	}
	
	//\_______________ On lit le fichier de conf.
	int iOriginalWidth = 1, iOriginalHeight = 1;
	gchar *cName = NULL, *cIconName = NULL;
	cd_logout_read_conf_file (*cConfFilePath, &iOriginalWidth, &iOriginalHeight, &cName, &cIconName);
	
	//\_______________ On cree notre icone.
	my_logout_pIcon = cairo_dock_create_icon_for_applet (pDock, iOriginalWidth, iOriginalHeight, cName, cIconName);
	my_logout_pDock = pDock;
	
	//\_______________ On enregistre nos notifications.
	cairo_dock_register_notification (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) cd_logout_notification_click_icon, CAIRO_DOCK_RUN_FIRST);
	cairo_dock_register_notification (CAIRO_DOCK_BUILD_MENU, (CairoDockNotificationFunc) cd_logout_notification_build_menu, CAIRO_DOCK_RUN_FIRST);

	
	g_free (cName);
	g_free (cIconName);
	return my_logout_pIcon;
}

void stop (void)
{
	cairo_dock_remove_notification_func (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) cd_logout_notification_click_icon);
	cairo_dock_remove_notification_func (CAIRO_DOCK_BUILD_MENU, (CairoDockNotificationFunc) cd_logout_notification_build_menu);
	
	my_logout_pIcon = NULL;
	my_logout_pDock = NULL;
}

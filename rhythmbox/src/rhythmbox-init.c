#include <stdlib.h>

#include "cairo-dock-dialogs.h"
#include "rhythmbox-draw.h"
#include "rhythmbox-config.h"
#include "rhythmbox-dbus.h"
#include "rhythmbox-menu-functions.h"
#include "rhythmbox-init.h"

#define MY_APPLET_CONF_FILE "rhythmbox.conf"
#define MY_APPLET_USER_DATA_DIR "rhythmbox"


Icon *myIcon = NULL;
CairoDock *myDock = NULL;
cairo_t *myDrawContext = NULL;

cairo_surface_t *rhythmbox_pSurface = NULL;
cairo_surface_t *rhythmbox_pPlaySurface = NULL;
cairo_surface_t *rhythmbox_pPauseSurface = NULL;
cairo_surface_t *rhythmbox_pStopSurface = NULL;
cairo_surface_t *rhythmbox_pBrokenSurface = NULL;

extern gchar *conf_defaultTitle;

static gboolean rhythmbox_dbus_enable = FALSE;


//*********************************************************************************
// rhythmbox_pre_init() : Fonction de pré-initialisation
//*********************************************************************************
CairoDockVisitCard *pre_init (void)
{
	CairoDockVisitCard *pVisitCard = g_new0 (CairoDockVisitCard, 1);
	pVisitCard->cModuleName = g_strdup ("rhythmbox");
	pVisitCard->cReadmeFilePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, MY_APPLET_README_FILE);
	pVisitCard->iMajorVersionNeeded = 1;
	pVisitCard->iMinorVersionNeeded = 4;
	pVisitCard->iMicroVersionNeeded = 5;
	return pVisitCard;
}

//*********************************************************************************
// rhythmbox_init() : Fonction d'initialisation
//*********************************************************************************
Icon *init (CairoDock *pDock, gchar **cConfFilePath, GError **erreur)
{
	g_print ("%s ()\n", __func__);
	myDock = pDock;
	
	//\_______________ On verifie que nos fichiers existent.
	*cConfFilePath = cairo_dock_check_conf_file_exists (MY_APPLET_USER_DATA_DIR, MY_APPLET_SHARE_DATA_DIR, MY_APPLET_CONF_FILE);
	
	//Lecture du fichier de configuration
	int iOriginalWidth = 48, iOriginalHeight = 48;
	gchar *cAppletName = NULL;
	rhythmbox_read_conf_file (*cConfFilePath, &iOriginalWidth, &iOriginalHeight, &cAppletName);
	
	//Création de l'icone
	myIcon = cairo_dock_create_icon_for_applet (pDock, iOriginalWidth, iOriginalHeight, cAppletName, NULL);
	myDrawContext = cairo_create (myIcon->pIconBuffer);
	
	conf_defaultTitle = g_strdup (cAppletName);
	
	GString *sImagePath = g_string_new ("");
	//Chargement de l'image "default"
	g_string_printf (sImagePath, "%s/default.svg", MY_APPLET_SHARE_DATA_DIR);
	rhythmbox_pSurface = cairo_dock_load_image_for_icon (myDrawContext,
		sImagePath->str,
		myIcon->fWidth * (1 + g_fAmplitude),
		myIcon->fHeight * (1 + g_fAmplitude));
	
	//Chargement de l'image "pause"
	g_string_printf (sImagePath, "%s/pause.svg", MY_APPLET_SHARE_DATA_DIR);
	rhythmbox_pPauseSurface = cairo_dock_load_image_for_icon (myDrawContext,
		sImagePath->str,
		myIcon->fWidth * (1 + g_fAmplitude),
		myIcon->fHeight * (1 + g_fAmplitude));
	
	//Chargement de l'image "play"
	g_string_printf (sImagePath, "%s/pause.svg", MY_APPLET_SHARE_DATA_DIR);
	rhythmbox_pPlaySurface = cairo_dock_load_image_for_icon (myDrawContext,
		sImagePath->str,
		myIcon->fWidth * (1 + g_fAmplitude),
		myIcon->fHeight * (1 + g_fAmplitude));
	
	//Chargement de l'image "stop"
	g_string_printf (sImagePath, "%s/pause.svg", MY_APPLET_SHARE_DATA_DIR);
	rhythmbox_pStopSurface = cairo_dock_load_image_for_icon (myDrawContext,
		sImagePath->str,
		myIcon->fWidth * (1 + g_fAmplitude),
		myIcon->fHeight * (1 + g_fAmplitude));
	
	//Chargement de l'image "play"
	g_string_printf (sImagePath, "%s/pause.svg", MY_APPLET_SHARE_DATA_DIR);
	rhythmbox_pBrokenSurface = cairo_dock_load_image_for_icon (myDrawContext,
		sImagePath->str,
		myIcon->fWidth * (1 + g_fAmplitude),
		myIcon->fHeight * (1 + g_fAmplitude));
	
	g_string_free (sImagePath, TRUE);
	
	if (rhythmbox_dbus_enable)
		cairo_dock_set_icon_surface_with_reflect (myDrawContext, rhythmbox_pSurface, myIcon, myDock);
	else
		cairo_dock_set_icon_surface_with_reflect (myDrawContext, rhythmbox_pBrokenSurface, myIcon, myDock);
	
	//Enregistrement des notifications	
	cairo_dock_register_notification (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) rhythmbox_action, CAIRO_DOCK_RUN_FIRST);
	cairo_dock_register_notification (CAIRO_DOCK_BUILD_MENU, (CairoDockNotificationFunc) rhythmbox_notification_build_menu, CAIRO_DOCK_RUN_FIRST);
	
	g_free (cAppletName);
	
	return myIcon;
}


//*********************************************************************************
// rhythmbox_stop() :  Fonction appelé à la fermeture de l'applet
//*********************************************************************************
void stop (void)
{
	g_print ("%s ()\n", __func__);
	myIcon = NULL;
	myDock = NULL;
	
	cairo_dock_remove_notification_func (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) rhythmbox_action);
	cairo_dock_remove_notification_func (CAIRO_DOCK_BUILD_MENU, (CairoDockNotificationFunc) rhythmbox_notification_build_menu);
	
	g_free (conf_defaultTitle);
	conf_defaultTitle = NULL;
	
	cairo_surface_destroy (rhythmbox_pSurface);
	rhythmbox_pSurface = NULL;
	cairo_surface_destroy (rhythmbox_pPlaySurface);
	rhythmbox_pPlaySurface = NULL;
	cairo_surface_destroy (rhythmbox_pPauseSurface);
	rhythmbox_pPauseSurface = NULL;
	cairo_surface_destroy (rhythmbox_pStopSurface);
	rhythmbox_pStopSurface = NULL;
	cairo_surface_destroy (rhythmbox_pBrokenSurface);
	rhythmbox_pBrokenSurface = NULL;
}

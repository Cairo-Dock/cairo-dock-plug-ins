#include <stdlib.h>

#include "cairo-dock-dialogs.h"
#include "rhythmbox-draw.h"
#include "rhythmbox-config.h"
#include "rhythmbox-dbus.h"
#include "rhythmbox-menu-functions.h"
#include "rhythmbox-init.h"

#define RHYTHMBOX_CONF_FILE "rhythmbox.conf"
#define RHYTHMBOX_USER_DATA_DIR "rhythmbox"


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
gchar *pre_init (void)
{
	//g_print ("%s ()\n", __func__);
	rhythmbox_dbus_enable = rhythmbox_dbus_init();
	return g_strdup_printf ("%s/%s", RHYTHMBOX_SHARE_DATA_DIR, RHYTHMBOX_README_FILE);
}

//*********************************************************************************
// rhythmbox_init() : Fonction d'initialisation
//*********************************************************************************
Icon *init (CairoDock *pDock, gchar **cConfFilePath, GError **erreur)
{
	g_print ("%s ()\n", __func__);
	myDock = pDock;
	
	//\_______________ On verifie que nos fichiers existent.
	*cConfFilePath = cairo_dock_check_conf_file_exists (RHYTHMBOX_USER_DATA_DIR, RHYTHMBOX_SHARE_DATA_DIR, RHYTHMBOX_CONF_FILE);
	
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
	g_string_printf (sImagePath, "%s/default.svg", RHYTHMBOX_SHARE_DATA_DIR);
	rhythmbox_pSurface = cairo_dock_load_image_for_icon (myDrawContext,
		sImagePath->str,
		myIcon->fWidth * (1 + g_fAmplitude),
		myIcon->fHeight * (1 + g_fAmplitude));
	
	//Chargement de l'image "pause"
	g_string_printf (sImagePath, "%s/pause.svg", RHYTHMBOX_SHARE_DATA_DIR);
	rhythmbox_pPauseSurface = cairo_dock_load_image_for_icon (myDrawContext,
		sImagePath->str,
		myIcon->fWidth * (1 + g_fAmplitude),
		myIcon->fHeight * (1 + g_fAmplitude));
	
	//Chargement de l'image "play"
	g_string_printf (sImagePath, "%s/pause.svg", RHYTHMBOX_SHARE_DATA_DIR);
	rhythmbox_pPlaySurface = cairo_dock_load_image_for_icon (myDrawContext,
		sImagePath->str,
		myIcon->fWidth * (1 + g_fAmplitude),
		myIcon->fHeight * (1 + g_fAmplitude));
	
	//Chargement de l'image "stop"
	g_string_printf (sImagePath, "%s/pause.svg", RHYTHMBOX_SHARE_DATA_DIR);
	rhythmbox_pStopSurface = cairo_dock_load_image_for_icon (myDrawContext,
		sImagePath->str,
		myIcon->fWidth * (1 + g_fAmplitude),
		myIcon->fHeight * (1 + g_fAmplitude));
	
	//Chargement de l'image "play"
	g_string_printf (sImagePath, "%s/pause.svg", RHYTHMBOX_SHARE_DATA_DIR);
	rhythmbox_pBrokenSurface = cairo_dock_load_image_for_icon (myDrawContext,
		sImagePath->str,
		myIcon->fWidth * (1 + g_fAmplitude),
		myIcon->fHeight * (1 + g_fAmplitude));
	
	g_string_free (sImagePath, TRUE);
	
	if (rhythmbox_dbus_enable)
		cairo_dock_set_icon_surface (myDrawContext, rhythmbox_pSurface);
	else
		cairo_dock_set_icon_surface (myDrawContext, rhythmbox_pBrokenSurface);
	
	myIcon->pReflectionBuffer = cairo_dock_create_reflection_surface (myIcon->pIconBuffer,
		myDrawContext,
		(myDock->bHorizontalDock ? myIcon->fWidth : myIcon->fHeight) * (1 + g_fAmplitude),
		(myDock->bHorizontalDock ? myIcon->fHeight : myIcon->fWidth) * (1 + g_fAmplitude),
		myDock->bHorizontalDock);
	myIcon->pFullIconBuffer = cairo_dock_create_icon_surface_with_reflection (myIcon->pIconBuffer,
		myIcon->pReflectionBuffer,
		myDrawContext,
		(myDock->bHorizontalDock ? myIcon->fWidth : myIcon->fHeight) * (1 + g_fAmplitude),
		(myDock->bHorizontalDock ? myIcon->fHeight : myIcon->fWidth) * (1 + g_fAmplitude),
		myDock->bHorizontalDock);
	
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

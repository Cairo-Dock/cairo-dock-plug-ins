#include <stdlib.h>

#include "cairo-dock-dialogs.h"
#include "rhythmbox-draw.h"
#include "rhythmbox-config.h"
#include "rhythmbox-dbus.h"
#include "rhythmbox-menu-functions.h"
#include "rhythmbox-init.h"

#define RHYTHMBOX_CONF_FILE "rhythmbox.conf"
#define RHYTHMBOX_USER_DATA_DIR "rhythmbox"


Icon *rhythmbox_pIcon = NULL;
CairoDock *rhythmbox_pDock = NULL;
cairo_t *rhythmbox_pCairoContext = NULL;
cairo_surface_t *rhythmbox_pSurface = NULL;
cairo_surface_t *rhythmbox_pPlaySurface = NULL;
cairo_surface_t *rhythmbox_pPauseSurface = NULL;
cairo_surface_t *rhythmbox_pStopSurface = NULL;
cairo_surface_t *rhythmbox_pBrokenSurface = NULL;
gboolean rhythmbox_opening = FALSE;
gboolean rhythmbox_playing = FALSE;
gboolean rhythmbox_dbus_enable = FALSE;
double fImageWidth, fImageHeight;

extern gchar *conf_defaultTitle;
extern gboolean conf_enableDialogs;
extern double conf_timeDialogs;


//*********************************************************************************
// rhythmbox_pre_init() : Fonction de pré-initialisation
//*********************************************************************************
gchar *cd_rhythmbox_pre_init (void)
{
	g_print ("%s ()\n", __func__);
	rhythmbox_dbus_enable = rhythmbox_dbus_init();
	return g_strdup_printf ("%s/%s", RHYTHMBOX_SHARE_DATA_DIR, RHYTHMBOX_README_FILE);
}

//*********************************************************************************
// rhythmbox_init() : Fonction d'initialisation
//*********************************************************************************
Icon *cd_rhythmbox_init (CairoDock *pDock, gchar **cConfFilePath, GError **erreur)
{
	g_print ("%s ()\n", __func__);
	

	rhythmbox_pDock = pDock;
	
	//\_______________ On verifie que nos fichiers existent.
	*cConfFilePath = cairo_dock_check_conf_file_exists (RHYTHMBOX_USER_DATA_DIR, RHYTHMBOX_SHARE_DATA_DIR, RHYTHMBOX_CONF_FILE);
	
	
	//Lecture du fichier de configuration
	int iOriginalWidth = 48, iOriginalHeight = 48;
	rhythmbox_read_conf_file (*cConfFilePath, &iOriginalWidth, &iOriginalHeight);
	
	//Création de l'icone
	rhythmbox_pIcon = cairo_dock_create_icon_for_applet (pDock, iOriginalWidth, iOriginalHeight, conf_defaultTitle, NULL);
	rhythmbox_pCairoContext = cairo_create (rhythmbox_pIcon->pIconBuffer);
	
	//Chargement de l'image "default"
	rhythmbox_pSurface = cairo_dock_create_surface_from_image (
		g_strdup_printf ("%s/default.svg", RHYTHMBOX_SHARE_DATA_DIR),
		rhythmbox_pCairoContext,
		1 + g_fAmplitude,
		(int) rhythmbox_pIcon->fWidth,
		(int) rhythmbox_pIcon->fHeight,
		(int) rhythmbox_pIcon->fWidth,
		(int) rhythmbox_pIcon->fHeight,
		&fImageWidth, &fImageHeight,
		0, 1, FALSE
	);
	//Chargement de l'image "pause"
	rhythmbox_pPauseSurface = cairo_dock_create_surface_from_image (
		g_strdup_printf ("%s/pause.svg", RHYTHMBOX_SHARE_DATA_DIR),
		rhythmbox_pCairoContext,
		1 + g_fAmplitude,
		(int) rhythmbox_pIcon->fWidth,
		(int) rhythmbox_pIcon->fHeight,
		(int) rhythmbox_pIcon->fWidth,
		(int) rhythmbox_pIcon->fHeight,
		&fImageWidth, &fImageHeight,
		0, 1, FALSE
	);
	//Chargement de l'image "play"
	rhythmbox_pPlaySurface = cairo_dock_create_surface_from_image (
		g_strdup_printf ("%s/play.svg", RHYTHMBOX_SHARE_DATA_DIR),
		rhythmbox_pCairoContext,
		1 + g_fAmplitude,
		(int) rhythmbox_pIcon->fWidth,
		(int) rhythmbox_pIcon->fHeight,
		(int) rhythmbox_pIcon->fWidth,
		(int) rhythmbox_pIcon->fHeight,
		&fImageWidth, &fImageHeight,
		0, 1, FALSE
	);
	//Chargement de l'image "stop"
	rhythmbox_pStopSurface = cairo_dock_create_surface_from_image (
		g_strdup_printf ("%s/stop.svg", RHYTHMBOX_SHARE_DATA_DIR),
		rhythmbox_pCairoContext,
		1 + g_fAmplitude,
		(int) rhythmbox_pIcon->fWidth,
		(int) rhythmbox_pIcon->fHeight,
		(int) rhythmbox_pIcon->fWidth,
		(int) rhythmbox_pIcon->fHeight,
		&fImageWidth, &fImageHeight,
		0, 1, FALSE
	);
	//Chargement de l'image "play"
	rhythmbox_pBrokenSurface = cairo_dock_create_surface_from_image (
		g_strdup_printf ("%s/broken.svg", RHYTHMBOX_SHARE_DATA_DIR),
		rhythmbox_pCairoContext,
		1 + g_fAmplitude,
		(int) rhythmbox_pIcon->fWidth,
		(int) rhythmbox_pIcon->fHeight,
		(int) rhythmbox_pIcon->fWidth,
		(int) rhythmbox_pIcon->fHeight,
		&fImageWidth, &fImageHeight,
		0, 1, FALSE
	);
	
	if(rhythmbox_dbus_enable)
	{
		cairo_set_source_surface (rhythmbox_pCairoContext,rhythmbox_pSurface,0,0);
	}
	else
	{
		cairo_set_source_surface (rhythmbox_pCairoContext,rhythmbox_pBrokenSurface,0,0);
	}
	
	cairo_paint (rhythmbox_pCairoContext);
	
	//Enregistrement des notifications	
	cairo_dock_register_notification (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) rhythmbox_action, CAIRO_DOCK_RUN_FIRST);
	cairo_dock_register_notification (CAIRO_DOCK_BUILD_MENU, (CairoDockNotificationFunc) rhythmbox_notification_build_menu, CAIRO_DOCK_RUN_FIRST);

	return rhythmbox_pIcon;
}


//*********************************************************************************
// rhythmbox_stop() :  Fonction appelé à la fermeture de l'applet
//*********************************************************************************
void cd_rhythmbox_stop (void)
{
	g_print ("%s ()\n", __func__);
	rhythmbox_pIcon = NULL;
	
	cairo_dock_remove_notification_func (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) rhythmbox_action);
	cairo_dock_remove_notification_func (CAIRO_DOCK_BUILD_MENU, (CairoDockNotificationFunc) rhythmbox_notification_build_menu);
}

//*********************************************************************************
// rhythmbox_action() : Fonction appelée au clique sur l'icone
// Cette fonction met le lecteur en pause ou en lecture selon son état
//*********************************************************************************
gboolean rhythmbox_action (gpointer *data)
{
	if (data[0] == rhythmbox_pIcon)
	{
		g_print ("%s ()\n", __func__);
		
		if(rhythmbox_getPlaying())
		{
			gchar *command = g_strdup_printf ("rhythmbox-client --pause");
			system (command);
			g_free (command);
		}
		else
		{
			gchar *command = g_strdup_printf ("rhythmbox-client --play");
			system (command);
			g_free (command);
		}
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

//*********************************************************************************
// rhythmbox_onChangeSong() : Fonction executée à chaque changement de musique
//*********************************************************************************
void rhythmbox_onChangeSong(DBusGProxy *player_proxy,const gchar *uri, gpointer data)
{
	if (rhythmbox_pIcon == NULL)
		return ;
	g_print ("%s ()\n",__func__);
	
	if(rhythmbox_playing)
	{
		gchar *songName;
		
		songName = rhythmbox_getSongName(uri);
		rhythmbox_setIconName( songName );
		rhythmbox_iconWitness(1);
		
		if(conf_enableDialogs)
		{
			cairo_dock_show_temporary_dialog (songName,rhythmbox_pIcon,rhythmbox_pDock,conf_timeDialogs);
		}
	}
	else
	{
		rhythmbox_setIconName(conf_defaultTitle);
		rhythmbox_setIconSurface( rhythmbox_pStopSurface );
		rhythmbox_opening = FALSE;
	}
}

//*********************************************************************************
// rhythmbox_onChangeSong() : Fonction executée à chaque changement play/pause
//*********************************************************************************
void rhythmbox_onChangePlaying(DBusGProxy *player_proxy,gboolean playing, gpointer data)
{
	if (rhythmbox_pIcon == NULL)
		return ;
	g_print ("%s ()\n",__func__);
	
	if(playing)
	{
		rhythmbox_setIconSurface( rhythmbox_pPlaySurface );
		rhythmbox_opening = TRUE;
	}
	else if(rhythmbox_opening)
	{
		rhythmbox_setIconSurface( rhythmbox_pPauseSurface );
	}
	
	rhythmbox_playing = playing;
}

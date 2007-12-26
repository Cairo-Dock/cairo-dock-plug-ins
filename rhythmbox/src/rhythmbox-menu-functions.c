#include <stdlib.h>

#include "rhythmbox-dbus.h"
#include "rhythmbox-draw.h"
#include "rhythmbox-menu-functions.h"

CD_APPLET_INCLUDE_MY_VARS

extern gboolean rhythmbox_opening;
extern gboolean rhythmbox_playing;
extern int conf_timeDialogs;

//*********************************************************************************
// rhythmbox_previous : Joue la piste précédante
//*********************************************************************************
static void rhythmbox_previous (GtkMenuItem *menu_item, gpointer *data)
{
	g_print ("%s ()\n", __func__);
	
	g_spawn_command_line_async ("rhythmbox-client --previous", NULL);
}

//*********************************************************************************
// rhythmbox_next : Joue la piste suivante
//*********************************************************************************
static void rhythmbox_next (GtkMenuItem *menu_item, gpointer *data)
{
	g_print ("%s ()\n", __func__);
	g_spawn_command_line_async ("rhythmbox-client --next", NULL);
}

//*********************************************************************************
// rhythmbox_pause : Stop la lecture
//*********************************************************************************
static void rhythmbox_pause (GtkMenuItem *menu_item, gpointer *data)
{
	g_print ("%s ()\n", __func__);
	g_spawn_command_line_async ("rhythmbox-client --pause", NULL);
}

//*********************************************************************************
// rhythmbox_play : Joue la piste
//*********************************************************************************
static void rhythmbox_play (GtkMenuItem *menu_item, gpointer *data)
{
	g_print ("%s ()\n", __func__);
	g_spawn_command_line_async ("rhythmbox-client --play", NULL);
}

static void rhythmbox_music (GtkMenuItem *menu_item, gpointer *data)
{
	music_dialog();
}


//*********************************************************************************
// Informations sur l'applet et l'auteur.
//*********************************************************************************
CD_APPLET_ABOUT ("Applet by Necropotame (Adrien Pilleboue)")


//*********************************************************************************
// Fonction appelée a la construction du menu.
// Cette fonction remplit le menu principal avec les actions previous, next, et information.
//*********************************************************************************
CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_IN_MENU ("Previous", rhythmbox_previous, CD_APPLET_MY_MENU)
	
	CD_APPLET_ADD_IN_MENU ("Next", rhythmbox_next, CD_APPLET_MY_MENU)
	
	CD_APPLET_ADD_IN_MENU ("Information", rhythmbox_music, CD_APPLET_MY_MENU)
	
	CD_APPLET_ADD_ABOUT_IN_MENU (CD_APPLET_MY_MENU)
CD_APPLET_ON_BUILD_MENU_END


//*********************************************************************************
// Fonction appelée au clique sur l'icone.
// Cette fonction met le lecteur en pause ou en lecture selon son état.
//*********************************************************************************
CD_APPLET_ON_CLICK_BEGIN
	g_print ("%s ()\n", __func__);
	
	if(rhythmbox_opening)
	{
		if(rhythmbox_playing)
		{
			g_spawn_command_line_async ("rhythmbox-client --pause", NULL);
		}
		else
		{
			g_spawn_command_line_async ("rhythmbox-client --play", NULL);
		}
	}
	else
	{
		g_spawn_command_line_async ("rhythmbox", NULL);
	}
CD_APPLET_ON_CLICK_END


//*********************************************************************************
// Fonction appelée au clique du milieu sur l'icone.
// Cette fonction passe a la chanson suivante.
//*********************************************************************************
CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	g_print ("%s ()\n", __func__);
	
	rhythmbox_getPlaying();
	if (rhythmbox_playing)
	{
		g_spawn_command_line_async ("rhythmbox-client --next", NULL);
	}
CD_APPLET_ON_MIDDLE_CLICK_END

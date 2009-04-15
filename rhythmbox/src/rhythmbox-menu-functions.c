#include <stdlib.h>
#include <glib/gi18n.h>

#include "rhythmbox-dbus.h"
#include "rhythmbox-draw.h"
#include "rhythmbox-struct.h"
#include "rhythmbox-menu-functions.h"
#include "3dcover-draw.h"


//*********************************************************************************
// rhythmbox_previous : Joue la piste précédante
//*********************************************************************************
static void rhythmbox_previous (GtkMenuItem *menu_item, gpointer *data)
{
	cd_message ("");
	cairo_dock_launch_command ("rhythmbox-client --previous");
}

//*********************************************************************************
// rhythmbox_next : Joue la piste suivante
//*********************************************************************************
static void rhythmbox_next (GtkMenuItem *menu_item, gpointer *data)
{
	cd_message ("");
	cairo_dock_launch_command ("rhythmbox-client --next");
}

static void rhythmbox_play_pause (GtkMenuItem *menu_item, gpointer *data)
{
	cd_message ("");
	cairo_dock_launch_command ("rhythmbox-client --play-pause");
}

static void rhythmbox_music (GtkMenuItem *menu_item, gpointer *data)
{
	music_dialog();
}

//*********************************************************************************
// Informations sur l'applet et l'auteur.
//*********************************************************************************


//*********************************************************************************
// Fonction appelée a la construction du menu.
// Cette fonction remplit le menu principal avec les actions previous, next, et information.
//*********************************************************************************
CD_APPLET_ON_BUILD_MENU_BEGIN
	if (myData.dbus_enable)
	{
		CD_APPLET_ADD_IN_MENU (D_("Previous"), rhythmbox_previous, CD_APPLET_MY_MENU);
		
		CD_APPLET_ADD_IN_MENU (D_("Next (middle-click)"), rhythmbox_next, CD_APPLET_MY_MENU);
		
		CD_APPLET_ADD_IN_MENU (D_("Play/Pause (left-click)"), rhythmbox_play_pause, CD_APPLET_MY_MENU);
		
		CD_APPLET_ADD_IN_MENU (D_("Information"), rhythmbox_music, CD_APPLET_MY_MENU);
	}
	CD_APPLET_ADD_ABOUT_IN_MENU (CD_APPLET_MY_MENU);
CD_APPLET_ON_BUILD_MENU_END


//*********************************************************************************
// Fonction appelée au clique sur l'icone.
// Cette fonction met le lecteur en pause ou en lecture selon son état.
//*********************************************************************************
CD_APPLET_ON_CLICK_BEGIN
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myData.numberButtons != 0  && myConfig.bOpenglThemes)
	{			
		// Actions au clic sur un bouton :
		if (myData.mouseOnButton1)
		{
			if(myData.opening)
			{
				if(myData.playing)
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
		}
		else if (myData.mouseOnButton2)
			g_spawn_command_line_async ("rhythmbox-client --previous", NULL);
		else if (myData.mouseOnButton3)
			g_spawn_command_line_async ("rhythmbox-client --next", NULL);
		else if (myData.mouseOnButton4)
			g_spawn_command_line_async ("rhythmbox", NULL);
		else
		{
			if(myData.opening)
				music_dialog();
			else
				g_spawn_command_line_async ("rhythmbox", NULL);
		}
	}
	else
	{
		cd_message ("");
		
		if(myData.opening)
		{
			if(myData.playing)
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
	}
CD_APPLET_ON_CLICK_END


//*********************************************************************************
// Fonction appelée au clique du milieu sur l'icone.
// Cette fonction passe a la chanson suivante.
//*********************************************************************************
CD_APPLET_ON_MIDDLE_CLICK_BEGIN	
	cd_message ("");
	rhythmbox_getPlaying();
	if (myData.playing)
	{
		g_spawn_command_line_async ("rhythmbox-client --next", NULL);
	}
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_DROP_DATA_BEGIN
	cd_message (" %s --> nouvelle pochette ou chanson !", CD_APPLET_RECEIVED_DATA);

	gboolean isJpeg = g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"jpg") 
		|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"JPG")
		|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"jpeg")
		|| g_str_has_suffix(CD_APPLET_RECEIVED_DATA,"JPEG");
	
	if(isJpeg)
	{
		if(myData.playing_artist != NULL && myData.playing_album != NULL)
		{
			cd_debug("Le fichier est un JPEG");
			GString *command = g_string_new ("");
			if(strncmp(CD_APPLET_RECEIVED_DATA, "http://", 7) == 0)
			{
				cd_debug("Le fichier est distant");
				g_string_printf (command, "wget -O %s/.cache/rhythmbox/covers/\"%s - %s.jpg\" %s",
					g_getenv ("HOME"),
					myData.playing_artist,
					myData.playing_album,
					CD_APPLET_RECEIVED_DATA);
			}
			else
			{
				cd_debug("Le fichier est local");
				gchar *cFilePath = (*CD_APPLET_RECEIVED_DATA == '/' ? g_strdup (CD_APPLET_RECEIVED_DATA) : g_filename_from_uri (CD_APPLET_RECEIVED_DATA, NULL, NULL));
				g_string_printf (command, "cp %s %s/.cache/rhythmbox/covers/\"%s - %s.jpg\"",
					cFilePath,
					g_getenv ("HOME"),
					myData.playing_artist,
					myData.playing_album);
				g_free (cFilePath);
			}
			g_spawn_command_line_async (command->str, NULL);
			cd_debug("La commande est passée");
			g_string_free (command, TRUE);
		}
	}
	else
	{
		gchar *cCommand = g_strdup_printf ("rhythmbox-client --enqueue %s", CD_APPLET_RECEIVED_DATA);
		g_spawn_command_line_async (cCommand, NULL);
		g_free (cCommand);
	}
CD_APPLET_ON_DROP_DATA_END



CD_APPLET_ON_SCROLL_BEGIN
		if (CD_APPLET_SCROLL_DOWN) {
			rhythmbox_next (NULL, NULL);
		}
		else if (CD_APPLET_SCROLL_UP) {
			rhythmbox_previous (NULL, NULL);
		}
		else
			return CAIRO_DOCK_LET_PASS_NOTIFICATION;
CD_APPLET_ON_SCROLL_END

CD_APPLET_ON_UPDATE_ICON_BEGIN

	if (CD_APPLET_MY_CONTAINER_IS_OPENGL  && myConfig.bOpenglThemes)
	{
		cd_opengl_mouse_on_buttons ();	
	}
	
CD_APPLET_ON_UPDATE_ICON_END

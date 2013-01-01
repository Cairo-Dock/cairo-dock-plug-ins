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

#include <stdlib.h>
#include <string.h>
#include <glib/gstdio.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-dbus.h"
#include "3dcover-draw.h"
#include "applet-musicplayer.h"
#include "applet-cover.h"
#include "applet-dbus.h"
#include "applet-draw.h"

static const gchar *s_cDefaultIconName[PLAYER_NB_STATUS] = {"default.svg", "play.svg", "pause.svg", "stop.svg", "broken.svg"};
static const gchar *s_cDefaultIconName3D[PLAYER_NB_STATUS] = {"default.jpg", "play.jpg", "pause.jpg", "stop.jpg", "broken.jpg"};


/* Update the icon on new song / status.
 */
void cd_musicplayer_update_icon (void)
{
	cd_message ("%s (uri : %s / title : %s)", __func__, myData.cPlayingUri, myData.cTitle);
	if (myData.cPlayingUri != NULL || myData.cTitle != NULL)
	{
		if (myData.iPlayingStatus == PLAYER_PLAYING || myData.iPlayingStatus == PLAYER_PAUSED)
		{
			// display current song on the label
			if (myDock)
			{
				if ((!myData.cArtist || !myData.cTitle) && myData.cPlayingUri)
				{
					gchar *str = strrchr (myData.cPlayingUri, '/');
					if (str)
						str ++;
					else
						str = myData.cPlayingUri;
					CD_APPLET_SET_NAME_FOR_MY_ICON (str);
				}
				else
					CD_APPLET_SET_NAME_FOR_MY_ICON_PRINTF ("%s - %s", myData.cArtist ? myData.cArtist : D_("Unknown artist"), myData.cTitle ? myData.cTitle : D_("Unknown title"));
			}
			
			// display current track on the quick-info.
			if (myConfig.iQuickInfoType == MY_APPLET_TRACK && myData.iTrackListLength > 0 && myData.iTrackListIndex > 0)
			{
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%s%d", (myDesklet && myDesklet->container.iWidth >= 64 ? D_("Track") : ""), myData.iTrackListIndex);  // inutile de redessiner notre icone, ce sera fait plus loin.
			}
			else
			{
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
			}
			
			// animate the icon and pop-up the dialog.
			if (myData.iPlayingStatus == PLAYER_PLAYING)
			{
				cd_musicplayer_animate_icon (1);
				if(myConfig.bEnableDialogs)
				{
					cd_musicplayer_popup_info (myConfig.iDialogDuration);
				}
			}
		}
		/**else
		{
			cd_musicplayer_apply_status_surface (PLAYER_STOPPED);
			CD_APPLET_SET_NAME_FOR_MY_ICON (myData.cTitle ? myData.cTitle : myData.pCurrentHandler ? myData.pCurrentHandler->name : myConfig.cDefaultTitle);
		}*/
		
		// re-paint the icon if needed.
		if (myConfig.bEnableCover && myData.cover_exist && myData.cCoverPath != NULL)  // cover is available
		{
			if (cairo_dock_strings_differ (myData.cCoverPath, myData.cPreviousCoverPath))  // and cover has changed -> apply the new one
				cd_musiplayer_apply_cover ();
		}
		else  // no cover -> set the status surface.
		{
			if ((myConfig.bEnableCover && myData.cPreviousCoverPath != NULL)  // currently a cover is set
			|| myData.pPreviousPlayingStatus != myData.iPlayingStatus)  // or the status has changed
			cd_musicplayer_apply_status_surface (myData.iPlayingStatus);
		}
	}
	else  // aucune donnees, c'est soit un probleme soit le lecteur qui s'est ferme.
	{
		if (myData.bIsRunning)
		{
			cd_musicplayer_apply_status_surface (PLAYER_STOPPED);
			if (myConfig.cDefaultTitle)
			{
				CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cDefaultTitle);
			}
			else if (myData.pCurrentHandler->cDisplayedName != NULL)
			{
				CD_APPLET_SET_NAME_FOR_MY_ICON (myData.pCurrentHandler->cDisplayedName);
			}
			else
			{
				CD_APPLET_SET_NAME_FOR_MY_ICON (myData.pCurrentHandler->name);
			}
		}
		else
		{
			cd_musicplayer_apply_status_surface (PLAYER_NONE);
			if (myConfig.cDefaultTitle)
			{
				CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cDefaultTitle);
			}
			else
			{
				CD_APPLET_SET_NAME_FOR_MY_ICON (myApplet->pModule->pVisitCard->cTitle);
			}
		}
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
	}
}


/* Display information about the current song in a dialog.
 */
void cd_musicplayer_popup_info (gint iDialogDuration)
{
	cairo_dock_remove_dialog_if_any (myIcon);
	if (myData.iPlayingStatus == PLAYER_PLAYING || myData.iPlayingStatus == PLAYER_PAUSED)
	{
		if (myData.cTitle || myData.cArtist || myData.cAlbum)
		{
			cairo_dock_show_temporary_dialog_with_icon_printf (
				"%s: %s\n%s: %s\n%s: %s\n%s: %d:%02d\n%s %d, %s %d/%d",
				myIcon,
				myContainer,
				iDialogDuration,
				MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
				D_("Artist"),
				myData.cArtist != NULL ? myData.cArtist : D_("Unknown"),
				D_("Title"),
				myData.cTitle != NULL ? myData.cTitle : D_("Unknown"),
				D_("Album"),
				myData.cAlbum != NULL ? myData.cAlbum : D_("Unknown"),
				D_("Length"),
				myData.iSongLength/60, myData.iSongLength%60,  // it's not often to have a song more than 1h!
				D_("Track n°"), myData.iTrackNumber,
				D_("Song n°"), myData.iTrackListIndex+1, myData.iTrackListLength);  // iTrackListIndex start with 0.
		}
		else if (myData.cPlayingUri)
		{
			// no tags but with a path...
			gchar *str = strrchr (myData.cPlayingUri, '/');
			if (str)
				str ++;
			else
				str = myData.cPlayingUri;
			cairo_dock_remove_html_spaces (str); // %20 => " "
			cairo_dock_show_temporary_dialog_with_icon_printf ("%s : %s",
				myIcon,
				myContainer,
				iDialogDuration,
				MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
				D_("Current song"),
				str);
		}  // else just ignore silently
	}
	else
	{
		cairo_dock_show_temporary_dialog_with_icon (D_("There is no media playing."),
			myIcon,
			myContainer,
			iDialogDuration,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
	}
}

/* Anime l'icone au changement de musique
 */
void cd_musicplayer_animate_icon (int animationLength)
{
	if (myDock && myConfig.cChangeAnimation != NULL)
	{
		CD_APPLET_ANIMATE_MY_ICON (myConfig.cChangeAnimation, animationLength);
	}
}

/* Applique la surface correspondant a un etat sur l'icone.
 */
void cd_musicplayer_apply_status_surface (MyPlayerStatus iStatus)
{
	cd_debug ("%s (%d)", __func__, iStatus);
	g_return_if_fail (iStatus < PLAYER_NB_STATUS);
	gboolean bUse3DTheme = (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.bOpenglThemes);
	cairo_surface_t *pSurface = myData.pSurfaces[iStatus];
	
	// load the surface if not already in cache
	if (pSurface == NULL)
	{
		gchar *cUserIcon = myConfig.cUserImage[iStatus];
		if (cUserIcon != NULL)  // l'utilisateur a defini une icone perso pour ce statut => on essaye de la charger.
		{
			gchar *cUserImagePath = cairo_dock_search_icon_s_path (cUserIcon, MAX (myIcon->image.iWidth, myIcon->image.iHeight));
			myData.pSurfaces[iStatus] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath ? cUserImagePath : cUserIcon);  // si on a trouve une icone, on la prend, sinon on considere le fichier comme une image.
			g_free (cUserImagePath);
		}
		if (myData.pSurfaces[iStatus] == NULL)  // pas d'icone perso pour ce statut, ou l'icone specifiee n'a pas ete trouvee ou pas ete chargee => on prend l'icone par defaut.
		{
			const gchar **cIconName = (bUse3DTheme ? s_cDefaultIconName3D : s_cDefaultIconName);
			gchar *cImagePath = g_strdup_printf (MY_APPLET_SHARE_DATA_DIR"/%s", cIconName[iStatus]);
			myData.pSurfaces[iStatus] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cImagePath);
			g_free (cImagePath);
		}
		pSurface = myData.pSurfaces[iStatus];
		g_return_if_fail (pSurface != NULL);
	}
	
	// apply the surface
	if (bUse3DTheme)  // 3D theme -> make a transition
	{
		if (myData.iPrevTextureCover != 0)
			_cairo_dock_delete_texture (myData.iPrevTextureCover);
		myData.iPrevTextureCover = myData.TextureCover;
		myData.TextureCover = cairo_dock_create_texture_from_surface (pSurface);
		if (myData.iPrevTextureCover != 0)
		{
			myData.iCoverTransition = NB_TRANSITION_STEP;
			cairo_dock_launch_animation (myContainer);
		}
		else
		{
			cd_opengl_render_to_texture (myApplet);
			CD_APPLET_REDRAW_MY_ICON;
		}
	}
	else  // just apply the surface (we could make a transition too ...)
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (pSurface);
	}
}

void cd_musiplayer_apply_cover (void)
{
	cd_debug ("%s (%s)", __func__, myData.cCoverPath);
	g_return_if_fail (myData.cCoverPath != NULL);
	
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.bOpenglThemes)
	{
		if (myData.iPrevTextureCover != 0)
			_cairo_dock_delete_texture (myData.iPrevTextureCover);
		myData.iPrevTextureCover = myData.TextureCover;
		myData.TextureCover = cairo_dock_create_texture_from_image (myData.cCoverPath);
		if (myData.iPrevTextureCover != 0)
		{
			myData.iCoverTransition = NB_TRANSITION_STEP;
			cairo_dock_launch_animation (myContainer);
		}
		else
		{
			cd_opengl_render_to_texture (myApplet);
			CD_APPLET_REDRAW_MY_ICON;
		}
	}
	else
	{
		CD_APPLET_SET_IMAGE_ON_MY_ICON (myData.cCoverPath);
		CD_APPLET_REDRAW_MY_ICON;
	}
}

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

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-musicplayer.h"
#include "3dcover-draw.h"

//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.iQuickInfoType 		= CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "quick-info_type", MY_APPLET_TIME_ELAPSED);
	
	myConfig.cMusicPlayer 			= CD_CONFIG_GET_STRING ("Configuration", "current-player");  // NULL by default
	myConfig.cLastKnownDesktopFile 	= CD_CONFIG_GET_STRING ("Configuration", "desktop-entry");  // see applet-struct.h
	myConfig.cMpris2Name			= CD_CONFIG_GET_STRING ("Configuration", "mpris2-name");
	myConfig.cDefaultTitle			= CD_CONFIG_GET_STRING ("Icon", "name");
	
	myConfig.bEnableDialogs 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_dialogs");
	myConfig.iDialogDuration 		= 1000 * CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "time_dialog", 4);
	
	myConfig.cChangeAnimation 		= CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", "change_animation", "wobbly");
	myConfig.bEnableCover			= CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_cover");
	myConfig.bOpenglThemes 			= g_bUseOpenGL && CD_CONFIG_GET_BOOLEAN ("Configuration", "enable_opengl_themes");
	myConfig.bStealTaskBarIcon 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "inhibate appli");
	
	myConfig.cUserImage[PLAYER_NONE] 	= CD_CONFIG_GET_STRING ("Configuration", "default icon");
	myConfig.cUserImage[PLAYER_PLAYING] = CD_CONFIG_GET_STRING ("Configuration", "play icon");
	myConfig.cUserImage[PLAYER_PAUSED] 	= CD_CONFIG_GET_STRING ("Configuration", "pause icon");
	myConfig.cUserImage[PLAYER_STOPPED] = CD_CONFIG_GET_STRING ("Configuration", "stop icon");
	myConfig.cUserImage[PLAYER_BROKEN] 	= CD_CONFIG_GET_STRING ("Configuration", "broken icon");

	myConfig.bDownload   = CD_CONFIG_GET_BOOLEAN ("Configuration", "DOWNLOAD");
	myConfig.bPauseOnClick = (CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "pause on click", 1) == 0);  // c'est une liste numerotee de 2 elements.
	if (!myConfig.bPauseOnClick)  // pour pouvoir agir sur la fenetre, il faut voler l'appli (plus tellement vrai avec MPRIS2...).
		myConfig.bStealTaskBarIcon = TRUE;
	myConfig.bNextPrevOnScroll = (CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "scrolling", 0) == 0);  // c'est une liste numerotee de 2 elements.
	
	//\_______________ On on recupere le theme choisi.
	if (myConfig.bOpenglThemes)
		myConfig.cThemePath = CD_CONFIG_GET_THEME_PATH ("Configuration", "theme", "themes", "cd_box_3d");
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN

	g_free (myConfig.cDefaultTitle);
	g_free (myConfig.cMusicPlayer);
	g_free (myConfig.cLastKnownDesktopFile);
	
	int i;
	for (i = 0; i < PLAYER_NB_STATUS; i ++) {
		g_free (myConfig.cUserImage[i]);
	}
	
	g_free (myConfig.cThemePath);
	
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	int i;
	for (i = 0; i < PLAYER_NB_STATUS; i ++) {
		if (myData.pSurfaces[i] != NULL)
			cairo_surface_destroy (myData.pSurfaces[i]);
	}
	
	if (myData.pCover != NULL)
		cairo_surface_destroy (myData.pCover);
	
	g_free (myData.cRawTitle);
	g_free (myData.cTitle);
	g_free (myData.cArtist);
	g_free (myData.cAlbum);
	g_free (myData.cCoverPath);
	g_free (myData.cPreviousCoverPath);
	g_free (myData.cPreviousRawTitle);
	
	// On s'occupe des handlers.
	g_list_free_full (myData.pHandlers, cd_musicplayer_free_handler);
	
	// Bye bye pauvres textures opengl
	cd_opengl_reset_opengl_datas (myApplet);
	
CD_APPLET_RESET_DATA_END



static GCancellable *s_pCancel = NULL;
static GtkWidget *s_pComboBox = NULL;
static CDMPInfo *s_pInfo = NULL; // current player set in the config

static void _combo_destroyed (G_GNUC_UNUSED gpointer data, G_GNUC_UNUSED GObject *pObj)
{
	CD_APPLET_ENTER;
	
	if (s_pCancel)
	{
		g_cancellable_cancel (s_pCancel);
		g_object_unref (s_pCancel);
		s_pCancel = NULL;
	}
	cd_musicplayer_info_free (s_pInfo);
	s_pInfo = NULL;
	s_pComboBox = NULL;
	
	CD_APPLET_LEAVE ();
}

static void _got_players (gboolean bSuccess, GList *res)
{
	CD_APPLET_ENTER;
	
	if (s_pCancel)
	{
		g_object_unref (s_pCancel);
		s_pCancel = NULL;
	}
	
	if (!s_pComboBox)
	{
		// should not happen
		CD_APPLET_LEAVE ();
	}
	
	if (bSuccess && res)
	{
		GtkListStore *pItems = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
		GList *it;
		for (it = res; it; it = it->next)
		{
			CDMPInfo *pInfo = (CDMPInfo*)it->data;
			
			if (s_pInfo && !g_strcmp0 (s_pInfo->cMpris2Name, pInfo->cMpris2Name))
				continue; // will add later
			
			GtkTreeIter iter;
			gtk_list_store_append (pItems, &iter);
			gtk_list_store_set (pItems, &iter, 0, pInfo->cName, 1, pInfo->cDesktopFile, 2, pInfo->cMpris2Name, -1);
		}
		
		gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (pItems), 0, GTK_SORT_ASCENDING);
		
		gtk_combo_box_set_model (GTK_COMBO_BOX (s_pComboBox), GTK_TREE_MODEL (pItems));
		// add currently selected element
		if (s_pInfo)
		{
			GtkTreeIter iter;
			gtk_list_store_append (pItems, &iter);
			gtk_list_store_set (pItems, &iter, 0, s_pInfo->cName, 1, s_pInfo->cDesktopFile, 2, s_pInfo->cMpris2Name, -1);
			gtk_combo_box_set_active_iter (GTK_COMBO_BOX (s_pComboBox), &iter);
			cd_musicplayer_info_free (s_pInfo);
			s_pInfo = NULL;
		}
		
		g_object_unref (G_OBJECT (pItems)); // ref taken by the combo box
		
		GtkCellRenderer *rend = gtk_cell_renderer_text_new ();
		gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (s_pComboBox), rend, FALSE);
		gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (s_pComboBox), rend, "text", 0, NULL);
		gtk_widget_set_sensitive (s_pComboBox, TRUE);
	}
	
	CD_APPLET_LEAVE ();
}

void cd_musicplayer_load_custom_widget (GldiModuleInstance *myApplet, GKeyFile* pKeyFile, GSList *pWidgetList)
{
	if (s_pComboBox) return; // already loaded
	
	CairoDockGroupKeyWidget *pGroupKeyWidget = cairo_dock_gui_find_group_key_widget_in_list (pWidgetList, "Configuration", "current-player-selector");
	g_return_if_fail (pGroupKeyWidget != NULL);
	
	if (s_pInfo) cd_musicplayer_info_free (s_pInfo);
	s_pInfo = g_new0 (CDMPInfo, 1);
	s_pInfo->cName = g_key_file_get_string (pKeyFile, "Configuration", "current-player", NULL);
	s_pInfo->cDesktopFile = g_key_file_get_string (pKeyFile, "Configuration", "desktop-entry", NULL);
	s_pInfo->cMpris2Name = g_key_file_get_string (pKeyFile, "Configuration", "mpris2-name", NULL);
	
	gboolean bValid = TRUE;
	if (!s_pInfo->cName || !*s_pInfo->cName) bValid = FALSE;
	else if (!s_pInfo->cDesktopFile || !*s_pInfo->cDesktopFile) bValid = FALSE;
	else if (!s_pInfo->cMpris2Name || !*s_pInfo->cMpris2Name) bValid = FALSE;
	if (!bValid)
	{
		cd_musicplayer_info_free (s_pInfo);
		s_pInfo = NULL;
	}
	
	GtkWidget *pComboBox = gtk_combo_box_new ();
	gtk_widget_set_sensitive (pComboBox, FALSE);
	gtk_widget_set_size_request (pComboBox, 100, -1);
	gtk_box_pack_end (GTK_BOX (pGroupKeyWidget->pKeyBox), pComboBox, FALSE, FALSE, 0);
	
	g_object_weak_ref (G_OBJECT (pComboBox), _combo_destroyed, NULL);
	s_pComboBox = pComboBox;
	
	s_pCancel = g_cancellable_new (); // should be NULL at this point
	
	cd_musicplayer_get_known_players (_got_players, s_pCancel);
}

void cd_musicplayer_save_custom_widget (GldiModuleInstance *myApplet, GKeyFile *pKeyFile, GSList *pWidgetList)
{
	if (!s_pComboBox) return;
	if (!gtk_widget_get_sensitive (s_pComboBox)) return;
	
	GtkTreeIter iter;
	if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (s_pComboBox), &iter))
		return;
	
	GtkTreeModel *pModel = gtk_combo_box_get_model (GTK_COMBO_BOX (s_pComboBox));
	
	gchar *name;
	gchar *id;
	gchar *mpris2;
	
	gtk_tree_model_get (pModel, &iter, 0, &name, 1, &id, 2, &mpris2, -1);
	
	g_key_file_set_string (pKeyFile, "Configuration", "current-player", name ? name : "");
	g_key_file_set_string (pKeyFile, "Configuration", "desktop-entry", id ? id : "");
	g_key_file_set_string (pKeyFile, "Configuration", "mpris2-name", mpris2 ? mpris2 : "");
	
	g_free (name);
	g_free (id);
	g_free (mpris2);
}


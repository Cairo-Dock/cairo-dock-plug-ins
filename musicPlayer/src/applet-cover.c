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

#include <glib/gstdio.h>
#include "applet-amazon.h"
#include "applet-draw.h"
#include "applet-cover.h"

static gchar *_find_cover_in_common_dirs (void)
{
	gchar *cCoverPath = NULL;
	gchar *cSongPath = (myData.cPlayingUri ? g_filename_from_uri (myData.cPlayingUri, NULL, NULL) : NULL);  // on teste d'abord dans le repertoire de la chanson.
	if (cSongPath != NULL)  // c'est une chanson en local.
	{
		gchar *cSongDir = g_path_get_dirname (cSongPath);
		g_free (cSongPath);

		cCoverPath = g_strdup_printf ("%s/%s - %s.jpg", cSongDir, myData.cArtist, myData.cAlbum);
		cd_debug ("MP -   test de %s", cCoverPath);
		if (! g_file_test (cCoverPath, G_FILE_TEST_EXISTS))
		{
			g_free (cCoverPath);
			cCoverPath = g_strdup_printf ("%s/cover.jpg", cSongDir);
			cd_debug ("MP -   test de %s", cCoverPath);
			if (! g_file_test (cCoverPath, G_FILE_TEST_EXISTS))
			{
				g_free (cCoverPath);
				cCoverPath = g_strdup_printf ("%s/Cover.jpg", cSongDir);
				cd_debug ("MP -   test de %s", cCoverPath);
				if (! g_file_test (cCoverPath, G_FILE_TEST_EXISTS))
				{
					g_free (cCoverPath);
					cCoverPath = g_strdup_printf ("%s/cover.jpeg", cSongDir);
					cd_debug ("MP -   test de %s", cCoverPath);
					if (! g_file_test (cCoverPath, G_FILE_TEST_EXISTS))
					{
						g_free (cCoverPath);
						cCoverPath = g_strdup_printf ("%s/album.jpg", cSongDir);
						cd_debug ("MP -   test de %s", cCoverPath);
						if (! g_file_test (cCoverPath, G_FILE_TEST_EXISTS))
						{
							g_free (cCoverPath);
							cCoverPath = g_strdup_printf ("%s/albumart.jpg", cSongDir);
							cd_debug ("MP -   test de %s", cCoverPath);
							if (! g_file_test (cCoverPath, G_FILE_TEST_EXISTS))
							{
								g_free (cCoverPath);
								cCoverPath = g_strdup_printf ("%s/folder.jpg", cSongDir);
								cd_debug ("MP -   test de %s", cCoverPath);
								if (! g_file_test (cCoverPath, G_FILE_TEST_EXISTS))
								{
									g_free (cCoverPath);
									cCoverPath = NULL;
								}
							}
						}
					}
				}
			}
		}
		g_free (cSongDir);
	}
	
	if (cCoverPath == NULL)  // on regarde maintenant dans le cache.
	{
		cd_debug("MP : we can also check the 'cache' directory");

		if (myData.pCurrentHandler->cCoverDir)
		{
			cCoverPath = g_strdup_printf("%s/%s - %s.jpg", myData.pCurrentHandler->cCoverDir, myData.cArtist, myData.cAlbum);
			myData.bCoverNeedsTest = TRUE;  // on testera sur sa taille.
		}
		else  // le lecteur n'a pas de cache, on utilise le notre.
		{
			cCoverPath = g_strdup_printf ("%s/musicplayer/%s - %s.jpg", g_cCairoDockDataDir, myData.cArtist, myData.cAlbum);
		}
	}
	
	return cCoverPath;
}

static gboolean cd_musicplayer_check_size_is_constant (const gchar *cFilePath)
{
	int iSize = cairo_dock_get_file_size (cFilePath);
	gboolean bConstantSize = (iSize != 0 && iSize == myData.iCurrentFileSize);
	myData.iCurrentFileSize = iSize;
	cd_debug ("MP: file size: %d", iSize);
	return bConstantSize;
}

static gboolean _check_cover_file_size (gpointer data)
{
	myData.iNbCheckCover ++;
	if (myData.iNbCheckCover > 5)
	{
		// still no file, try to set it ourselves.
		g_free (myData.cCoverPath);
		myData.cCoverPath = _find_cover_in_common_dirs ();
		if (myData.cCoverPath != NULL)
		{
			if (cairo_dock_strings_differ (myData.cCoverPath, myData.cPreviousCoverPath))  // cover has changed, apply it.
			{
				cd_musiplayer_apply_cover ();
			}
		}
		else if (myConfig.bDownload)
		{
			/// search it online ...
			cd_musicplayer_dl_cover ();  // seems that the Amazon service no longer works :-/
		}
		
		myData.iSidCheckCover = 0;
		return FALSE;
	}
	if (cd_musicplayer_check_size_is_constant (myData.cCoverPath))
	{
		/// file is complete, apply it on the icon...
		myData.cover_exist = TRUE;
		if (myData.iPlayingStatus == PLAYER_PLAYING || myData.iPlayingStatus == PLAYER_PAUSED)  // if it has stopped before we got the cover, don't draw it.
		{
			cd_musiplayer_apply_cover ();
		}
		
		myData.iSidCheckCover = 0;
		return FALSE;
	}
	return TRUE;
}
static gboolean _check_cover_file_exists (gpointer data)
{
	myData.iNbCheckCover ++;
	if (myData.iNbCheckCover > 3)
	{
		// still no file, try to set it ourselves.
		g_free (myData.cCoverPath);
		myData.cCoverPath = _find_cover_in_common_dirs ();
		if (myData.cCoverPath != NULL)
		{
			if (cairo_dock_strings_differ (myData.cCoverPath, myData.cPreviousCoverPath))  // cover has changed, apply it.
			{
				myData.iNbCheckCover = 0;
				myData.iSidCheckCover = g_timeout_add_seconds (1, (GSourceFunc)_check_cover_file_size, NULL);
				return FALSE;
			}
		}
		else if (myConfig.bDownload)
		{
			/// search it online ...
			cd_musicplayer_dl_cover ();  // seems that the Amazon service no longer works :-/
		}
		
		myData.iSidCheckCover = 0;
		return FALSE;
	}
	
	if (myData.cCoverPath && g_file_test (myData.cCoverPath, G_FILE_TEST_EXISTS))
	{
		/// file exists, now check for its size
		myData.iNbCheckCover = 0;
		myData.iSidCheckCover = g_timeout_add_seconds (1, (GSourceFunc)_check_cover_file_size, NULL);
		
		return FALSE;
	}
	return TRUE;
}
static gboolean _get_cover_again (gpointer data)
{
	g_print ("%s ()\n", __func__);
	myData.iNbCheckCover ++;
	if (myData.iNbCheckCover > 1)
	{
		// still no file, try to set it ourselves.
		g_free (myData.cCoverPath);
		myData.cCoverPath = _find_cover_in_common_dirs ();
		if (myData.cCoverPath != NULL)
		{
			if (cairo_dock_strings_differ (myData.cCoverPath, myData.cPreviousCoverPath))  // cover has changed, apply it.
			{
				myData.iNbCheckCover = 0;
				myData.iSidCheckCover = g_timeout_add_seconds (1, (GSourceFunc)_check_cover_file_size, NULL);
				return FALSE;
			}
		}
		else if (myConfig.bDownload)
		{
			/// search it online ...
			cd_musicplayer_dl_cover ();  // seems that the Amazon service no longer works :-/
		}
		
		myData.iSidCheckCover = 0;
		return FALSE;
	}
	
	myData.pCurrentHandler->get_cover ();  // will call 'cd_musicplayer_set_cover_path()' if it got a cover.
	return TRUE;
}
/* 3 cases:
- a file is given and exists -> wait until its size is constant
- a file is given but doesn't exist yet -> wait until it exists, then until its size is constant
- nothing -> re-try, and it will call 'cd_musicplayer_set_cover_path' back.
*/
void cd_musicplayer_set_cover_path (const gchar *cGivenCoverPath)
{
	if (! myConfig.bEnableCover)  // cover not welcome => abort the mission.
	{
		myData.cover_exist = FALSE;
		return;
	}
	
	if (myData.cCoverPath && ! cairo_dock_strings_differ (myData.cCoverPath, cGivenCoverPath))  // cover has not changed => nothing to do (if we are checking for it, keep doing).
	{
		return;
	}
	g_print ("%s (%s -> %s)\n", __func__, myData.cCoverPath, cGivenCoverPath);
	
	g_free (myData.cPreviousCoverPath);
	myData.cPreviousCoverPath = myData.cCoverPath;  // remember the previous cover..
	myData.cCoverPath = NULL;
	myData.iCurrentFileSize = 0;
	myData.cover_exist = FALSE;
	if (myData.iSidCheckCover != 0)
	{
		g_source_remove (myData.iSidCheckCover);
		myData.iSidCheckCover = 0;
	}
	myData.iNbCheckCover = 0;
	
	if (cGivenCoverPath != NULL)  // we got something from the service, check it.
	{
		if (strncmp (cGivenCoverPath, "file://", 7) == 0)  // local URI
		{
			myData.cCoverPath = g_filename_from_uri (cGivenCoverPath, NULL, NULL);
		}
		else  // local file or remote adress.
		{
			myData.cCoverPath = g_strdup (cGivenCoverPath);
		}
		
		if (! g_file_test (myData.cCoverPath, G_FILE_TEST_EXISTS))  // file does not exist, re-try a few times.
		{
			myData.iSidCheckCover = g_timeout_add_seconds (1, (GSourceFunc)_check_cover_file_exists, NULL);
		}
		else  // file exists, check its size until it's constant
		{
			myData.iSidCheckCover = g_timeout_add_seconds (1, (GSourceFunc)_check_cover_file_size, NULL);
		}
	}
	else  // no data from the service, re-try later if possible.
	{
		if (myData.pCurrentHandler->get_cover != NULL)
			myData.iSidCheckCover = g_timeout_add_seconds (2, (GSourceFunc)_get_cover_again, NULL);
		else
		{
			g_free (myData.cCoverPath);
			myData.cCoverPath = _find_cover_in_common_dirs ();
			if (myData.cCoverPath != NULL)
			{
				if (cairo_dock_strings_differ (myData.cCoverPath, myData.cPreviousCoverPath))  // cover has changed, apply it.
				{
					cd_musiplayer_apply_cover ();
				}
			}
			else if (myConfig.bDownload)
			{
				/// search it online ...
				cd_musicplayer_dl_cover ();  // seems that the Amazon service no longer works :-/
			}
		}
	}
}



static void _cd_download_missing_cover (const gchar *cURL)
{
	if (cURL == NULL)
		return ;
	g_return_if_fail (myData.cCoverPath != NULL);
	if (! g_file_test (myData.cCoverPath, G_FILE_TEST_EXISTS))
	{
		gchar *cCommand = g_strdup_printf ("wget \"%s\" -O \"%s\" -t 2 -T 30 > /dev/null 2>&1", cURL, myData.cCoverPath);
		cd_debug ("MP - %s",cCommand);
		cairo_dock_launch_command (cCommand);
		g_free (cCommand);
		g_free (myData.cMissingCover);
		myData.cMissingCover = g_strdup (myData.cCoverPath);
	}
}

static gboolean _check_xml_file (gpointer data)
{
	CD_APPLET_ENTER;
	// on teste la presence du fichier xml.
	if (g_file_test (myData.cCurrentXmlFile, G_FILE_TEST_EXISTS))
	{
		cd_message ("MP : this XML file '%s' is available", myData.cCurrentXmlFile);
		// s'il est complet, on le lit.
		if (cd_musicplayer_check_size_is_constant (myData.cCurrentXmlFile))
		{
			cd_message ("MP : constant size (%d)", myData.iCurrentFileSize);
			
			cd_debug ("MP - before the extraction: %s / %s", myData.cArtist, myData.cAlbum);
			gchar *cURL = cd_extract_url_from_xml_file (myData.cCurrentXmlFile, &myData.cArtist, &myData.cAlbum, &myData.cTitle);
			cd_debug ("MP - after the extraction: %s / %s", myData.cArtist, myData.cAlbum);
			cd_debug ("MP - we can download this cover: %s -> %s", cURL, myData.cCoverPath);
			if ((!myData.cCoverPath || g_strstr_len (myData.cCoverPath, -1, "(null)") != NULL) && myData.cArtist && myData.cAlbum)
			{
				g_free (myData.cCoverPath);
				if (myData.pCurrentHandler->cCoverDir)
				{
					myData.cCoverPath = g_strdup_printf("%s/%s - %s.jpg", myData.pCurrentHandler->cCoverDir, myData.cArtist, myData.cAlbum);
				}
				else  // le lecteur n'a pas de cache, on utilise le notre.
				{
					myData.cCoverPath = g_strdup_printf ("%s/musicplayer/%s - %s.jpg", g_cCairoDockDataDir, myData.cArtist, myData.cAlbum);
				}
				cd_debug ("MP - new cCoverPath: %s", myData.cCoverPath);
			}
			
			// on lance le dl du fichier image.
			_cd_download_missing_cover (cURL);
			g_free (cURL);
			
			// on teste en boucle sur la taille du fichier image.
			myData.iCurrentFileSize = 0;
			myData.iNbCheckFile = 0;
			myData.iSidCheckCover = g_timeout_add_seconds (1, (GSourceFunc)_check_cover_file_size, NULL);  /// TODO: we need to pass a parameter to not redo the online search if the download fails ...
			
			// on quitte la boucle de test du fichier XML.
			g_remove (myData.cCurrentXmlFile);
			g_free (myData.cCurrentXmlFile);
			myData.cCurrentXmlFile = NULL;
			myData.iSidCheckXmlFile = 0;
			CD_APPLET_LEAVE (FALSE);
			//return FALSE;
		}
	}
	// si non present ou non complet, on continue a tester qques secondes.
	myData.iNbCheckFile ++;
	if (myData.iNbCheckFile > 12)  // on abandonne au bout de 3s.
	{
		cd_debug ("MP - we delete the XML file");
		g_remove (myData.cCurrentXmlFile);
		g_free (myData.cCurrentXmlFile);
		myData.cCurrentXmlFile = NULL;
		myData.iSidCheckXmlFile = 0;
		myData.iNbCheckFile = 0;
		CD_APPLET_LEAVE (FALSE);
		//return FALSE;
	}
	CD_APPLET_LEAVE (TRUE);
	//return TRUE;
}
void cd_musicplayer_dl_cover (void)
{
	g_print ("MP-COVER - %s (%s, %s, %s)\n", __func__, myData.cArtist, myData.cAlbum, myData.cPlayingUri);
	// on oublie ce qu'on etait en train de recuperer.
	g_free (myData.cCurrentXmlFile);
	myData.cCurrentXmlFile = NULL;
	myData.bCoverNeedsTest = TRUE;  // on testera sur sa taille.
	
	// lance le dl du fichier XML.
	myData.cCurrentXmlFile = cd_get_xml_file (myData.cArtist, myData.cAlbum, myData.cPlayingUri);
	
	g_print ("XML file: %s\n", myData.cCurrentXmlFile);
	// on teste en boucle sur la taille du fichier XML.
	myData.iCurrentFileSize = 0;
	myData.iNbCheckFile = 0;
	if (myData.iSidCheckXmlFile == 0)
	{
		if (myData.cCurrentXmlFile != NULL)
			myData.iSidCheckXmlFile = g_timeout_add (500, (GSourceFunc) _check_xml_file, NULL);
	}
	else if (myData.cCurrentXmlFile == NULL)
	{
		g_source_remove (myData.iSidCheckXmlFile);
		myData.iSidCheckXmlFile = 0;
	}
}

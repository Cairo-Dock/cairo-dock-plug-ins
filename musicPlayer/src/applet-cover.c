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

static void cd_musicplayer_dl_cover (void);


static gchar *_find_cover_in_common_dirs (void)
{
	gchar *cCoverPath = NULL;
	// search a cover in the local folder where the song is located.
	gchar *cSongPath = (myData.cPlayingUri ? g_filename_from_uri (myData.cPlayingUri, NULL, NULL) : NULL);
	if (cSongPath != NULL)  // local file, let's go on.
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
	
	// if not found, search in the cache
	if (cCoverPath == NULL)
	{
		cd_debug("MP : we can also check the 'cache' directory");
		if (myData.pCurrentHandler->cCoverDir)  // the player has its own cache.
		{
			cCoverPath = g_strdup_printf("%s/%s - %s.jpg", myData.pCurrentHandler->cCoverDir, myData.cArtist, myData.cAlbum);
		}
		else  // else, use our own cache.
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
			cd_musicplayer_dl_cover ();
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
			cd_musicplayer_dl_cover ();
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
/* Not used
static gboolean _get_cover_again (gpointer data)
{
	cd_debug ("%s ()", __func__);
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
			cd_musicplayer_dl_cover ();
		}
		
		myData.iSidCheckCover = 0;
		return FALSE;
	}
	
	myData.pCurrentHandler->get_cover ();  // will call 'cd_musicplayer_set_cover_path()' if it got a cover.
	return TRUE;
}
*/
/* 3 cases:
- a file is given and exists -> wait until its size is constant
- a file is given but doesn't exist yet -> wait until it exists, then until its size is constant
- nothing -> search in the local dir, then in the cache -> if exists, wait until its size is constant, else download it.
*/
static void _reset_cover_state (void)
{
	myData.cover_exist = FALSE;
	myData.iCurrentFileSize = 0;
	if (myData.iSidCheckCover != 0)
	{
		g_source_remove (myData.iSidCheckCover);
		myData.iSidCheckCover = 0;
	}
	myData.iNbCheckCover = 0;
	if (myData.pCoverTask != NULL)
	{
		cairo_dock_discard_task (myData.pCoverTask);
		myData.pCoverTask = NULL;
	}
}
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
	cd_debug ("%s (%s -> %s)", __func__, myData.cCoverPath, cGivenCoverPath);
	
	g_free (myData.cPreviousCoverPath);
	myData.cPreviousCoverPath = myData.cCoverPath;  // remember the previous cover.
	myData.cCoverPath = NULL;
	
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
		
		if (myData.cCoverPath != NULL && cairo_dock_strings_differ (myData.cCoverPath, myData.cPreviousCoverPath))  // cover is valid and different from the previous one
		{
			_reset_cover_state ();
			if (! g_file_test (myData.cCoverPath, G_FILE_TEST_EXISTS))  // file does not exist, re-try a few times.
			{
				myData.iSidCheckCover = g_timeout_add_seconds (1, (GSourceFunc)_check_cover_file_exists, NULL);
			}
			else  // file exists, check its size until it's constant
			{
				myData.iSidCheckCover = g_timeout_add_seconds (1, (GSourceFunc)_check_cover_file_size, NULL);
			}
		}
	}
	else  // no data from the service, search by ourselves.
	{
		myData.cCoverPath = _find_cover_in_common_dirs ();
		if (myData.cCoverPath != NULL && cairo_dock_strings_differ (myData.cCoverPath, myData.cPreviousCoverPath))  // cover is valid and different from the previous one.
		{
			_reset_cover_state ();
			if (g_file_test (myData.cCoverPath, G_FILE_TEST_EXISTS))  // cover is already persent on the disk -> check it
			{
				myData.iSidCheckCover = g_timeout_add_seconds (1, (GSourceFunc)_check_cover_file_size, NULL);
			}
			else  // cover is not yet on the disk (cache) -> download it.
			{
				if (myConfig.bDownload)
				{
					cd_musicplayer_dl_cover ();
				}
			}
		}
	}
}


static void _get_cover_async (CDSharedMemory *pSharedMemory)
{
	pSharedMemory->bSuccess = cd_amazon_dl_cover (pSharedMemory->cArtist, pSharedMemory->cAlbum, pSharedMemory->cPlayingUri, pSharedMemory->cLocalPath);
}
static gboolean _on_got_cover (CDSharedMemory *pSharedMemory)
{
	CD_APPLET_ENTER;
	if (pSharedMemory->bSuccess)
	{
		myData.cover_exist = TRUE;
		cd_musiplayer_apply_cover ();
	}
	cairo_dock_discard_task (myData.pCoverTask);
	myData.pCoverTask = NULL;
	CD_APPLET_LEAVE (FALSE);
}
static void _free_shared_memory (CDSharedMemory *pSharedMemory)
{
	g_free (pSharedMemory->cArtist);
	g_free (pSharedMemory->cAlbum);
	g_free (pSharedMemory->cPlayingUri);
	g_free (pSharedMemory->cLocalPath);
	g_free (pSharedMemory);
}
static void cd_musicplayer_dl_cover (void)
{
	cd_debug ("MP-COVER - %s (%s, %s, %s)", __func__, myData.cArtist, myData.cAlbum, myData.cPlayingUri);
	
	if (myData.pCoverTask != NULL)
	{
		cairo_dock_discard_task (myData.pCoverTask);
		myData.pCoverTask = NULL;
	}
	
	CDSharedMemory *pSharedMemory = g_new0 (CDSharedMemory, 1);
	pSharedMemory->cArtist = g_strdup (myData.cArtist);
	pSharedMemory->cAlbum = g_strdup (myData.cAlbum);
	pSharedMemory->cPlayingUri = g_strdup (myData.cPlayingUri);
	pSharedMemory->cLocalPath = g_strdup (myData.cCoverPath);
	
	myData.pCoverTask = cairo_dock_new_task_full (0,
		(CairoDockGetDataAsyncFunc) _get_cover_async,
		(CairoDockUpdateSyncFunc) _on_got_cover,
		(GFreeFunc) _free_shared_memory,
		pSharedMemory);
	cairo_dock_launch_task (myData.pCoverTask);
}

/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

******************************************************************************/

#include <glib/gstdio.h>
#include "applet-amazon.h"
#include "applet-draw.h"
#include "applet-cover.h"

/*gchar *cd_check_musicPlayer_cover_exists (gchar *cURI, MySupportedPlayers iSMP) {
	gchar **cCleanURI;
	gchar **cSplitedURI;
	gint cpt=0;

	if( !cURI ) return NULL;
	
	switch (iSMP) {
		case MP_AMAROK1 :
			cCleanURI = g_strsplit (cURI,"@",0);
			cSplitedURI = g_strsplit (cCleanURI[1],".",0);
			if (g_strcasecmp(cSplitedURI[0],"nocover")==0) {
				g_strfreev (cCleanURI);
				g_strfreev (cSplitedURI);
				myData.iCheckIter = 0;
				
				if (myData.iPlayingStatus == PLAYER_PLAYING)
					g_timeout_add (1000, (GSourceFunc) cd_download_musicPlayer_cover, (gpointer) NULL);
				return NULL;
			}
			g_strfreev (cCleanURI);
			g_strfreev (cSplitedURI);
		break;
		
		case MP_EXAILE :
			cCleanURI = g_strsplit (cURI,"/",0);
			while (cCleanURI[cpt]!=NULL) 
				cpt++;
			cSplitedURI = g_strsplit (cCleanURI[cpt-1],".",0);
			if (g_strcasecmp(cSplitedURI[0],"nocover")==0) {
				g_strfreev (cCleanURI);
				g_strfreev (cSplitedURI);
				myData.iCheckIter = 0;
				
				if (myData.iPlayingStatus == PLAYER_PLAYING)
					g_timeout_add (1000, (GSourceFunc) cd_download_musicPlayer_cover, (gpointer) NULL);
				return NULL;
			}
			g_strfreev (cCleanURI);
			g_strfreev (cSplitedURI);
		break;
		
		default:
			return cURI;
		break;
	}
	return cURI;
}*/


void cd_musicplayer_get_cover_path (const gchar *cGivenCoverPath, gboolean bHandleCover)  // bHandleCover permet de ne pas regarder dans le cache ou dl la couverture, pour le cas ou le lecteur ne refilerait une adresse qu'au bout d'un certain temps. Dans ce cas-la, on fera l'operation 2 fois en laissant une tempo de ~1s, et on ne gerera la couverture nous-memes que la 2eme fois si le lecteur ne nous a toujours rien refile.
{
	g_free (myData.cPreviousCoverPath);
	myData.cPreviousCoverPath = myData.cCoverPath;  // on memorise la precedente couverture.
	myData.cCoverPath = NULL;
	myData.bCoverNeedsTest = FALSE;
	
	if (cGivenCoverPath != NULL)  // le lecteur nous donne une adresse, eventuellement distante.
	{
		const gchar *cString = cGivenCoverPath;
		g_print ("MP : le lecteur nous a refile cette adresse : %s\n", cString);
		
		if (strncmp(cString, "http://", 7) == 0)  // fichier distant, on decide de le telecharger nous-memes.
		{
			g_print ("MP : Le fichier est distant\n");
			
			if (myData.pCurrentHandeler->cCoverDir)
			{
				myData.cCoverPath = g_strdup_printf("%s/%s - %s.jpg", myData.pCurrentHandeler->cCoverDir, myData.cArtist, myData.cAlbum);
			}
			else  // le lecteur n'a pas de cache, on utilise le notre.
			{
				myData.cCoverPath = g_strdup_printf("%s/musicplayer/%s - %s.jpg", g_cCairoDockDataDir, myData.cArtist, myData.cAlbum);
			}
			
			if (! g_file_test (myData.cCoverPath, G_FILE_TEST_EXISTS))  // si le lecteur n'est pas deja en train de le telecharger.
			{
				gchar *cCommand = g_strdup_printf ("wget -O \"%s\" \"%s\"",
					myData.cCoverPath,
					cString);
				g_spawn_command_line_async (cCommand, NULL);
				g_free (cCommand);
			}
			myData.bCoverNeedsTest = TRUE;  // on testera sur sa taille.
		}
		else if (strncmp (cString, "file://", 7) == 0)  // URI locale, on l'accepte sans verifier.
		{
			myData.cCoverPath = g_filename_from_uri (cString, NULL, NULL);
		}
		else if (*cString == '/')  // fichier local, on l'accepte sans verifier.
		{
			myData.cCoverPath = g_strdup (cString);
		}
	}
	else if (bHandleCover)  // le lecteur ne nous a rien file => on va etablir une adresse locale qu'on testera dans le update_icon.
	{
		g_print ("MP : Pas d'adresse de la part du lecteur ... on regarde si elle n'existe pas deja en local\n");
		gchar *cSongPath = (myData.cPlayingUri ? g_filename_from_uri (myData.cPlayingUri, NULL, NULL) : NULL);  // on teste d'abord dans le repertoire de la chanson.
		if (cSongPath != NULL)  // c'est une chanson en local.
		{
			gchar *cSongDir = g_path_get_dirname (cSongPath);
			g_free (cSongPath);
			
			myData.cCoverPath = g_strdup_printf ("%s/%s - %s.jpg", cSongDir, myData.cArtist, myData.cAlbum);
			cd_debug ("  test de %s", myData.cCoverPath);
			if (! g_file_test (myData.cCoverPath, G_FILE_TEST_EXISTS))
			{
				g_free (myData.cCoverPath);
				myData.cCoverPath = g_strdup_printf ("%s/cover.jpg", cSongDir);
				cd_debug ("  test de %s", myData.cCoverPath);
				if (! g_file_test (myData.cCoverPath, G_FILE_TEST_EXISTS))
				{
					g_free (myData.cCoverPath);
					myData.cCoverPath = g_strdup_printf ("%s/Cover.jpg", cSongDir);
					cd_debug ("  test de %s", myData.cCoverPath);
					if (! g_file_test (myData.cCoverPath, G_FILE_TEST_EXISTS))
					{
						g_free (myData.cCoverPath);
						myData.cCoverPath = g_strdup_printf ("%s/cover.jpeg", cSongDir);
						cd_debug ("  test de %s", myData.cCoverPath);
						if (! g_file_test (myData.cCoverPath, G_FILE_TEST_EXISTS))
						{
							g_free (myData.cCoverPath);
							myData.cCoverPath = g_strdup_printf ("%s/album.jpg", cSongDir);
							cd_debug ("  test de %s", myData.cCoverPath);
							if (! g_file_test (myData.cCoverPath, G_FILE_TEST_EXISTS))
							{
								g_free (myData.cCoverPath);
								myData.cCoverPath = g_strdup_printf ("%s/albumart.jpg", cSongDir);
								cd_debug ("  test de %s", myData.cCoverPath);
								if (! g_file_test (myData.cCoverPath, G_FILE_TEST_EXISTS))
								{
									g_free (myData.cCoverPath);
									myData.cCoverPath = g_strdup_printf ("%s/folder.jpg", cSongDir);
									cd_debug ("  test de %s", myData.cCoverPath);
									if (! g_file_test (myData.cCoverPath, G_FILE_TEST_EXISTS))
									{
										g_free (myData.cCoverPath);
										myData.cCoverPath = NULL;
									}
								}
							}
						}
					}
				}
			}
			g_free (cSongDir);
		}
		
		if (myData.cCoverPath == NULL)  // on regarde maintenant dans le cache.
		{
			cd_debug("MP : On regarde dans le répertoire cache");
			
			if (myData.pCurrentHandeler->cCoverDir)
			{
				myData.cCoverPath = g_strdup_printf("%s/%s - %s.jpg", myData.pCurrentHandeler->cCoverDir, myData.cArtist, myData.cAlbum);
				myData.bCoverNeedsTest = TRUE;  // on testera sur sa taille.
			}
			else  // le lecteur n'a pas de cache, on utilise le notre.
			{
				myData.cCoverPath = g_strdup_printf ("%s/musicplayer/%s - %s.jpg", g_cCairoDockDataDir, myData.cArtist, myData.cAlbum);
			}
			
			if (! g_file_test (myData.cCoverPath, G_FILE_TEST_EXISTS))  // la couverture n'est pas en cache, on la telecharge nous-memes.
			{
				cd_musicplayer_dl_cover ();
				myData.bCoverNeedsTest = TRUE;  // on testera sur sa taille.
			}
		}
	}
	
	g_print ("MP :  cCoverPath <- %s (%d)\n", myData.cCoverPath, bHandleCover);

	if (myData.cCoverPath == NULL || cairo_dock_strings_differ (myData.cPreviousCoverPath, myData.cCoverPath))  // la couverture a change, son existence est incertaine et il faudra charger la surface/texture avec une transition. Sinon son existence ne change pas et il n'y a rien a faire.
	{
		g_print (" c'est une nouvelle couverture (%s -> %s)\n", myData.cPreviousCoverPath, myData.cCoverPath);
		myData.cover_exist = FALSE;
	}
}



static gboolean _check_xml_file (gpointer data)
{
	if (g_file_test (myData.cCurrentXmlFile, G_FILE_TEST_EXISTS))
	{
		cd_message ("MP : le fichier XML '%s' est present sur le disque", myData.cCurrentXmlFile);
		if (cd_musicplayer_check_size_is_constant (myData.cCurrentXmlFile))
		{
			cd_message ("MP : sa taille est constante (%d)", myData.iCurrentFileSize);
			
			// on lance le dl du fichier image.
			g_print ("avant extraction : %s / %s\n", myData.cArtist, myData.cAlbum);
			gchar *cURL = cd_extract_url_from_xml_file (myData.cCurrentXmlFile, &myData.cArtist, &myData.cAlbum, &myData.cTitle);
			g_print ("apres extraction : %s / %s\n", myData.cArtist, myData.cAlbum);
			g_print ("on s'apprete a telecharger la pochette : %s -> %s\n", cURL, myData.cCoverPath);
			if (g_strstr_len (myData.cCoverPath, -1, "(null)") != NULL && myData.cArtist && myData.cAlbum)
			{
				g_print ("on corrige cCoverPath\n");
				g_free (myData.cCoverPath);
				if (myData.pCurrentHandeler->cCoverDir)
				{
					myData.cCoverPath = g_strdup_printf("%s/%s - %s.jpg", myData.pCurrentHandeler->cCoverDir, myData.cArtist, myData.cAlbum);
				}
				else  // le lecteur n'a pas de cache, on utilise le notre.
				{
					myData.cCoverPath = g_strdup_printf ("%s/musicplayer/%s - %s.jpg", g_cCairoDockDataDir, myData.cArtist, myData.cAlbum);
				}
			}
			cd_download_missing_cover (cURL);
			g_free (cURL);
			
			// on teste en boucle sur la taille du fichier image.
			myData.iCurrentFileSize = 0;
			myData.iNbCheckFile = 0;
			myData.iSidCheckCover = g_timeout_add_seconds (1, (GSourceFunc) cd_musiplayer_set_cover_if_present, GINT_TO_POINTER (TRUE));  // TRUE <=> tester la taille contante.
			
			// on quitte la boucle de test du fichier XML.
			///g_remove (myData.cCurrentXmlFile);
			g_free (myData.cCurrentXmlFile);
			myData.cCurrentXmlFile = NULL;
			myData.iSidCheckXmlFile = 0;
			return FALSE;
		}
	}
	else  // on continue a tester.
	{
		if (myData.iNbCheckFile > 12)  // on abandonne au bout de 3s.
		{
			g_print ("on abandonne le XML\n");
			g_remove (myData.cCurrentXmlFile);
			g_free (myData.cCurrentXmlFile);
			myData.cCurrentXmlFile = NULL;
			myData.iSidCheckXmlFile = 0;
			return FALSE;
		}
	}
	return TRUE;
}
void cd_musicplayer_dl_cover (void)
{
	g_print ("%s (%s, %s, %s)\n", __func__, myData.cArtist, myData.cAlbum, myData.cPlayingUri);
	// on oublie ce qu'on etait en train de recuperer.
	g_free (myData.cCurrentXmlFile);
	myData.cCurrentXmlFile = NULL;
	
	// lance le dl du fichier XML.
	myData.cCurrentXmlFile = cd_get_xml_file (myData.cArtist, myData.cAlbum, myData.cPlayingUri);
	
	// on teste en boucle sur la taille du fichier XML.
	myData.iCurrentFileSize = 0;
	myData.iNbCheckFile = 0;
	if (myData.iSidCheckXmlFile == 0)
	{
		if (myData.cCurrentXmlFile != NULL)
			myData.iSidCheckXmlFile = g_timeout_add (250, (GSourceFunc) _check_xml_file, NULL);
	}
	else if (myData.cCurrentXmlFile == NULL)
	{
		g_source_remove (myData.iSidCheckXmlFile);
		myData.iSidCheckXmlFile = 0;
	}
}

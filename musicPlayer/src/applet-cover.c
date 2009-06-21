#include "applet-cover.h"
#include "applet-amazon.h"

extern gboolean bCurrentlyDownloading, bCurrentlyDownloadingXML;
static gchar *cImageURL = NULL;
static CairoDockTask *pTask = NULL;

gboolean _cd_proceed_download_cover (gpointer p) {

    // Si on ne télécharge pas, on arrête la boucle direct
    if (!myConfig.bDownload) {
        cairo_dock_stop_task (pTask);
        return FALSE;
    }

    // Avant tout, on dl le xml
    if (!bCurrentlyDownloadingXML && !bCurrentlyDownloading) 
        cd_get_xml_file(myData.cArtist,myData.cAlbum);

    // Quand on a le xml, on dl la pochette
    if (g_file_test (DEFAULT_XML_LOCATION, G_FILE_TEST_EXISTS) && !bCurrentlyDownloading) {
        if (cImageURL)
            g_free(cImageURL);
        cImageURL = NULL;
        cd_stream_file(DEFAULT_XML_LOCATION,&cImageURL);
        cd_debug ("URL : %s",cImageURL);
        if (cImageURL) {
            cd_download_missing_cover(cImageURL,DEFAULT_DOWNLOADED_IMAGE_LOCATION);
            bCurrentlyDownloadingXML = FALSE;
        } else {
            bCurrentlyDownloadingXML = FALSE;
            bCurrentlyDownloading = FALSE;
            cd_debug ("Téléchargement impossible\n");
            cairo_dock_stop_task (pTask);
            return FALSE;
        }
    }

    // Quand on a la pochette, on l'affiche et on stoppe la boucle
    if (g_file_test (DEFAULT_DOWNLOADED_IMAGE_LOCATION, G_FILE_TEST_EXISTS)) {
        bCurrentlyDownloadingXML = FALSE;
        bCurrentlyDownloading = FALSE;
        CD_APPLET_SET_IMAGE_ON_MY_ICON (DEFAULT_DOWNLOADED_IMAGE_LOCATION);
        return FALSE;
    }
    
    return TRUE;
}

gboolean cd_download_musicPlayer_cover (gpointer data) {
	myData.iCheckIter++;
	if (myData.iCheckIter > myConfig.iTimeToWait && myConfig.bDownload) {
		if (pTask) {
            if (cairo_dock_task_is_running(pTask))
                cairo_dock_stop_task(pTask);
            if (cairo_dock_task_is_active(pTask))
                cairo_dock_free_task(pTask);
        }
        pTask = cairo_dock_new_task (2 *1000, NULL, (CairoDockUpdateSyncFunc) _cd_proceed_download_cover, NULL);
        
        cairo_dock_launch_task (pTask);
		return FALSE;
	}
	return TRUE;
}

gchar *cd_check_musicPlayer_cover_exists (gchar *cURI, MySupportedPlayers iSMP) {
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
				
				if (myData.pPlayingStatus == PLAYER_PLAYING)
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
				
				if (myData.pPlayingStatus == PLAYER_PLAYING)
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
}

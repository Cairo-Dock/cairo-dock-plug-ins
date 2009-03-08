#include "applet-cover.h"
#include "applet-amazon.h"

gboolean _cd_proceed_download_cover (gpointer p) {
	if (myConfig.bDownload && cd_get_xml_file(myData.cArtist,myData.cAlbum)) {
		gchar *cImageURL = NULL;
		cd_stream_file(DEFAULT_XML_LOCATION,&cImageURL);
		if (cImageURL && cd_download_missing_cover(cImageURL,DEFAULT_DOWNLOADED_IMAGE_LOCATION))
				CD_APPLET_SET_IMAGE_ON_MY_ICON (DEFAULT_DOWNLOADED_IMAGE_LOCATION);
		cd_debug ("URL : %s\n",cImageURL);
		g_print ("URL : %s\n%s\n",cImageURL,myData.cCoverPath);
	} else
		cd_debug ("Téléchargement impossible\n");
	return FALSE;
}

gboolean cd_download_musicPlayer_cover (gpointer data) {
	myData.iCheckIter++;
	if (myData.iCheckIter > myConfig.iTimeToWait) {
		g_timeout_add (0,(GSourceFunc) _cd_proceed_download_cover, NULL);
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
	}
	return cURI;
}

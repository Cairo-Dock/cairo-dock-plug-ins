#include "applet-amazon.h"
#include "applet-struct.h"

static gboolean flag, found;

gboolean bCurrentlyDownloading = FALSE;
gboolean bCurrentlyDownloadingXML = FALSE;

gchar *URL;
gchar *TAB_IMAGE_SIZES[] = {"MediumImage","LargeImage"};

/**
 * Lit les noeuds du fichier en cours de parsage
 * @param reader le noeud courrant
 * @param imageSize Noeud que l'on cherche
 */
static void cd_process_node (xmlTextReaderPtr reader, gchar **cValue) {
	const xmlChar *name, *value;

	name = xmlTextReaderConstName(reader);
	if (name == NULL)
		name = BAD_CAST "--";

		value = xmlTextReaderConstValue(reader);
		if ((strcmp(name,TAB_IMAGE_SIZES[myConfig.iImagesSize])==0 || flag) && !found) {
			//printf("node: %s ", name);
		if (value != NULL) {
			cd_message ("%s\n", value);
			found=TRUE;
			*cValue=g_strdup(value);
		}
		if (strcmp(name,"#text")!=0) {
			flag=TRUE;
		} else {
			flag=FALSE;
		}
	}
}

/**
 * Parse le fichier XML passé en argument
 * à la recherche de l'URL de la pochette
 * @param filename URI du fichier à lire
 * @param imageSize Taille de l'image que l'on souhaite
 */
void cd_stream_file(const char *filename, gchar **cValue) {
	/*
	* this initialize the library and check potential ABI mismatches
	* between the version it was compiled for and the actual shared
	* library used.
	*/
	LIBXML_TEST_VERSION
	
	int ret;
	
	xmlTextReaderPtr reader;
	flag = FALSE;
	found=FALSE;

	reader = xmlReaderForFile(filename, NULL, 0);
	if (reader != NULL) {
		ret = xmlTextReaderRead(reader);
		while (ret == 1) {
			cd_process_node (reader,cValue);
			ret = xmlTextReaderRead(reader);
		}
		xmlFreeTextReader(reader);
		if (ret != 0) {
			cd_warning ("%s : failed to parse\n", filename);
		}
	} else {
		cd_warning ("Unable to open %s\n", filename);
	}
	/*
	 * Cleanup function for the XML library.
	 */
	xmlCleanupParser();
	/*
	 * this is to debug memory for regression tests
	 */
	xmlMemoryDump();
}

gboolean cd_get_xml_file (const gchar *artist, const gchar *album) {
    if (g_strcasecmp("Unknown",artist)==0 || g_strcasecmp("Unknown",album)==0)
        return FALSE;
        
    gchar *cFileToDownload = g_strdup_printf("%s%s%s&Artist=%s&Album=%s",AMAZON_API_URL_1,LICENCE_KEY,AMAZON_API_URL_2,artist,album);
    gchar *cTmpFilePath = g_strdup (DEFAULT_XML_LOCATION);
    
    gchar *cCommand = g_strdup_printf ("rm %s", DEFAULT_DOWNLOADED_IMAGE_LOCATION);
    if (!system (cCommand)) return FALSE;
    g_free (cCommand);
    cCommand = g_strdup_printf ("wget \"%s\" -O '%s-bis' -t 2 -T 2 > /dev/null 2>&1 && mv %s-bis %s", cFileToDownload, cTmpFilePath, cTmpFilePath, cTmpFilePath);
    cd_debug ("%s\n",cCommand);
    //system (cCommand);
    cd_launch_command (cCommand);
    bCurrentlyDownloadingXML = TRUE;
    g_free (cCommand);
    g_free (cTmpFilePath);
    g_free (cFileToDownload);
    return TRUE;
}

gboolean cd_download_missing_cover (const gchar *cURL, const gchar *cDestPath) {
    gchar *cCommand = g_strdup_printf ("wget \"%s\" -O '%s-bis' -t 2 -T 2 > /dev/null 2>&1 && mv %s-bis %s", cURL, cDestPath, cDestPath, cDestPath);
    cd_debug ("%s\n",cCommand);
    //system (cCommand);
    cd_launch_command (cCommand);
    bCurrentlyDownloading = TRUE;
    g_free (cCommand);
    cCommand = g_strdup_printf ("rm %s", DEFAULT_XML_LOCATION);
    if (!system (cCommand)) return FALSE;
    g_free (cCommand);
    return TRUE;
}

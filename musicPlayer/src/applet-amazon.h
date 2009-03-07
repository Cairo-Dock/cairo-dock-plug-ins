
#ifndef __CD_AMAZON__
#define  __CD_AMAZON__

#include <cairo-dock.h>
#include <libxml/xmlreader.h>

G_BEGIN_DECLS

#define LICENCE_KEY "0C3430YZ2MVJKQ4JEKG2"
#define AMAZON_API_URL_1 "http://ecs.amazonaws.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="
#define AMAZON_API_URL_2 "&AssociateTag=webservices-20&ResponseGroup=Images,ItemAttributes&Operation=ItemSearch&ItemSearch.Shared.SearchIndex=Music"
#define DEFAULT_XML_LOCATION "/tmp/cd_amazon.xml"
#define DEFAULT_DOWNLOADED_IMAGE_LOCATION "/tmp/default.jpg"

/**
 * Lit les noeuds du fichier en cours de parsage
 * @param reader le noeud courrant
 * @param imageSize Noeud que l'on cherche
 */
static void cd_process_node (xmlTextReaderPtr reader, gchar **cValue);

/**
 * Parse le fichier XML passé en argument
 * à la recherche de l'URL de la pochette
 * @param filename URI du fichier à lire
 * @param imageSize Taille de l'image que l'on souhaite
 */
void cd_stream_file(const char *filename, gchar **cValue);

/**
 * Recupere le fichier xml sur amazon.
 * @param artist Nom de l'artiste.
 * @param album Nom de l'album.
 * @return succes du telechargement.
 */
gboolean cd_get_xml_file (const gchar *artist, const gchar *album);

/**
 * Recupere la pochette.
 * @param cURL URL de la pochette.
 * @param cDestPath Ou en enregistre la pochette telechargee.
 * @return succes du telechargement.
 */
gboolean cd_download_missing_cover (const gchar *cURL, const gchar *cDestPath);

G_END_DECLS
#endif

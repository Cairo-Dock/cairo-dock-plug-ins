
#ifndef __CD_AMAZON__
#define  __CD_AMAZON__

#include <cairo-dock.h>
#include <libxml/xmlreader.h>

G_BEGIN_DECLS

/**
 * Parse le fichier XML passé en argument
 * à la recherche de l'URL de la pochette
 * @param filename URI du fichier à lire
 * @param imageSize Taille de l'image que l'on souhaite
 */
gchar *cd_extract_url_from_xml_file (const gchar *filename);

/**
 * Recupere le fichier xml sur amazon.
 * @param artist Nom de l'artiste.
 * @param album Nom de l'album.
 * @return succes du telechargement.
 */
gchar *cd_get_xml_file (const gchar *artist, const gchar *album);

/**
 * Recupere la pochette.
 * @param cURL URL de la pochette.
 * @param cDestPath Ou en enregistre la pochette telechargee.
 * @return succes du telechargement.
 */
void cd_download_missing_cover (const gchar *cURL);


G_END_DECLS
#endif

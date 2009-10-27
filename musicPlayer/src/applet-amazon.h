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
gchar *cd_extract_url_from_xml_file (const gchar *filename, gchar **artist, gchar **album, gchar **title);

/**
 * Recupere le fichier xml sur amazon.
 * @param artist Nom de l'artiste.
 * @param album Nom de l'album.
 * @return succes du telechargement.
 */
gchar *cd_get_xml_file (const gchar *artist, const gchar *album, const gchar *cUri);


G_END_DECLS
#endif

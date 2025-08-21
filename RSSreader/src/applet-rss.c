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

#include <glib.h>
#include <glib/gprintf.h>

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-rss.h"

const gchar *cExtendedAscii[256-32] = {" ", "!", "\"", "#", "$", "%", "&", "’", "(", ")", "*", "+", ",", "-", ".", "/", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?", "@", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "[", "\"", "]", "^", "_", "`", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "{", "|", "}", "~", " ", "€", "�", "‚", "ƒ", "„", "…", "†", "‡", "ˆ", "‰", "Š", "<", "Œ", "�", "Ž", "�", "�", "‘", "’", "“", "”", "•", "–", "—", "˜", "™", "š", "›", "œ", "�", "ž", "Ÿ", " ", "¡", "¢", "£", "¤", "¥", "¦", "§", "¨", "©", "ª", "«", "¬", " ", "®", "¯", "°", "±", "²", "³", "´", "µ", "¶", "·", "¸", "¹", "º", "»", "¼", "½", "¾", "¿", "À", "Á", "Â", "Ã", "Ä", "Å", "Æ", "Ç", "È", "É", "Ê", "Ë", "Ì", "Í", "Î", "Ï", "Ð", "Ñ", "Ò", "Ó", "Ô", "Õ", "Ö", "×", "Ø", "Ù", "Ú", "Û", "Ü", "Ý", "Þ", "ß", "à", "á", "â", "ã", "ä", "å", "æ", "ç", "è", "é", "ê", "ë", "ì", "í", "î", "ï", "ð", "ñ", "ò", "ó", "ô", "õ", "ö", "÷", "ø", "ù", "ú", "û", "ü", "ý", "þ", "ÿ"};

/* Insere des retours chariots dans une chaine de caracteres de facon a la faire tenir dans un rectangle donne.
 */
void cd_rssreader_cut_line (gchar *cLine, PangoLayout *pLayout, int iMaxWidth)
{
	cd_debug ("%s (%s)", __func__, cLine);
	// on convertit les caracteres internet.
	gchar *str=cLine, *amp;
	do
	{
		amp = strchr (str, '&');
		if (!amp)
			break;
		if (amp[1] == '#' && g_ascii_isdigit (amp[2]) && g_ascii_isdigit (amp[3]) && g_ascii_isdigit (amp[4]) && amp[5] == ';')  // &#039;
		{
			int i = atoi (amp+2) - 32;
			cd_debug ("%d", i);
			
			if (i >= 0 && i < 256 - 32)
			{
				cd_debug ("%d -> %s", i, cExtendedAscii[i]);
				strcpy (amp, cExtendedAscii[i]);
				strcpy (amp+strlen (cExtendedAscii[i]), amp+6);
			}
		}
		str = amp + 1;
	} while (1);
	
	// on insere des retours chariot pour tenir dans la largeur donnee.
	PangoRectangle ink, log;
	gchar *sp, *last_sp=NULL;
	double w;
	int iNbLines = 0;
	
	str = cLine;
	while (*str == ' ')  // on saute les espaces en debut de ligne.
		str ++;
	
	sp = str;
	do
	{
		sp = strchr (sp+1, ' ');  // on trouve l'espace suivant.
		if (!sp)  // plus d'espace, on quitte.
			break ;
		
		*sp = '\0';  // on coupe a cet espace.
		pango_layout_set_text (pLayout, str, -1);  // on regarde la taille de str a sp.
		pango_layout_get_pixel_extents (pLayout, &ink, &log);
		//g_print ("%s => w:%d/%d, x:%d/%d\n", str, log.width, ink.width, log.x, ink.x);
		w = log.width + log.x;
		
		if (w > iMaxWidth)  // on deborde.
		{
			if (last_sp != NULL)  // on coupe au dernier espace connu.
			{
				*sp = ' ';  // on remet l'espace.
				*last_sp = '\n';  // on coupe.
				iNbLines ++;
				str = last_sp + 1;  // on place le debut de ligne apres la coupure.
			}
			else  // aucun espace, c'est un mot entier.
			{
				*sp = '\n';  // on coupe apres le mot.
				iNbLines ++;
				str = sp + 1;  // on place le debut de ligne apres la coupure.
			}
			
			while (*str == ' ')  // on saute les espaces en debut de ligne.
				str ++;
			sp = str;
			last_sp = NULL;
		}
		else  // ca rentre.
		{
			*sp = ' ';  // on remet l'espace.
			last_sp = sp;  // on memorise la derniere cesure qui fait tenir la ligne en largeur.
			sp ++;  // on se place apres.
			while (*sp == ' ')  // on saute tous les espaces.
				sp ++;
		}
	} while (sp);
	
	// dernier mot.
	pango_layout_set_text (pLayout, str, -1);  // on regarde la taille de str a sp.
	pango_layout_get_pixel_extents (pLayout, &ink, &log);
	w = log.width + log.x;
	if (w > iMaxWidth)  // on deborde.
	{
		if (last_sp != NULL)  // on coupe au dernier espace connu.
			*last_sp = '\n';
	}
}


static void _free_item (CDRssItem *pItem)
{
	if (pItem == NULL)
		return;
	
	g_free (pItem->cTitle);
	g_free (pItem->cDescription);
	g_free (pItem->cLink);
	g_free (pItem->cDate);
	g_free (pItem);
}
void cd_rssreader_free_item_list (GldiModuleInstance *myApplet)
{
	if (myData.pItemList == NULL)
		return;
	CDRssItem *pItem;
	GList *it;
	for (it = myData.pItemList; it != NULL; it = it->next)
	{
		pItem = it->data;
		_free_item (pItem);
	}
	g_list_free (myData.pItemList);
	myData.pItemList = NULL;
	myData.iFirstDisplayedItem = 0;
	
	gldi_object_unref (GLDI_OBJECT(myData.pDialog));  // un peu bourrin mais bon ... on pourrait le reafficher s'il etait present ...
	myData.pDialog = NULL;
}


static void _get_feeds (CDSharedMemory *pSharedMemory)
{
	if (pSharedMemory->cUrl == NULL)
		return ;

	gchar *cUrlWithLoginPwd = NULL;
	if (pSharedMemory->cUrlLogin && pSharedMemory->cUrlPassword
	&& *pSharedMemory->cUrlLogin != '\0' && *pSharedMemory->cUrlPassword != '\0')
	{
		// An URL is composed of that: "protocol://login:password@server/path"
		// so look for the "://" string and insert "login:password@" at that place
		gchar *location = g_strstr_len(pSharedMemory->cUrl, 10, "://");
		if( location )
		{
			gsize length_first_part = location - pSharedMemory->cUrl + 3;
			if( length_first_part > 0 )
			{
				gchar *first_part = g_strndup(pSharedMemory->cUrl, length_first_part);
				cUrlWithLoginPwd = g_strdup_printf("%s%s:%s@%s", first_part, pSharedMemory->cUrlLogin, pSharedMemory->cUrlPassword, location+3);
				g_free(first_part);
			}
		}
	}
	
	pSharedMemory->cTaskBridge = cairo_dock_get_url_data (cUrlWithLoginPwd ? cUrlWithLoginPwd : pSharedMemory->cUrl, NULL);
	g_free (cUrlWithLoginPwd);
}

static GList * _parse_rss_item (xmlNodePtr node, CDRssItem *pItem, GList *pItemList)
{
	xmlChar *content;
	xmlNodePtr item;
	for (item = node->children; item != NULL; item = item->next)
	{
		cd_debug ("  + item: %s", item->name);
		if (xmlStrcmp (item->name, BAD_CAST "item") == 0)  // c'est un nouvel item.
		{
			CDRssItem *pNewItem = g_new0 (CDRssItem, 1);
			pItemList = g_list_prepend (pItemList, pNewItem);
			
			pItemList = _parse_rss_item (item, pNewItem, pItemList);
		}
		else if (xmlStrcmp (item->name, BAD_CAST "title") == 0)  // c'est le titre.
		{
			if (pItem->cTitle == NULL)  // cas du titre du flux force a une valeur par l'utilisateur.
			{
				content = xmlNodeGetContent (item);
				if (content != NULL)
				{
					// remove leading and trailing carriage returns.
					gchar *str = (gchar *) content;
					while (*str == '\n') str ++;
					int n = strlen(str);
					while (str[n-1] == '\n') str[--n] = '\0';
					
					pItem->cTitle = g_strdup (str);
					
					xmlFree (content);
				}
			}
			//g_print ("   + titre : '%s'\n", pItem->cTitle);
		}
		else if (xmlStrcmp (item->name, BAD_CAST "description") == 0)  // c'est la description.
		{
			content = xmlNodeGetContent (item);
			pItem->cDescription = g_strdup ((gchar *)content);
			xmlFree (content);
			
			// on elimine les balises integrees a la description.
			gchar *str = pItem->cDescription, *balise, *balise2;
			do
			{
				balise2 = NULL;
				balise = strchr (str, '<');  // debut de balise ("<")
				if (balise)
				{
					balise2 = strchr (balise+1, '>');  // fin de balise (">")
					if (balise2)
					{
						strcpy (balise, balise2+1);
						str = balise;
					}
				}
			}
			while (balise2);
			str = pItem->cDescription;
			do
			{
				balise = g_strstr_len (str, -1, "&nbsp;");
				if (balise)
				{
					memset (balise, ' ', 6);
					str = balise+6;
				}
			}
			while (balise);
			cd_debug ("   + description : '%s'", pItem->cDescription);
		}
		else if (xmlStrcmp (item->name, BAD_CAST "link") == 0)  // c'est le lien.
		{
			content = xmlNodeGetContent (item);
			pItem->cLink = g_strdup ((gchar *)content);
			xmlFree (content);
			cd_debug ("   + link : '%s'", pItem->cLink);
		}
		else if (xmlStrcmp (item->name, BAD_CAST "pubDate") == 0 || xmlStrcmp (item->name, BAD_CAST "date") == 0)  // c'est la date (pubDate pour RSS, data pour RDF).
		{
			content = xmlNodeGetContent (item);
			pItem->cDate = g_strdup ((gchar *)content);
			xmlFree (content);
		}
		// pour recuperer l'image, on dirait qu'on a plusieurs cas, entre autre :
		// <enclosure url="http://medias.lemonde.fr/mmpub/edt/ill/2009/11/01/h_1_ill_1261356_5d02_258607.jpg" length="2514" type="image/jpeg" />  ----> bien verifier que c'est une image.
		// ou
		// <media:thumbnail url="http://www.france24.com/fr/files_fr/EN-interview-Gursel-m.jpg" />
	}
	return pItemList;
}

static GList * _parse_atom_item (xmlNodePtr node, CDRssItem *pItem, GList *pItemList, const gchar *cBaseUrl)
{
	xmlChar *content;
	xmlNodePtr item, author;
	for (item = node->children; item != NULL; item = item->next)
	{
		if (xmlStrcmp (item->name, BAD_CAST "entry") == 0)  // c'est un nouvel item.
		{
			CDRssItem *pNewItem = g_new0 (CDRssItem, 1);
			pItemList = g_list_prepend (pItemList, pNewItem);
			
			pItemList = _parse_atom_item (item, pNewItem, pItemList, cBaseUrl);
		}
		else if (xmlStrcmp (item->name, BAD_CAST "title") == 0)  // c'est le titre.
		{
			if (pItem->cTitle == NULL)  // cas du titre du flux force a une valeur par l'utilisateur.
			{
				content = xmlNodeGetContent (item);
				if (content != NULL)
				{
					// remove leading and trailing carriage returns.
					gchar *str = (gchar *)content;
					while (*str == '\n') str ++;
					int n = strlen(str);
					while (str[n-1] == '\n') str[--n] = '\0';
					
					pItem->cTitle = g_strdup (str);
					
					xmlFree (content);
				}
			}
			cd_debug ("+ title : '%s'", pItem->cTitle);
		}
		else if (xmlStrcmp (item->name, BAD_CAST "content") == 0)  // c'est la description.
		{
			xmlAttrPtr attr = xmlHasProp (item, BAD_CAST "type");
			if (attr && attr->children)
			{
				cd_debug ("   description type : %s", attr->children->content);
				if (strncmp ((gchar *)attr->children->content, "text", 4) != 0)
				{
					continue;
				}
			}
			content = xmlNodeGetContent (item);
			pItem->cDescription = g_strdup ((gchar *)content);
			xmlFree (content);
			
			// on elimine les balises integrees a la description.
			gchar *str = pItem->cDescription, *balise, *balise2;
			do
			{
				balise2 = NULL;
				balise = strchr (str, '<');  // debut de balise ("<")
				if (balise)
				{
					balise2 = strchr (balise+1, '>');  // fin de balise (">")
					if (balise2)
					{
						strcpy (balise, balise2+1);
						str = balise;
					}
				}
			}
			while (balise2);
			cd_debug ("+ description : '%s'", pItem->cDescription);
		}
		else if (xmlStrcmp (item->name, BAD_CAST "link") == 0)  // c'est le lien.
		{
			xmlAttrPtr attr = xmlHasProp (item, BAD_CAST "type");  // type="text/html" rel="alternate"
			if (attr && attr->children)
			{
				cd_debug ("   link type : %s", attr->children->content);
				if (strncmp ((gchar *)attr->children->content, "text", 4) != 0)
				{
					continue;
				}
			}
			attr = xmlHasProp (item, BAD_CAST "href");
			if (attr && attr->children)
			{
				content = xmlNodeGetContent (attr->children);
				if (strncmp ((gchar *)content, "http://", 7) == 0)
				{
					pItem->cLink = g_strdup ((gchar *)content);
				}
				else if (cBaseUrl != NULL)
				{
					pItem->cLink = g_strdup_printf ("%s%s", cBaseUrl, (gchar *)content);
				}
				xmlFree (content);
				cd_debug ("+ link : '%s'", pItem->cLink);
			}
		}
		else if (xmlStrcmp (item->name, BAD_CAST "updated") == 0)  // c'est la date.
		{
			content = xmlNodeGetContent (item);
			pItem->cDate = g_strdup ((gchar *)content);
			xmlFree (content);
			cd_debug ("+ date : '%s'", pItem->cDate);
		}
		else if (xmlStrcmp (item->name, BAD_CAST "author") == 0)  // c'est l'auteur.
		{
			for (author = item->children; author != NULL; author = author->next)
			{
				if (xmlStrcmp (author->name, BAD_CAST "name") == 0)  // c'est le nom de l'auteur.
				{
					content = xmlNodeGetContent (author);
					pItem->cAuthor = g_strdup ((gchar *)content);
					xmlFree (content);
					cd_debug ("+ author : '%s'", pItem->cAuthor);
				}
			}
		}
		// et pour l'image je ne sais pas.
	}
	return pItemList;
}

static void _insert_error_message (GldiModuleInstance *myApplet, const gchar *cErrorMessage)
{
	cd_debug ("%s (%s, %d)", __func__, cErrorMessage, myData.bError);
	CDRssItem *pItem;
	if (myData.pItemList != NULL)  // on garde la liste courante, mais on insere un message.
	{
		if (! myData.bError)  // pas encore de message d'erreur, on en insere un.
		{
			pItem = g_new0 (CDRssItem, 1);
			pItem->cTitle = g_strdup (D_("Warning: couldn't retrieve data last time we tried."));
			myData.pItemList = g_list_insert (myData.pItemList, pItem, 1);
		}
	}
	else  // aucune liste : c'est la 1ere recuperation => on met le titre si possible, suivi du message d'erreur.
	{
		pItem = g_new0 (CDRssItem, 1);
		myData.pItemList = g_list_prepend (myData.pItemList, pItem);
		if (myConfig.cUserTitle != NULL && myConfig.cUrl != NULL)  // si le titre est connu on l'utilise (si aucun URL n'est defini ce n'est pas pertinent par contre).
		{
			pItem->cTitle = g_strdup (myConfig.cUserTitle);
			pItem = g_new0 (CDRssItem, 1);
			myData.pItemList = g_list_append (myData.pItemList, pItem);
		}
		pItem->cTitle = g_strdup (cErrorMessage);
	}
	
	myData.bError = TRUE;
}

static gboolean _update_from_feeds (CDSharedMemory *pSharedMemory)
{
	GldiModuleInstance *myApplet = pSharedMemory->pApplet;
	CD_APPLET_ENTER;
	if (! myData.bInit)  // pas encore initialise, on vire le message d'attente.
	{
		cd_rssreader_free_item_list (myApplet);
		myData.pItemList = NULL;
		myData.bInit = TRUE;
	}
	
	// On parse le flux XML.
	if (pSharedMemory->cTaskBridge == NULL || *pSharedMemory->cTaskBridge == '\0')
	{
		cd_warning ("RSSresader : no data");
		const gchar *cErrorMessage = (myConfig.cUrl == NULL ?
			D_("No URL is defined.") :
			D_("No data (no connection?)"));
		_insert_error_message (myApplet, cErrorMessage);
		if (myDesklet)
		{
			cd_applet_update_my_icon (myApplet);
		}
		myData.bUpdateIsManual = FALSE;
		
		if (myData.pTask->iPeriod > 20)
		{
			cd_message ("no data, will re-try in 20s");
			gldi_task_change_frequency (myData.pTask, 20);  // on re-essaiera dans 20s.
		}
		
		CD_APPLET_LEAVE (TRUE);
	}
	
	if (myData.pTask->iPeriod != myConfig.iRefreshTime)
	{
		cd_message ("revert to normal frequency");
		gldi_task_change_frequency (myData.pTask, myConfig.iRefreshTime);
	}
	
	//g_print (" --> RSS: '%s'\n", myData.cTaskBridge);
	xmlDocPtr doc = xmlParseMemory (pSharedMemory->cTaskBridge, strlen (pSharedMemory->cTaskBridge));
	g_free (pSharedMemory->cTaskBridge);
	pSharedMemory->cTaskBridge = NULL;
	
	if (doc == NULL)
	{
		cd_warning ("RSSresader : got invalid XML data");
		const gchar *cErrorMessage = D_("Invalid data (invalid RSS/Atom feed?)");
		_insert_error_message (myApplet, cErrorMessage);
		if (myDesklet)
		{
			cd_applet_update_my_icon (myApplet);
		}
		g_free (myData.PrevFirstTitle);
		myData.PrevFirstTitle = NULL;
		myData.bUpdateIsManual = FALSE;
		CD_APPLET_LEAVE (TRUE);
	}
	
	xmlNodePtr rss = xmlDocGetRootElement (doc);
	if (rss == NULL || (xmlStrcmp (rss->name, BAD_CAST "rss") != 0 && xmlStrcmp (rss->name, BAD_CAST "feed") != 0 && xmlStrcmp (rss->name, BAD_CAST "RDF") != 0))
	{
		cd_warning ("RSSresader : got invalid XML data");
		///xmlCleanupParser ();
		xmlFreeDoc (doc);
		
		const gchar *cErrorMessage = D_("Invalid data (invalid RSS/Atom feed?)");
		_insert_error_message (myApplet, cErrorMessage);
		if (myDesklet)
		{
			cd_applet_update_my_icon (myApplet);
		}
		g_free (myData.PrevFirstTitle);
		myData.PrevFirstTitle = NULL;
		myData.bUpdateIsManual = FALSE;
		CD_APPLET_LEAVE (TRUE);
	}
	
	// on extrait chaque item.
	GList *pNewItemList = NULL;
	CDRssItem *pItem = g_new0 (CDRssItem, 1);  // on commence au debut de la liste (c'est le titre).
	pNewItemList = g_list_prepend (pNewItemList, pItem);
	if (myConfig.cUserTitle != NULL)
		pItem->cTitle = g_strdup (myConfig.cUserTitle);  // ne sera pas ecrase par les donnees du flux.
	
	if (xmlStrcmp (rss->name, BAD_CAST "rss") == 0)  // RSS
	{
		xmlAttrPtr attr = xmlHasProp (rss, BAD_CAST "version");
		if (attr && attr->children)
		{
			cd_debug ("RSS version : %s", attr->children->content);
		}
		
		xmlNodePtr channel;
		for (channel = rss->children; channel != NULL; channel = channel->next)
		{
			if (xmlStrcmp (channel->name, BAD_CAST "channel") == 0)
			{
				pNewItemList = _parse_rss_item (channel, pItem, pNewItemList);  // on parse le channel comme un item, ce qui fait que le titre du flux est considere comme un simple item.
				break;  // un seul channel.
			}
		}
	}
	else if (xmlStrcmp (rss->name, BAD_CAST "RDF") == 0)  // RDF
	{
		pNewItemList = _parse_rss_item (rss, pItem, pNewItemList);  // on parse le premier groupe "channel" comme un item, ce qui fait que le titre du flux est considere comme un simple item.
	}
	else  // Atom
	{
		xmlNodePtr feed = rss;
		gchar *cBaseUrl = NULL;  // on recupere la base de l'URL, pour le cas ou les link seraient exprimes relativement a elle.
		gchar *str = g_strstr_len(myConfig.cUrl, 10, "://");
		if (str)
		{
			str = strchr (str + 3, '/');
			if (str)
				cBaseUrl = g_strndup (myConfig.cUrl, (gpointer)str-(gpointer)myConfig.cUrl);
		}
		pNewItemList = _parse_atom_item (feed, pItem, pNewItemList, cBaseUrl);  // on parse le feed comme un item, ce qui fait que le titre du flux est considere comme un simple item.
		g_free (cBaseUrl);
	}
	pNewItemList = g_list_reverse (pNewItemList);
	
	xmlFreeDoc (doc);  // ne pas utiliser xmlCleanupParser dans un thread !
	
	// si aucune donnee, on l'affiche et on quitte.
	if (pNewItemList == NULL)
	{
		cd_debug ("RSS: aucune donnee");
		
		const gchar *cErrorMessage = D_("No data");
		_insert_error_message (myApplet, cErrorMessage);
		if (myDesklet)
		{
			cd_applet_update_my_icon (myApplet);
		}
		g_free (myData.PrevFirstTitle);
		myData.PrevFirstTitle = NULL;
		myData.bUpdateIsManual = FALSE;
		CD_APPLET_LEAVE (TRUE);
	}
	
	// si on est arrive a ce point, c'est qu'il n'y a pas eu d'erreur.
	// on vide l'ancienne liste d'items.
	cd_rssreader_free_item_list (myApplet);
	myData.pItemList = pNewItemList;
	
	// on met a jour le titre.
	if (myIcon->cName == NULL)  // il faut mettre a jour le titre
	{
		if (myDock && myConfig.cUserTitle == NULL)  // en mode desklet inutile, le titre sera redessine avec le reste.
		{
			pItem = myData.pItemList->data;
			if (pItem != NULL && pItem->cTitle != NULL)
				CD_APPLET_SET_NAME_FOR_MY_ICON (pItem->cTitle);
		}
	}
	
	// si aucun changement, on le signale, et si aucune erreur precedente, on quitte.
	pItem = (myData.pItemList && myData.pItemList->next ? myData.pItemList->next->data : NULL);
	gchar *cFirstTitle = (pItem ? pItem->cTitle : NULL);
	if (! cairo_dock_strings_differ (myData.PrevFirstTitle, cFirstTitle))
	{
		cd_debug ("RSS: aucune modif");
		
		if (myData.bUpdateIsManual)  // L'update a ete manuel -> On affiche donc un dialogue meme s'il n'y a pas eu de changement
		{
			gldi_dialogs_remove_on_icon (myIcon);
			gldi_dialog_show_temporary_with_icon (D_("No modification"),
				myIcon,
				myContainer,
				2000, // Suffisant vu que la MaJ est manuelle
				myDock ? "same icon" : MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
				
			myData.bUpdateIsManual = FALSE;
		}
		
		if (! myData.bError)
			CD_APPLET_LEAVE (TRUE);
	}
	
	// on dessine le texte.
	if (myDesklet)
	{
		cd_applet_update_my_icon (myApplet);
	}
	
	// on avertit l'utilisateur.
	if (myData.PrevFirstTitle != NULL && myConfig.iNotificationType != 0)
	{
		if (myConfig.iNotificationType != 1)
		{
			gldi_dialogs_remove_on_icon (myIcon);
			gldi_dialog_show_temporary_with_icon (D_("This RSS feed has been modified..."),
				myIcon,
				myContainer,
				1000*myConfig.iNotificationDuration,
				myDock ? "same icon" : MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
		}
		if (myConfig.iNotificationType != 2)
		{
			CD_APPLET_DEMANDS_ATTENTION (myConfig.cNotificationAnimation, 3);  /// myConfig.iNotificationDuration ?...
		}
	}
	
	g_free (myData.PrevFirstTitle);
	myData.PrevFirstTitle = g_strdup (cFirstTitle);
	myData.bUpdateIsManual = FALSE;
	myData.bError = FALSE;
	CD_APPLET_LEAVE (TRUE);
}


static void _free_shared_memory (CDSharedMemory *pSharedMemory)
{
	g_free (pSharedMemory->cUrl);
	g_free (pSharedMemory->cUrlLogin);
	g_free (pSharedMemory->cUrlPassword);
	g_free (pSharedMemory->cTaskBridge);
	g_free (pSharedMemory);
}
void cd_rssreader_launch_task (GldiModuleInstance *myApplet)
{
	if (myData.pTask != NULL)
	{
		gldi_task_discard (myData.pTask);
		myData.pTask = NULL;
	}
	
	CDSharedMemory *pSharedMemory = g_new0 (CDSharedMemory, 1);
	pSharedMemory->cUrl = g_strdup (myConfig.cUrl);
	pSharedMemory->cUrlLogin = g_strdup (myConfig.cUrlLogin);
	pSharedMemory->cUrlPassword = g_strdup (myConfig.cUrlPassword);
	pSharedMemory->pApplet = myApplet;
	
	myData.pTask = gldi_task_new_full (myConfig.iRefreshTime,
		(GldiGetDataAsyncFunc) _get_feeds,
		(GldiUpdateSyncFunc) _update_from_feeds,
		(GFreeFunc) _free_shared_memory,
		pSharedMemory);
	gldi_task_launch (myData.pTask);
}



static void _on_dialog_destroyed (GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	myData.pDialog = NULL;
	CD_APPLET_LEAVE();
}
void cd_rssreader_show_dialog (GldiModuleInstance *myApplet)
{
	if (myData.pDialog != NULL)  // on detruit le dialogue actuel.
	{
		gldi_object_unref (GLDI_OBJECT(myData.pDialog));
		myData.pDialog = NULL;
		return;
	}
	gldi_dialogs_remove_on_icon (myIcon);  // on enleve tout autre dialogue (message d'erreur).
	
	if (myData.pItemList != NULL && myData.pItemList->next != NULL && (myData.pItemList->next->next != NULL || ! myData.bError))  // on construit le dialogue contenant toutes les infos.
	{
		// On construit le widget GTK qui contient les lignes avec les liens.
		GtkWidget *pVBox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);  // le widget qu'on va inserer dans le dialogue.
		GtkWidget *pScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
		g_object_set (pScrolledWindow, "height-request", 250, NULL);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (pScrolledWindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
		gtk_container_add (GTK_CONTAINER (pScrolledWindow), pVBox);
		
		PangoLayout *pLayout = pango_cairo_create_layout (myDrawContext);
		PangoFontDescription *fd = pango_font_description_from_string ("");
		pango_layout_set_font_description (pLayout, fd);
		
		int w = MIN (600, g_desktopGeometry.Xscreen.width / g_desktopGeometry.iNbScreens / 2);  // we don't know on which screen is place the container...
		gchar *cLine;
		GtkWidget *pLinkButton, *pAlign;
		CDRssItem *pItem;
		GList *it;
		for (it = myData.pItemList->next; it != NULL; it = it->next)
		{
			pItem = it->data;
			if (pItem->cTitle == NULL)
				continue;
			
			cLine = g_strdup (pItem->cTitle);
			cd_rssreader_cut_line (cLine, pLayout, w);
			
			if (pItem->cLink != NULL)
				pLinkButton = gtk_link_button_new_with_label (pItem->cLink, cLine);
			else
				pLinkButton = gtk_label_new (cLine);
			g_free (cLine);
			
			pAlign = gtk_alignment_new (0., 0.5, 0., 0.);
			gtk_container_add (GTK_CONTAINER (pAlign), pLinkButton);
			gtk_box_pack_start (GTK_BOX (pVBox), pAlign, FALSE, FALSE, 0);
			
			if (pItem->cDescription != NULL)
			{
				cLine = g_strdup (pItem->cDescription);
				cd_rssreader_cut_line (cLine, pLayout, w);
				pLinkButton = gtk_label_new (cLine);
				gtk_label_set_selectable (GTK_LABEL (pLinkButton), TRUE);
				g_free (cLine);
				
				pAlign = gtk_alignment_new (0., 0.5, 0., 0.);
				gtk_alignment_set_padding (GTK_ALIGNMENT (pAlign), 0, 0, 20, 0);
				gtk_container_add (GTK_CONTAINER (pAlign), pLinkButton);
				gtk_box_pack_start (GTK_BOX (pVBox), pAlign, FALSE, FALSE, 0);
			}
			
			if (pItem->cAuthor != NULL)
			{
				gchar *by = g_strdup_printf ("  [by %s]", pItem->cAuthor);
				pLinkButton = gtk_label_new (by);
				g_free (by);
				
				pAlign = gtk_alignment_new (0., 0.5, 0., 0.);
				gtk_alignment_set_padding (GTK_ALIGNMENT (pAlign), 0, 0, 40, 0);
				gtk_container_add (GTK_CONTAINER (pAlign), pLinkButton);
				gtk_box_pack_start (GTK_BOX (pVBox), pAlign, FALSE, FALSE, 0);
			}
			
			if (pItem->cDate != NULL)
			{
				pLinkButton = gtk_label_new (pItem->cDate);
				
				pAlign = gtk_alignment_new (1., 0.5, 0., 0.);
				gtk_alignment_set_padding (GTK_ALIGNMENT (pAlign), 0, 0, 40, 0);
				gtk_container_add (GTK_CONTAINER (pAlign), pLinkButton);
				gtk_box_pack_start (GTK_BOX (pVBox), pAlign, FALSE, FALSE, 0);
			}
		}
		pango_font_description_free (fd);
		
		pItem = myData.pItemList->data;  // le nom du flux en titre du dialogue.
		
		// on affiche le dialogue.
		myData.pDialog = gldi_dialog_show (pItem->cTitle,
			myIcon, myContainer,
			0,
			myDock ? "same icon" : MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
			pScrolledWindow,
			NULL,
			myApplet,
			(GFreeFunc)_on_dialog_destroyed);
		/**g_signal_connect (G_OBJECT (myData.pDialog->container.pWidget),
			"button-press-event",
			G_CALLBACK (on_button_press_dialog),
			myApplet);*/
	}
	else  // on affiche un message clair a l'utilisateur.
	{
		if (myConfig.cUrl == NULL)
			gldi_dialog_show_temporary_with_icon (D_("No URL is defined\nYou can define one by copying the URL in the clipboard,\n and selecting \"Paste the URL\" in the menu."),
				myIcon,
				myContainer,
				1000*myConfig.iNotificationDuration,
				myDock ? "same icon" : MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
		else
			gldi_dialog_show_temporary_with_icon (D_("No data\nDid you set a valid RSS feed?\nIs your connection alive?"),
				myIcon,
				myContainer,
				1000*myConfig.iNotificationDuration,
				myDock ? "same icon" : MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
	}
}

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

//\________________ Add your name in the copyright file (and / or modify your name here)

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-rss.h"

/* Insere des retours chariots dans une chaine de caracteres de facon a la faire tenir dans un rectangle donne.
 */
void cd_rssreader_cut_line (gchar *cLine, PangoLayout *pLayout, int iMaxWidth)
{
	g_print ("%s (%s)\n", __func__, cLine);
	/*// on convertit les caracteres internet.
	gchar *str=cLine, *amp;
	do
	{
		amp = strchr (str, '&');
		if (!amp)
			break;
		if (amp[1] == '#' && g_ascii_isdigit (amp[2]) && g_ascii_isdigit (amp[3]) && g_ascii_isdigit (amp[4]) && amp[5] == ';')  // &#039;
		{
			*amp = atoi (amp+2);
			sprintf (amp+1, amp+6);
		}
		str = amp + 1;
	} while (1);*/
	
	// on insere des retours chariot pour tenir dans la largeur donnee.
	PangoRectangle ink, log;
	gchar *sp, *last_sp=NULL;
	double w;
	
	gchar *str = cLine;
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
				str = last_sp + 1;  // on place le debut de ligne apres la coupure.
			}
			else  // aucun espace, c'est un mot entier.
			{
				*sp = '\n';  // on coupe apres le mot.
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


void cd_rssreader_free_item (CDRssItem *pItem)
{
	if (pItem == NULL)
		return;
	
	g_free (pItem->cTitle);
	g_free (pItem->cDescription);
	g_free (pItem->cLink);
	g_free (pItem);
}

void cd_rssreader_free_item_list (CairoDockModuleInstance *myApplet)
{
	if (myData.pItemList == NULL)
		return;
	CDRssItem *pItem;
	GList *it;
	for (it = myData.pItemList; it != NULL; it = it->next)
	{
		pItem = it->data;
		cd_rssreader_free_item (pItem);
	}
	g_list_free (myData.pItemList);
	myData.pItemList = NULL;
}


static void _get_feeds (CairoDockModuleInstance *myApplet)
{
	if (myConfig.cUrl == NULL)
		return ;
	
	gchar *cCommand = g_strdup_printf ("curl -s --connect-timeout 30 \"%s\"", myConfig.cUrl);
	myData.cTaskBridge = cairo_dock_launch_command_sync (cCommand);
	cd_debug ("cTaskBridge : '%s'", myData.cTaskBridge);
	g_free (cCommand);
}

static GList * _parse_rss_item (xmlNodePtr node, CDRssItem *pItem, GList *pItemList)
{
	xmlChar *content;
	xmlNodePtr item;
	for (item = node->children; item != NULL; item = item->next)
	{
		if (xmlStrcmp (item->name, (const xmlChar *) "item") == 0)  // c'est un nouvel item.
		{
			CDRssItem *pNewItem = g_new0 (CDRssItem, 1);
			pItemList = g_list_prepend (pItemList, pNewItem);
			
			pItemList = _parse_rss_item (item, pNewItem, pItemList);
		}
		else if (xmlStrcmp (item->name, (const xmlChar *) "title") == 0)  // c'est le titre.
		{
			if (pItem->cTitle == NULL)  // cas du titre du flux force a une valeur par l'utilisateur.
			{
				content = xmlNodeGetContent (item);
				pItem->cTitle = g_strdup (content);
				xmlFree (content);
			}
			cd_debug ("+ titre : '%s'", pItem->cTitle);
		}
		else if (xmlStrcmp (item->name, (const xmlChar *) "description") == 0)  // c'est la description.
		{
			content = xmlNodeGetContent (item);
			pItem->cDescription = g_strdup (content);
			xmlFree (content);
			
			// on elimine les balises integrees a la description.
			gchar *str = pItem->cDescription, *balise, *balise2;
			/*do
			{
				balise2 = NULL;
				balise = g_strstr_len (str, -1, "&lt;");  // debut de balise ("<")
				if (balise)
					balise2 = g_strstr_len (balise+4, -1, "&gt;");  // fin de balise (">")
				if (balise2)
				{
					strcpy (balise, balise2+4);
					str = balise;
				}
			}
			while (balise2);*/
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
		else if (xmlStrcmp (item->name, (const xmlChar *) "link") == 0)  // c'est le lien.
		{
			content = xmlNodeGetContent (item);
			pItem->cLink = g_strdup (content);
			xmlFree (content);
			cd_debug ("+ link : '%s'", pItem->cLink);
		}
		// pour recuperer l'image, on dirait qu'on a plusieurs cas, entre autre :
		// <enclosure url="http://medias.lemonde.fr/mmpub/edt/ill/2009/11/01/h_1_ill_1261356_5d02_258607.jpg" length="2514" type="image/jpeg" />  ----> bien verifier que c'est une image.
		// ou
		// <media:thumbnail url="http://www.france24.com/fr/files_fr/EN-interview-Gursel-m.jpg" />
	}
	return pItemList;
}

static GList * _parse_atom_item (xmlNodePtr node, CDRssItem *pItem, GList *pItemList)
{
	xmlChar *content;
	xmlNodePtr item, author;
	for (item = node->children; item != NULL; item = item->next)
	{
		if (xmlStrcmp (item->name, (const xmlChar *) "entry") == 0)  // c'est un nouvel item.
		{
			CDRssItem *pNewItem = g_new0 (CDRssItem, 1);
			pItemList = g_list_prepend (pItemList, pNewItem);
			
			pItemList = _parse_atom_item (item, pNewItem, pItemList);
		}
		else if (xmlStrcmp (item->name, (const xmlChar *) "title") == 0)  // c'est le titre.
		{
			if (pItem->cTitle == NULL)  // cas du titre du flux force a une valeur par l'utilisateur.
			{
				content = xmlNodeGetContent (item);
				pItem->cTitle = g_strdup (content);
				xmlFree (content);
			}
			cd_debug ("+ titre : '%s'", pItem->cTitle);
		}
		else if (xmlStrcmp (item->name, (const xmlChar *) "content") == 0)  // c'est la description.
		{
			xmlAttrPtr attr = xmlHasProp (item, "type");
			if (attr && attr->children)
			{
				g_print ("content type : %s\n", attr->children->content);
				if (strncmp (attr->children->content, "text", 4) != 0)
				{
					continue;
				}
			}
			content = xmlNodeGetContent (item);
			pItem->cDescription = g_strdup (content);
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
		else if (xmlStrcmp (item->name, (const xmlChar *) "link") == 0)  // c'est le lien.
		{
			xmlAttrPtr attr = xmlHasProp (item, "type");  // type="text/html" rel="alternate"
			if (attr && attr->children)
			{
				g_print ("link type : %s\n", attr->children->content);
				if (strncmp (attr->children->content, "text", 4) != 0)
				{
					continue;
				}
			}
			attr = xmlHasProp (item, "href");
			if (attr && attr->children)
			{
				content = xmlNodeGetContent (attr->children);
				pItem->cLink = g_strdup (content);
				xmlFree (content);
				cd_debug ("+ link : '%s'", pItem->cLink);
			}
		}
		else if (xmlStrcmp (item->name, (const xmlChar *) "author") == 0)  // c'est l'auteur.
		{
			for (author = item->children; author != NULL; author = author->next)
			{
				if (xmlStrcmp (author->name, (const xmlChar *) "name") == 0)  // c'est le nom de l'auteur.
				{
					content = xmlNodeGetContent (author);
					pItem->cAuthor = g_strdup (content);
					xmlFree (content);
					cd_debug ("+ author : '%s'", pItem->cAuthor);
				}
			}
		}
		// et pour l'image je ne sais pas.
	}
	return pItemList;
}

static gboolean _update_from_feeds (CairoDockModuleInstance *myApplet)
{
	// on vide l'ancienne liste d'items.
	cd_rssreader_free_item_list (myApplet);
	myData.pItemList = NULL;
	
	// On parse le flux XML.
	if (myData.cTaskBridge == NULL || *myData.cTaskBridge == '\0')
	{
		cd_warning ("RSSresader : no data");
		CDRssItem *pItem = g_new0 (CDRssItem, 1);
		myData.pItemList = g_list_prepend (myData.pItemList, pItem);
		if (myConfig.cUrl == NULL)
			pItem->cTitle = g_strdup (D_("No URL is defined."));
		else
			pItem->cTitle = g_strdup (D_("No data (no connection ?)"));
		if (myDesklet)
		{
			cd_applet_update_my_icon (myApplet);
		}
		return TRUE;
	}
	
	xmlDocPtr doc = xmlParseMemory (myData.cTaskBridge, strlen (myData.cTaskBridge));
	g_free (myData.cTaskBridge);
	myData.cTaskBridge = NULL;
	
	if (doc == NULL)
	{
		cd_warning ("RSSresader : got invalid XML data");
		CDRssItem *pItem = g_new0 (CDRssItem, 1);
		myData.pItemList = g_list_prepend (myData.pItemList, pItem);
		pItem->cTitle = g_strdup (D_("Invalid data (invalid RSS/Atom feed ?)"));
		if (myDesklet)
		{
			cd_applet_update_my_icon (myApplet);
		}
		return TRUE;
	}
	
	xmlNodePtr rss = xmlDocGetRootElement (doc);
	if (rss == NULL || (xmlStrcmp (rss->name, (const xmlChar *) "rss") != 0 && xmlStrcmp (rss->name, (const xmlChar *) "feed") != 0))
	{
		cd_warning ("RSSresader : got invalid XML data");
		xmlCleanupParser ();
		xmlFreeDoc (doc);
		
		CDRssItem *pItem = g_new0 (CDRssItem, 1);
		myData.pItemList = g_list_prepend (myData.pItemList, pItem);
		pItem->cTitle = g_strdup (D_("Invalid data (invalid RSS/Atom feed ?)"));
		if (myDesklet)
		{
			cd_applet_update_my_icon (myApplet);
		}
		return TRUE;
	}
	
	// on extrait chaque item.
	CDRssItem *pItem = g_new0 (CDRssItem, 1);  // on commence au debut de la liste (c'est le titre).
	myData.pItemList = g_list_prepend (myData.pItemList, pItem);
	if (myConfig.cUserTitle != NULL)
		pItem->cTitle = g_strdup (myConfig.cUserTitle);
	
	if (xmlStrcmp (rss->name, (const xmlChar *) "rss") == 0)  // RSS
	{
		xmlAttrPtr attr = xmlHasProp (rss, "version");
		if (attr && attr->children)
		{
			g_print ("RSS version : %s\n", attr->children->content);
		}
		
		xmlNodePtr channel, item;
		for (channel = rss->children; channel != NULL; channel = channel->next)
		{
			if (xmlStrcmp (channel->name, (const xmlChar *) "channel") == 0)
			{
				myData.pItemList = _parse_rss_item (channel, pItem, myData.pItemList);  // on parse le channel comme un item, ce qui fait que le titre du flux est considere comme un simple item.
				break;  // un seul channel.
			}
		}
	}
	else  // Atom
	{
		xmlNodePtr feed = rss;
		myData.pItemList = _parse_atom_item (feed, pItem, myData.pItemList);  // on parse le feed comme un item, ce qui fait que le titre du flux est considere comme un simple item.
	}
	myData.pItemList = g_list_reverse (myData.pItemList);
	
	xmlCleanupParser ();
	xmlFreeDoc (doc);
	
	// si aucune donnee, on l'affiche et on quitte.
	if (myData.pItemList == NULL)
	{
		g_print ("RSS: aucune donnee\n");
		
		pItem = g_new0 (CDRssItem, 1);
		myData.pItemList = g_list_prepend (myData.pItemList, pItem);
		pItem->cTitle = g_strdup (D_("No data"));
		
		if (myDesklet)
		{
			cd_applet_update_my_icon (myApplet);
		}
		
		return TRUE;
	}
	
	// on met a jour le titre.
	if (myIcon->cName == NULL)  // il faut mettre a jour le titre
	{
		if (myDock && myConfig.cUserTitle == NULL)  // en mode desklet inutile, le titre sera redessine avec le reste.
		{
			pItem = (myData.pItemList ? myData.pItemList->data : NULL);
			if (pItem != NULL && pItem->cTitle != NULL)
				CD_APPLET_SET_NAME_FOR_MY_ICON (pItem->cTitle);
			else if (myData.cTaskBridge)
				CD_APPLET_SET_NAME_FOR_MY_ICON (D_("No data (invalid rss ?)"));
			else
				CD_APPLET_SET_NAME_FOR_MY_ICON (D_("No data (no connection ?)"));
		}
	}
	
	// si aucun changement, on quitte.
	pItem = (myData.pItemList && myData.pItemList->next ? myData.pItemList->next->data : NULL);
	gchar *cFirstTitle = (pItem ? pItem->cTitle : NULL);
	if (! cairo_dock_strings_differ (myData.PrevFirstTitle, cFirstTitle))
	{
		g_print ("RSS: aucune modif\n");
		
		if (myData.bUpdateIsManual)  // L'update a été manuel -> On affiche donc un dialogue même s'il n'y a pas eu de changement
		{
			cairo_dock_remove_dialog_if_any (myIcon);
			cairo_dock_show_temporary_dialog_with_icon (D_("No modification"),
				myIcon,
				myContainer,
				2000, // Suffisant vu que la MaJ est manuelle
				MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
				
			myData.bUpdateIsManual = FALSE;
		}
		
		return TRUE;
	}
	// on dessine le texte.
	if (myDesklet)
	{
		cd_applet_update_my_icon (myApplet);
	}
	
	// on avertit l'utilisateur.
	if (myData.PrevFirstTitle != NULL)
	{
		if (myConfig.bDialogIfFeedChanged)
		{
			cairo_dock_remove_dialog_if_any (myIcon);
			cairo_dock_show_temporary_dialog_with_icon (D_("This RSS feed has been modified..."),
				myIcon,
				myContainer,
				myConfig.iDialogsDuration,
				MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
		}
		if (myConfig.cAnimationIfFeedChanged)
		{
			CD_APPLET_ANIMATE_MY_ICON (myConfig.cAnimationIfFeedChanged, 3);  // 3 tours.
		}
	}
	
	g_free (myData.PrevFirstTitle);
	myData.PrevFirstTitle = g_strdup (cFirstTitle);
	
	return TRUE;
}

void cd_rssreader_upload_feeds_TASK (CairoDockModuleInstance *myApplet)
{
	if (myData.pTask == NULL) // la tache n'existe pas, on la cree et on la lance.
	{
		myData.pTask = cairo_dock_new_task (myConfig.iRefreshTime,
			(CairoDockGetDataAsyncFunc) _get_feeds,
			(CairoDockUpdateSyncFunc) _update_from_feeds,
			myApplet);
		cairo_dock_launch_task (myData.pTask);
	}
	else // la tache existe, on la relance immediatement, avec la nouvelle frequence eventuellement.
	{
		cairo_dock_relaunch_task_immediately (myData.pTask, myConfig.iRefreshTime);
	}
}


void cd_rssreader_show_dialog (CairoDockModuleInstance *myApplet)
{
	cairo_dock_remove_dialog_if_any (myIcon);
	
	if (myData.pItemList != NULL)
	{
		/// TODO : arriver a un dialogue remplacant completement l'utilisation du browser :
		/// title + link + description
		/// je pensais a un widget GTK a mettre dans le dialogue, et peut-etre un expander pour les descriptions.
		/// aussi, il faudrait couper les lignes pour éviter que le dialgue soit immense en largeur.
		
		/// pour le moment c'est juste les titres, je n'ai rien ajoute ;-)
		GString *sText = g_string_new ("");
		CDRssItem *pItem;
		GList *it;
		int i;
		for (it = myData.pItemList, i = 0; it != NULL && i < myConfig.iNbLinesInDialog + 1; it = it->next, i ++)
		{
			pItem = it->data;
			if (pItem->cTitle == NULL)
			{
				i --;
				continue;
			}
			
			g_string_append_printf (sText, "%s\n", pItem->cTitle);
		}
		cairo_dock_show_temporary_dialog_with_icon (sText->str,
			myIcon,
			myContainer,
			myConfig.iDialogsDuration,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
		g_string_free (sText, TRUE);
	}
	else  // on affiche un message clair a l'utilisateur.
	{
		if (myConfig.cUrl == NULL)
			cairo_dock_show_temporary_dialog_with_icon (D_("No URL is defined\nYou can define one by copying the URL in the clipboard,\n and selecting \"Paste the RL\n in the menu."),
				myIcon,
				myContainer,
				myConfig.iDialogsDuration,
				MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
		else
			cairo_dock_show_temporary_dialog_with_icon (D_("No data\nDid you set a valid RSS feed ?\nIs your connection alive ?"),
				myIcon,
				myContainer,
				myConfig.iDialogsDuration,
				MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
	}
}

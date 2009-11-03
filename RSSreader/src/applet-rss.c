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

#include <math.h>
#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-rss.h"

/* Insere des retours chariots dans une chaine de caracteres de facon à la faire tenir dans un rectangle donne.
 */
void cd_rssreader_cut_line (gchar *cLine, PangoLayout *pLayout, int iMaxWidth)
{
	g_print ("%s (%s)\n", __func__, cLine);
	// on convertit les caracteres internet.
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
	} while (1);
	
	// on insere des retours chariot pour tenir dans la largeur donnee.
	PangoRectangle ink, log;
	gchar *sp, *last_sp=NULL;
	double w;
	
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


void cd_rssreader_free_item (CDRssItem *pItem, gboolean bFree)
{
	if (pItem == NULL)
		return;
	
	g_free (pItem->cTitle);
	g_free (pItem->cDescription);
	g_free (pItem->cLink);
	if (bFree)
		g_free (pItem);
}

void cd_rssreader_free_item_list (CairoDockModuleInstance *myApplet)
{
	if (myData.pItemList == NULL)
		return;
	CDRssItem *pItem;
	int i;
	for (i = 0; i < myData.iNbItems; i ++)
	{
		pItem = &myData.pItemList[i];
		cd_rssreader_free_item (pItem, FALSE);
	}
	g_free (myData.pItemList);
	myData.pItemList = NULL;
}

static void _get_feeds (CairoDockModuleInstance *myApplet)
{		
	if (myConfig.cUrl == NULL)
	{
		myData.cTaskBridge = g_strdup_printf ("%s\n",myConfig.cMessageNoUrl);  // Le \n est important pour la suite
		cd_debug ("RSSreader-debug : TASK ---------------> myData.cTaskBridge = \"%s\"", myData.cTaskBridge);	
	}
	else
	{
		///g_free (myData.cSingleFeedLine);  // ici on se contente de recuperer le flux.
		///myData.cSingleFeedLine = NULL;		
		
		gchar *cCommand = g_strdup_printf ("%s/rss_reader.sh %s %i %i", MY_APPLET_SHARE_DATA_DIR, myConfig.cUrl, myConfig.iLines + myConfig.iTitleNum, myConfig.iTitleNum);
		
		g_print("RSSreader-debug : TASK ---------------------->  %s\n", cCommand);
		
		//g_spawn_command_line_sync (cCommand, &myData.cTaskBridge, NULL, NULL, NULL);
		myData.cTaskBridge = cairo_dock_launch_command_sync (cCommand);
		g_print ("RSSreader-debug : TASK ---------------> myData.cTaskBridge = \"%s\"^n", myData.cTaskBridge);
		g_free (cCommand);		
		
		// On vérifie que la commande nous a renvoyé quelque chose de cohérent
		if (myData.cTaskBridge == NULL || *myData.cTaskBridge == '\0')
			myData.cTaskBridge = g_strdup_printf ("%s\n", myConfig.cMessageFailedToConnect);  // Le \n est important pour la suite
	}	
}
static gboolean _update_from_feeds (CairoDockModuleInstance *myApplet)
{
	// On récupère le flux.
	myData.cAllFeedLines = g_strdup (myData.cTaskBridge);  // On récupère le contenu de myData.cTaskBridge -> myData.cAllFeedLines nous servira pour les dialogues ;-)		
	
	cd_rssreader_free_item_list (myApplet);
	myData.pItemList = g_new0 (CDRssItem, myConfig.iLines+2);  // la 1ere ligne est le titre, +1 pour finir sur un item vide.
	CDRssItem *pItem = myData.pItemList;
	int i = 0;
	gchar *str = myData.cTaskBridge, *rc;
	do
	{
		rc = strchr (str, '\n');  // on cherche le prochain retour chariot.
		if (rc)
			*rc = '\0';  // on coupe.
		
		if (strncmp (str, "<title>", 7) == 0)  // c'est le titre.
		{
			if (pItem->cTitle != NULL)  // titre deja renseigne => c'est un nouvel item.
			{
				i ++;  // item suivant.
				if (i == myConfig.iLines + 1)  // on arrive a la fin => on quitte.
					break;
				pItem = &myData.pItemList[i];
			}
			pItem->cTitle = g_strdup (str+7);
			g_print ("+ titre : '%s'\n", pItem->cTitle);
		}
		else if (strncmp (str, "<description>", 13) == 0)  // c'est la description.
		{
			if (pItem->cDescription != NULL)  // description deja renseignee => c'est un nouvel item.
			{
				i ++;  // item suivant.
				if (i == myConfig.iLines + 1)  // on arrive a la fin => on quitte.
					break;
				pItem = &myData.pItemList[i];
			}
			pItem->cDescription = g_strdup (str+13);
			
			// on elimine les balises integrees a la description.
			gchar *str = pItem->cDescription, *balise, *balise2;
			do
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
			while (balise2);
			g_print ("+ description : '%s'\n", pItem->cDescription);
		}
		else if (strncmp (str, "<link>", 6) == 0)  // c'est le lien (TODO : verifier qu'il n'y a bien qu'un seul lien par item.)
		{
			if (pItem->cLink != NULL)  // lien deja renseigne => on ecrase.
			{
				g_free (pItem->cLink);
			}
			pItem->cLink = g_strdup (str+6);
			g_print ("+ link : '%s'\n", pItem->cLink);
		}
		// pour recuperer l'image, on dirait qu'on a 2 cas :
		// <enclosure url="http://medias.lemonde.fr/mmpub/edt/ill/2009/11/01/h_1_ill_1261356_5d02_258607.jpg" length="2514" type="image/jpeg" />  ----> bien verifier que c'est une image.
		// ou
		// <media:thumbnail url="http://www.france24.com/fr/files_fr/EN-interview-Gursel-m.jpg" />
		
		str = rc + 1;  // on passe a la suite.
	}
	while (rc);
	myData.iNbItems = i;
	
	// si aucune donnee, on l'affiche et on quitte.
	if (i == 0)
	{
		g_print ("RSS: aucune donnee\n");
		myData.pItemList = g_new0 (CDRssItem, 2);
		myData.pItemList[0].cTitle = g_strdup (D_("No data"));
		myData.iNbItems = 1;
		
		if (myDesklet)
		{
			cd_applet_update_my_icon (myApplet);
		}
		return TRUE;
	}
	
	// on met a jour le titre.
	if (myIcon->cName == NULL)  // il faut mettre a jour le titre
	{
		if (myDock)  // en mode desklet inutile, le titre sera redessine avec le reste.
		{
			if (myData.pItemList[0].cTitle)
				CD_APPLET_SET_NAME_FOR_MY_ICON (myData.pItemList[0].cTitle);
			else if (myData.cTaskBridge)
				CD_APPLET_SET_NAME_FOR_MY_ICON (D_("No data (invalid rss ?)"));
			else
				CD_APPLET_SET_NAME_FOR_MY_ICON (D_("No data (no connection ?)"));
		}
	}
	
	// si aucun changement, on quitte.
	if (! cairo_dock_strings_differ (myData.PrevFirstTitle, myData.pItemList[1].cTitle))
	{
		g_print ("RSS: aucune modif\n");
		
		if (myData.bUpdateIsManual)  // L'update a été manuel -> On affiche donc un dialogue même s'il n'y a pas eu de changement
		{				
			cairo_dock_remove_dialog_if_any (myIcon);
			cairo_dock_show_temporary_dialog_with_icon (D_("No modification"),
				myIcon,
				myContainer,
				2000, // Suffisant vu que la MàJ est manuelle
				MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
				
			myData.bUpdateIsManual = FALSE;				
		}
		
		return TRUE;
	}
	g_free (myData.PrevFirstTitle);
	myData.PrevFirstTitle = g_strup (myData.pItemList[1].cTitle);
	
	// on dessine le texte.
	if (myDesklet)
	{
		cd_applet_update_my_icon (myApplet);
	}
	
	// on avertit l'utilisateur.
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
	
	/**
	// On vérifie le nombre de lignes reçues
 	gchar cCurrentLetter = {0};
	gint iNbLines = 1;
	long i=0;
	for (i=0 ; i < strlen(myData.cAllFeedLines) ; i++)
	{
		cCurrentLetter = myData.cAllFeedLines[i];				
		if (cCurrentLetter == '\n')
			iNbLines++;
	}
	
	// On sépare toutes les lignes reçues
	if (strcmp(myData.cAllFeedLines, g_strdup_printf ("%s\n", myConfig.cMessageNoUrl) ) == 0) // Si strcmp renvoie 0 (chaînes identiques)
		myData.cAllFeedLines = g_strdup_printf ("%s\n%s", myConfig.cMessageNoUrl, myConfig.cMessageNoUrl2);
	
	myData.cSingleFeedLine = g_strsplit (myData.cAllFeedLines,"\n",0);
	
	i = 0;
	do
	{
		cd_debug ("RSSreader-debug : UPDATE ---------------> myData.cSingleFeedLine[%i] \"%s\"",i ,myData.cSingleFeedLine[i]);
		i++;
	} while (myData.cSingleFeedLine[i] != NULL);
	
	cd_debug ("RSSreader-debug : UPDATE ---------------> Current first line = \"%s\"",myData.cSingleFeedLine[0]);
	cd_debug ("RSSreader-debug : UPDATE ---------------> Last first line    = \"%s\"",myData.cLastFirstFeedLine);
	cd_debug ("RSSreader-debug : UPDATE ---------------> Current second line = \"%s\"",myData.cSingleFeedLine[1]);
	cd_debug ("RSSreader-debug : UPDATE ---------------> Last second line    = \"%s\"",myData.cLastSecondFeedLine);	
	
	
	// On teste s'il y a eu une modification depuis le dernier update
	if (myData.cLastFirstFeedLine == NULL)
	{
		myData.cLastFirstFeedLine = g_strdup_printf ("%s", myData.cSingleFeedLine[0]); // On mémorise pour le prochain update
		if (myData.cSingleFeedLine[1] != NULL)
			myData.cLastSecondFeedLine = g_strdup_printf ("%s", myData.cSingleFeedLine[1]); // On mémorise pour le prochain update
		else
			myData.cLastSecondFeedLine = NULL;
		cd_debug ("RSSreader-debug : CONTROL MODIFICATION --------------->  1st START !");
	}
	else
	{
		if (strcmp(myData.cSingleFeedLine[0], myData.cLastFirstFeedLine) != 0 || strcmp(myData.cSingleFeedLine[1], myData.cLastSecondFeedLine) != 0) // On vérifie aussi la 2nd ligne car la première peut être le titre
		{
			cd_debug ("RSSreader-debug : CONTROL MODIFICATION --------------->  Feed has been modified !");	
			cairo_dock_remove_dialog_if_any (myIcon);			
			
			myData.cDialogMessage = g_strdup_printf ("\"%s\"\n%s",myConfig.cName, D_("This RSS feed has been modified...") );
			
			// Si modif ET si myConfig.bDialogIfFeedChanged est vrai, on affiche une bulle de dialogue pour le signaler
			if (myConfig.bDialogIfFeedChanged)
			{
				cairo_dock_show_temporary_dialog_with_icon (myData.cDialogMessage,
					myIcon,
					myContainer,
					myConfig.iDialogsDuration,
					MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
			}
			
			myData.cLastFirstFeedLine = g_strdup_printf ("%s", myData.cSingleFeedLine[0]); // On mémorise pour le prochain update
			
			if (myData.cSingleFeedLine[1] != NULL)
				myData.cLastSecondFeedLine = g_strdup_printf ("%s", myData.cSingleFeedLine[1]); // On mémorise pour le prochain update
			else
				myData.cLastSecondFeedLine = NULL;
			cd_debug ("RSSreader-debug : CONTROL MODIFICATION --------------->  Feed has been stored for next update !");				
		}
		else
		{
			cd_debug ("RSSreader-debug : CONTROL MODIFICATION --------------->  No modification.");
			if (myData.bUpdateIsManual)  // L'update a été manuel -> On affiche donc un dialogue même s'il n'y a pas eu de changement
			{				
				cairo_dock_remove_dialog_if_any (myIcon);
				
				myData.cDialogMessage = g_strdup_printf ("\"%s\"\n%s",myConfig.cName, D_("No modification"));				
				// On signale tout de même qu'il n'y a pas de changement
				cairo_dock_show_temporary_dialog_with_icon (myData.cDialogMessage,
					myIcon,
					myContainer,
					2000, // Suffisant vu que la MàJ est manuelle
					MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
					
				myData.bUpdateIsManual = FALSE;				
			}
		}
	}
	cd_applet_update_my_icon (myApplet);*/
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

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

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-session.h"
#include "applet-appli-finder.h"
#include "applet-icon-finder.h"

static inline gboolean _cd_do_icon_match (Icon *pIcon, const gchar *cCommandPrefix, int length)
{
	gboolean bMatch = FALSE;
	if (pIcon->cBaseURI != NULL)
	{
		gchar *cFile = g_path_get_basename (pIcon->cCommand);
		bMatch = (cFile && g_ascii_strncasecmp (cCommandPrefix, cFile, length) == 0);
		g_free (cFile);
	}
	else if (pIcon->cCommand)
	{
		bMatch = (g_ascii_strncasecmp (cCommandPrefix, pIcon->cCommand, length) == 0);
		if (!bMatch)
		{
			gchar *str = strchr (pIcon->cCommand, '-');  // on se limite au 1er tiret.
			if (str && *(str-1) != ' ')  // on verifie qu'il n'est pas un tiret d'option
			{
				str ++;
				bMatch = (g_ascii_strncasecmp (str, cCommandPrefix, length) == 0);
			}
			if (!bMatch && pIcon->cName)
				bMatch = (g_ascii_strncasecmp (cCommandPrefix, pIcon->cName, length) == 0);
		}
	}
	return bMatch;
}


static inline void _cd_do_search_matching_icons_in_dock (CairoDock *pDock)
{
	Icon *pIcon;
	GList *ic;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (_cd_do_icon_match (pIcon, myData.sCurrentText->str, myData.sCurrentText->len))
		{
			myData.pMatchingIcons = g_list_prepend (myData.pMatchingIcons, pIcon);
		}
	}
}
static void _cd_do_search_in_one_dock (Icon *pIcon, CairoDock *pDock, gpointer data)
{
	if (_cd_do_icon_match (pIcon, myData.sCurrentText->str, myData.sCurrentText->len))
	{
		myData.pMatchingIcons = g_list_prepend (myData.pMatchingIcons, pIcon);
	}
}
void cd_do_search_matching_icons (void)
{
	if (myData.sCurrentText->len == 0)
		return;
	cd_debug ("%s (%s)\n", __func__, myData.sCurrentText->str);
	gchar *str = strchr (myData.sCurrentText->str, ' ');  // on ne compte pas les arguments d'une eventuelle commande deja tapee.
	guint length = myData.sCurrentText->len;
	if (str != NULL)
	{
		g_string_set_size (myData.sCurrentText, str - myData.sCurrentText->str + 1);
		cd_debug (" on ne cherchera que '%s' (len=%d)\n", myData.sCurrentText->str, myData.sCurrentText->len);
	}
		
	if (myData.pMatchingIcons == NULL)
	{
		cd_debug ("on cherche tout\n");
		// on parcours tous les docks.
		cairo_dock_foreach_icons_in_docks ((CairoDockForeachIconFunc) _cd_do_search_in_one_dock, NULL);
		myData.pMatchingIcons = g_list_reverse (myData.pMatchingIcons);
		
		// on rajoute les icones ne venant pas du dock.
		cd_do_find_matching_applications ();
	}
	else  // optimisation : on peut se contenter de chercher parmi les icones deja trouvees.
	{
		cd_debug ("on se contente d'enlever celles en trop\n");
		GList *ic, *next_ic;
		Icon *pIcon;
		ic = myData.pMatchingIcons;
		while (ic != NULL)
		{
			pIcon = ic->data;
			next_ic = ic->next;
			if (! _cd_do_icon_match (pIcon, myData.sCurrentText->str, myData.sCurrentText->len))
				myData.pMatchingIcons = g_list_delete_link (myData.pMatchingIcons, ic);
			ic = next_ic;
		}
	}
	myData.pCurrentMatchingElement = myData.pMatchingIcons;
	myData.iMatchingGlideCount = 0;
	myData.iPreviousMatchingOffset = 0;
	myData.iCurrentMatchingOffset = 0;
	if (myData.pCurrentApplicationToLoad != NULL)  // on va continuer le chargement sur la sous-liste.
		myData.pCurrentApplicationToLoad = myData.pMatchingIcons;  // comme l'ordre de la liste n'a pas ete altere, on n'est sur de ne pas sauter d'icone.
	cairo_dock_redraw_container (CAIRO_CONTAINER (g_pMainDock));
	//g_print ("%d / %d\n", length , myData.sCurrentText->len);
	if (length != myData.sCurrentText->len)
		g_string_set_size (myData.sCurrentText, length);
}


void cd_do_select_previous_next_matching_icon (gboolean bNext)
{
	GList *pMatchingElement = myData.pCurrentMatchingElement;
	do
	{
		if (!bNext)
			myData.pCurrentMatchingElement = cairo_dock_get_previous_element (myData.pCurrentMatchingElement, myData.pMatchingIcons);
		else
			myData.pCurrentMatchingElement = cairo_dock_get_next_element (myData.pCurrentMatchingElement, myData.pMatchingIcons);
	} while (myData.pCurrentMatchingElement != pMatchingElement && ((Icon*)myData.pCurrentMatchingElement->data)->pIconBuffer == NULL);
	
	if (myData.pCurrentMatchingElement != pMatchingElement)  // on complete le texte et on redessine.
	{
		Icon *pIcon = myData.pCurrentMatchingElement->data;
		if (pIcon->cCommand && *pIcon->cCommand != *myData.sCurrentText->str)  // cas d'une commande avec un tiret.
			myData.iNbValidCaracters = 0;
		cd_do_delete_invalid_caracters ();
		
		if (pIcon->cBaseURI != NULL)
		{
			gchar *cFile = g_path_get_basename (pIcon->cCommand);
			g_string_assign (myData.sCurrentText, cFile);
			g_free (cFile);
		}
		else
			g_string_assign (myData.sCurrentText, pIcon->cCommand);
		
		
		cd_do_load_pending_caracters ();
		
		// on arme l'animation de decalage.
		myData.iMatchingGlideCount = 10;  // on rembobine l'animation.
		myData.iPreviousMatchingOffset = myData.iCurrentMatchingOffset;  // on part du point courant.
		CairoDock *pParentDock = cairo_dock_search_dock_from_name (pIcon->cParentDockName);
		int iWidth, iHeight;
		cairo_dock_get_icon_extent (pIcon, &iWidth, &iHeight);
		if (iHeight != 0)
		{
			double fZoom = (double) g_pMainDock->container.iHeight/2 / iHeight;
			myData.iMatchingAimPoint += (bNext ? 1 : -1) * iWidth * fZoom;  // on cherche a atteindre le nouveau point.
		}
		
		// on repositionne les caracteres et on anime tout ca.
		cd_do_launch_appearance_animation ();
		cairo_dock_redraw_container (CAIRO_CONTAINER (g_pMainDock));
	}
}

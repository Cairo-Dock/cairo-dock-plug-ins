/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-stacks.h"
 
CD_APPLET_INCLUDE_MY_VARS

void cd_stacks_build_icons (void) {
	if (myConfig.bLocalDir) {
		myConfig.cMonitoredDirectory[0] = g_strdup_printf("/home/%s/.cairo-dock/stacks", g_getenv ("USER"));
	}
	
	if (myConfig.cMonitoredDirectory == NULL)
    return;
	
	myData.iIconOrder = 0;
	gint i=0, j=0;
	GList *pIconList = NULL; // ne nous appartiendra plus, donc ne pas desallouer.
	while (myConfig.cMonitoredDirectory[i] != NULL) { 
	  gchar *cFullURI = NULL, *cDirectory = g_strdup(myConfig.cMonitoredDirectory[i]);
		GList *pIconDirList = NULL;
		//On liste le dossier a surveiller
		cd_message("Stacks(%d) - Now Listing: %s", i, cDirectory); 
		if (strcmp (cDirectory, "_LocalDirectory_") == 0) {
			g_free (cDirectory);
			cDirectory = g_strdup_printf("/home/%s/.cairo-dock/stacks", g_getenv ("USER"));
		}
		
		if (! g_file_test (cDirectory, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_EXECUTABLE)) {
			cd_warning ("Attention : no such directory (%s)", cDirectory);
			i++;
			continue;
		}
		
		if (i > 0 && myConfig.bUseSeparator) {
			Icon *pSeparatorIcon = g_new0 (Icon, 1);
			pSeparatorIcon->iType = 1;
			pIconList = g_list_append (pIconList, pSeparatorIcon);
		}
		pIconDirList = cairo_dock_fm_list_directory (cDirectory, CAIRO_DOCK_FM_SORT_BY_NAME, 2 + j, myConfig.bHiddenFiles, &cFullURI);
		pIconList = g_list_concat (pIconList, pIconDirList);
		if (! cairo_dock_fm_add_monitor_full (cFullURI, TRUE, NULL, (CairoDockFMMonitorCallback) cd_stacks_update, NULL))
			cd_warning ("Attention : can't monitor files (%s)", cFullURI);
			
		if (myConfig.bLocalDir && i == 0)
			break; //Solution temporaire au bug
		
		g_free (cDirectory);
		j++; //Compte uniquement les dossiers réellement listés par l'applet.
		i++;
	}
	
	g_list_foreach (pIconList, (GFunc) cd_stacks_debug_icon, NULL);
	
	if (myConfig.bFilter)
		pIconList = cd_stacks_mime_filter(pIconList);
	
	if (myDock) {
		CD_APPLET_CREATE_MY_SUBDOCK (pIconList, myConfig.cRenderer)
	}
	else {
		myDesklet->icons = pIconList;
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Tree", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		gtk_widget_queue_draw (myDesklet->pWidget);  // utile ?
	}
		
	CD_APPLET_REDRAW_MY_ICON
}

void cd_stacks_destroy_icons (void) {
	if (myDock && myIcon->pSubDock != NULL) {
		cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
		myIcon->pSubDock = NULL;
	}
	else if (myDesklet && myDesklet->icons != NULL) {
		g_list_foreach (myDesklet->icons, (GFunc) cairo_dock_free_icon, NULL);
		g_list_free (myDesklet->icons);
		myDesklet->icons = NULL;
	}
}


void cd_stacks_debug_icon(Icon *pIcon) {
	pIcon->cWorkingDirectory = NULL;
	pIcon->fOrder = myData.iIconOrder;
	myData.iIconOrder++;
}

//A mettre en quelque part, on en aura surment besoin.
gchar* cd_get_path_from_uri (const gchar *cURI) {
	GError *erreur = NULL;
	gchar *cHostName = NULL, *cPath = g_filename_from_uri (cURI, &cHostName, &erreur);
	if (erreur != NULL) {
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
	}
	
	gchar *str = strrchr (cPath, '/');
	if (str != NULL)
		*str = '\0';
	
	cd_debug ("Path to file: %s", cPath);
	
	return cPath;
}

//Merci M. Fab pour ces 2 fonctions
int cairo_dock_compare_icons_order (Icon *icon1, Icon *icon2)
{
	int iOrder1 = cairo_dock_get_group_order (icon1);
	int iOrder2 = cairo_dock_get_group_order (icon2);
	if (iOrder1 < iOrder2)
		return -1;
	else if (iOrder1 > iOrder2)
		return 1;
	else
	{
		if (icon1->fOrder < icon2->fOrder)
			return -1;
		else if (icon1->fOrder > icon2->fOrder)
			return 1;
		else
			return 0;
	}
}

static int cairo_dock_compare_icons_name (Icon *icon1, Icon *icon2)
{
	if (icon1->acName == NULL)
		return -1;
	if (icon2->acName == NULL)
		return 1;
	gchar *cURI_1 = g_ascii_strdown (icon1->acName, -1);
	gchar *cURI_2 = g_ascii_strdown (icon2->acName, -1);
	int iOrder = strcmp (cURI_1, cURI_2);
	g_free (cURI_1);
	g_free (cURI_2);
	return iOrder;
}

void _placeIcon (Icon *pIcon, double fOrder, int iType) {
	pIcon->fOrder = fOrder;
	pIcon->iType = iType;
	GList *pStacksIconList = (myDock ? myIcon->pSubDock->icons : myDesklet->icons);
	pStacksIconList = g_list_remove (pStacksIconList, pIcon);
	pStacksIconList = g_list_insert_sorted (pStacksIconList, pIcon, (GCompareFunc) cairo_dock_compare_icons_order);
	
	if (myDock) {
		cairo_dock_insert_icon_in_dock (pIcon, myIcon->pSubDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, FALSE);
		cairo_dock_update_dock_size (myIcon->pSubDock);
	}
	else
		gtk_widget_queue_draw (myDesklet->pWidget);
}

void _sort_my_new_icon (const gchar *cURI, Icon *pAddedIcon) {
	if (cURI == NULL || pAddedIcon == NULL)
		return;
	
	gchar *cPath = cd_get_path_from_uri (cURI); //On récupère le path vers le fichier fraichement ajouter
	gint iType=0,i=0,j=0;
	while (myConfig.cMonitoredDirectory[i] != NULL) { //On compare le path avec tous les dossiers de myConfig.cMonitoredDirectory
		gchar *cDirectory = g_strdup(myConfig.cMonitoredDirectory[i]); 
		if (strcmp (cDirectory, "_LocalDirectory_") == 0) {
			g_free (cDirectory);
			cDirectory = g_strdup_printf("/home/%s/.cairo-dock/stacks", g_getenv ("USER"));
		}
		
		if (! g_file_test (cDirectory, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_EXECUTABLE)) {
			cd_warning ("Attention : no such directory (%s)", cDirectory);
			i++;
			continue;
		}
				
		if (strcmp (cDirectory, cPath) == 0) { //On a trouver sa famille d'icône
			iType = 2 + j; // On sait que le type de l'icône ayant un path précis a pour formule iType = 2 +j
			cd_debug ("Found type: %d", iType);
			break;
		}
		
		g_free (cDirectory);
		j++; i++;
	}
	
	if (iType == 0) { //Pas de type d'icône, on la rajoute a la fin de la liste...
		cd_warning ("Can't sort new file.");
		return;
	}
	
	GList *pStacksIconList = (myDock ? myIcon->pSubDock->icons : myDesklet->icons);
	Icon *pPreviousIcon = NULL, *pCurrentIcon = cairo_dock_get_last_icon_of_type (pStacksIconList, iType);
	
	if (pCurrentIcon == NULL) //Ne devrais pas arriver
		pCurrentIcon = cairo_dock_get_last_icon (pStacksIconList);
	
	if (cairo_dock_compare_icons_name (pCurrentIcon, pAddedIcon) < 0 || cairo_dock_compare_icons_name (pCurrentIcon, pAddedIcon) == 0) { //Notre icône doit se placer en dernier
		_placeIcon (pAddedIcon, pCurrentIcon->fOrder + 1, iType);
		cd_debug ("Placed After %s", pCurrentIcon->acName);
	}
	else { //On boucle pour chercher devant quelle icônes elle doit être
		while (1) {
			pPreviousIcon = pCurrentIcon;
			pCurrentIcon = cairo_dock_get_previous_icon (pStacksIconList, pCurrentIcon);
			if (pCurrentIcon == NULL) { //On a remonté toute la liste, Notre icône doit se placer en 1er
				_placeIcon (pAddedIcon, pPreviousIcon->fOrder - 0.01, iType);
				cd_debug ("Placed Before %s", pPreviousIcon->acName);
				break;
			}
			else if (cairo_dock_compare_icons_name (pCurrentIcon, pAddedIcon) < 0) { //Notre icône après la courante
				_placeIcon (pAddedIcon, pCurrentIcon->fOrder + 0.01, iType);
				cd_debug ("Placed After %s", pCurrentIcon->acName);
				break;
			}
		}
	}
}

gboolean _on_animation_complete (Icon *pAddedIcon) {
	//On retire l'icône et on met a jour le sous-dock
	GList *pStacksIconList = (myDock ? myIcon->pSubDock->icons : myDesklet->icons);
	pStacksIconList = g_list_remove (pStacksIconList, pAddedIcon);
	cairo_dock_free_icon (pAddedIcon);
	
	if (myDock) 
		cairo_dock_update_dock_size (myIcon->pSubDock);
	else
		gtk_widget_queue_draw (myDesklet->pWidget);
	
	return FALSE;
}

//A optimiser comme shortcuts - C'est fait M. Fab ^^
void cd_stacks_update (CairoDockFMEventType iEventType, const gchar *cURI, Icon *pIcon) {
	cd_debug("%s (%d %s)", __func__, iEventType, cURI);
	
	GList *pStacksIconList = (myDock ? myIcon->pSubDock->icons : myDesklet->icons);
	cd_get_path_from_uri (cURI);
	
	if (iEventType == CAIRO_DOCK_FILE_CREATED || iEventType == CAIRO_DOCK_FILE_MODIFIED) {
		//cd_debug ("On a ajouter un fichier");
		cairo_dock_fm_manage_event_on_file (iEventType, cURI, myIcon, 9); //On la rajoute
		Icon *pAddedIcon = cairo_dock_get_icon_with_base_uri (pStacksIconList, cURI);
		_sort_my_new_icon (cURI, pAddedIcon);
		if (myDock && pAddedIcon != NULL) {
			cairo_dock_show_subdock (myIcon, FALSE, myDock);
			cairo_dock_animate_icon (pAddedIcon, myIcon->pSubDock, CAIRO_DOCK_BOUNCE, 2);
		}
	}
	else { //Ne fonctionne pas si on passe par le dock pour supprimer le fichier, car l'icône est détaché avant notre fonction. Dommage!
		//cd_debug ("On a retirer un fichier");
		Icon *pAddedIcon = cairo_dock_get_icon_with_base_uri (pStacksIconList, cURI);
		if (myDock && pAddedIcon != NULL) {
			cairo_dock_show_subdock (myIcon, FALSE, myDock);
			cairo_dock_animate_icon (pAddedIcon, myIcon->pSubDock, CAIRO_DOCK_BLINK, 2);
			//Il faut attendre que l'animation se termine pour virer l'icône du sous-dock
			g_timeout_add (1500, (GSourceFunc) _on_animation_complete, (gpointer) pAddedIcon);
		}
		if (myDesklet)
			_on_animation_complete (pAddedIcon);
	}
	
	myData.iIconOrder = 0; //On rétablie le fOrdre normal des icônes (de 1 en 1)
	g_list_foreach (pStacksIconList, (GFunc) cd_stacks_debug_icon, NULL);
	
}

void cd_stacks_reload (void) {
	cd_debug("");
	cd_stacks_destroy_icons();
	cd_stacks_build_icons();
}

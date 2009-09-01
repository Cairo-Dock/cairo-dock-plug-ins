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
 

void cd_stacks_build_icons (void) {
	if (myConfig.bLocalDir) {
		myConfig.cMonitoredDirectory[0] = g_strdup_printf("%s/stacks", g_cCairoDockDataDir);
	}
	
	if (myConfig.cMonitoredDirectory == NULL)
		return;
	
	myData.iIconOrder = 1;
	gint i=0, j=0;
	GList *pIconList = NULL; // ne nous appartiendra plus, donc ne pas desallouer.
	while (myConfig.cMonitoredDirectory[i] != NULL) {
		gchar *cFullURI = NULL, *cDirectory = g_strdup(myConfig.cMonitoredDirectory[i]);
		GList *pIconDirList = NULL;
		//On liste le dossier a surveiller
		//cd_message("Stacks (%d) - Now Listing: %s", i, cDirectory); 
		if (strcmp (cDirectory, "_LocalDirectory_") == 0) {
			g_free (cDirectory);
			cDirectory = g_strdup_printf("%s/stacks", g_cCairoDockDataDir);
		}
		
		if (! g_file_test (cDirectory, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_EXECUTABLE)) {
			cd_warning ("Attention : no such directory (%s)", cDirectory);
			i++;
			continue;
		}
		
		pIconDirList = cairo_dock_fm_list_directory (cDirectory, CAIRO_DOCK_FM_SORT_BY_NAME, 2 + j, myConfig.bHiddenFiles, &cFullURI);
		
		if (i > 0 && myConfig.bUseSeparator && pIconDirList != NULL) {
			Icon *pSeparatorIcon = g_new0 (Icon, 1);
			pSeparatorIcon->iType = 1;
			pIconList = g_list_append (pIconList, pSeparatorIcon);
		}
		
		pIconList = g_list_concat (pIconList, pIconDirList);
		if (! cairo_dock_fm_add_monitor_full (cFullURI, TRUE, NULL, (CairoDockFMMonitorCallback) cd_stacks_update, NULL))
			cd_warning ("Attention : can't monitor file (%s)", cFullURI);
			
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
		CD_APPLET_CREATE_MY_SUBDOCK (pIconList, myConfig.cRenderer);
	}
	else {
		myDesklet->icons = pIconList;
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Tree", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		gtk_widget_queue_draw (myDesklet->pWidget);  // utile ?
	}
	
	gsize length = 0;
	GError *erreur = NULL;
	myData.pKeyFile = g_key_file_new (); //On ouvre le fichier de conf
	g_key_file_load_from_file (myData.pKeyFile, myData.cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL) {
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		return;
	}
	myData.cMonitoredDirectory = cairo_dock_get_string_list_key_value (myData.pKeyFile, "Configuration", "directory", TRUE, &length, NULL, NULL, NULL); //On recharge la liste
	g_key_file_free (myData.pKeyFile); //On ferme le fichier de conf
	
	myData.iNbAnimation = 0; //On reset le nombre d'animation
	CD_APPLET_REDRAW_MY_ICON;
}

void cd_stacks_destroy_icons (void) {
	cd_debug ("");
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


void cd_stacks_debug_icon (Icon *pIcon) {
	pIcon->cWorkingDirectory = NULL;
	pIcon->fOrder = myData.iIconOrder;
	myData.iIconOrder++;
}

//A mettre en quelque part, on en aura surment besoin.
gchar* cd_get_path_from_uri (const gchar *cURI) {
	if (cURI == NULL)
		return NULL;
	
	GError *erreur = NULL;
	gchar *cHostName = NULL, *cPath = g_strstr_len (cURI, -1, "://");
	if (cPath != NULL) {
		cPath = g_filename_from_uri (cURI, &cHostName, &erreur);
		if (erreur != NULL) {
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
		}
	}
	else
		cPath = cURI;
	
	gchar *str = strrchr (cPath, '/');
	if (str != NULL)
		*str = '\0';
	
	//cd_debug ("Path to file: %s", cPath);
	return cPath;
}

void _stacks_remove_one_icon (Icon *pAddedIcon) {
	//cd_debug ("Removing %s", pAddedIcon->acName);
	GList *pStacksIconList = (myDock ? myIcon->pSubDock->icons : myDesklet->icons);
	if (myDock)
		cairo_dock_detach_icon_from_dock (pAddedIcon, myIcon->pSubDock, FALSE);
	else
		pStacksIconList = g_list_remove (pStacksIconList, pAddedIcon);
		
	cairo_dock_free_icon (pAddedIcon);
	if (g_list_length (pStacksIconList) < 1) 
		cd_stacks_destroy_icons (); //plus d'icône a dessiner!
	
	if (myDock)
		cairo_dock_update_dock_size (myIcon->pSubDock);
	else
		gtk_widget_queue_draw (myDesklet->pWidget);
}

void _removeUselessSeparator (void) {
	cd_debug ("");
	GList *pStacksIconList = (myDock ? myIcon->pSubDock->icons : myDesklet->icons);
	if (pStacksIconList == NULL)
		return;
		
	GList *ic;
	Icon *icon=NULL, *prevIcon=NULL;
	for (ic = pStacksIconList; ic != NULL; ic = ic->next) {
		prevIcon = icon;
		icon = ic->data;
		if (prevIcon == NULL)
			continue;
			
		if (prevIcon->iType == 1 && icon->iType == 1) //Deux séparateur cote a cote
			_stacks_remove_one_icon (icon);
	}
	if (icon->iType == 1) //Dernière icône étant séparateur
		_stacks_remove_one_icon (icon);
}

void _placeIconWithSeparator (Icon *pAddedIcon, double fOrder, int iType, gboolean bUseSeparator) {
	pAddedIcon->fOrder = fOrder;
	pAddedIcon->iType = iType;
	GList *pStacksIconList = (myDock ? myIcon->pSubDock->icons : myDesklet->icons);
	if (bUseSeparator) {
		Icon *pSeparatorIcon = g_new0 (Icon, 1);
		pSeparatorIcon->iType = 1;
		pStacksIconList = g_list_append (pStacksIconList, pSeparatorIcon);
	}
		
	pStacksIconList = g_list_remove (pStacksIconList, pAddedIcon);
	pStacksIconList = g_list_insert_sorted (pStacksIconList, pAddedIcon, (GCompareFunc) cairo_dock_compare_icons_order);
		
	if (myDock) {
		if (g_list_length (pStacksIconList) == 1)  //Sinon on a pas l'icône qui saute joyeusement.
			CD_APPLET_CREATE_MY_SUBDOCK (pStacksIconList, myConfig.cRenderer);
			
		cairo_dock_insert_icon_in_dock (pAddedIcon, myIcon->pSubDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, FALSE);
		cairo_dock_update_dock_size (myIcon->pSubDock);
	}
	else
		gtk_widget_queue_draw (myDesklet->pWidget);
}

void _sort_my_new_icon (const gchar *cURI, Icon *pAddedIcon) {
	if (cURI == NULL || pAddedIcon == NULL)
		return;
	
	cd_debug ("");
	gchar *cPath = cd_get_path_from_uri (cURI); //On récupère le path vers le fichier fraichement ajouter
	gint iType=0,i=0,j=0;
	while (myConfig.cMonitoredDirectory[i] != NULL) { //On compare le path avec tous les dossiers de myConfig.cMonitoredDirectory
		gchar *cDirectory = g_strdup(myConfig.cMonitoredDirectory[i]); 
		if (strcmp (cDirectory, "_LocalDirectory_") == 0) {
			g_free (cDirectory);
			cDirectory = g_strdup_printf("%s/stacks", g_cCairoDockDataDir);
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
	
	GList *pStacksIconList = (myDock ? myIcon->pSubDock->icons : myDesklet->icons);
	if (pStacksIconList == NULL)
		return;
	Icon *pPreviousIcon = NULL, *pCurrentIcon = cairo_dock_get_last_icon_of_type (pStacksIconList, iType);
	if (pCurrentIcon == NULL) //1er fichier dont son répertoire est vide.
		pCurrentIcon = cairo_dock_get_last_icon (pStacksIconList);
	
	if (iType == 0) { //Pas de type d'icône, on la rajoute a la fin de la liste et on ajoute un séparateur s'il le faut.
		iType = 2 + j;
		_placeIconWithSeparator (pAddedIcon, pCurrentIcon->fOrder + 1, iType, (j > 0 ? myConfig.bUseSeparator : FALSE));
		return;
	}
	
	if (cairo_dock_compare_icons_name (pCurrentIcon, pAddedIcon) < 0 || cairo_dock_compare_icons_name (pCurrentIcon, pAddedIcon) == 0) {
		//cd_debug ("Notre icône doit se placer en dernier");
		if (strcmp (pCurrentIcon->cBaseURI, pAddedIcon->cBaseURI) == 0 && iType > 2) //1er icône d'un dossier vide, on rajoute un séparateur si necessaire
			_placeIconWithSeparator (pAddedIcon, pCurrentIcon->fOrder + 0.01, iType, myConfig.bUseSeparator);
		else
			_placeIconWithSeparator (pAddedIcon, pCurrentIcon->fOrder + 0.01, iType, FALSE);
		cd_debug ("Placed After %s", pCurrentIcon->acName);
	}
	else {
		//cd_debug ("On boucle pour chercher devant quelle icônes elle doit être");
		int iNBLoop = 0;
		while (1) {
			pPreviousIcon = pCurrentIcon;
			pCurrentIcon = cairo_dock_get_previous_icon (pStacksIconList, pCurrentIcon);
				
			if (pCurrentIcon == NULL) { //On a remonté toute la liste, Notre icône doit se placer en 1er
				_placeIconWithSeparator (pAddedIcon, pPreviousIcon->fOrder - 0.01, iType, FALSE);
				cd_debug ("Placed Before %s", pPreviousIcon->acName);
				break;
			}
			else if (cairo_dock_compare_icons_name (pCurrentIcon, pAddedIcon) < 0) { //Notre icône après la courante
				_placeIconWithSeparator (pAddedIcon, pCurrentIcon->fOrder + 0.01, iType, FALSE);
				cd_debug ("Placed After %s", pCurrentIcon->acName);
				break;
			}
			
			if (strcmp(pCurrentIcon->cBaseURI, pPreviousIcon->cBaseURI) == 0 && pCurrentIcon->fOrder == pPreviousIcon->fOrder) {
				//En cas d'ajout en masse, l'icône est ajouter en 1er et va bloquer ici. TODO Trouver une solution.
				break; //On va bouclé a l'infinie et ce n'est pas l'effet voulue
			}

		}
	}
}

gboolean _on_animation_complete (Icon *pAddedIcon) {
	//On retire l'icône et on met a jour le sous-dock
	_stacks_remove_one_icon (pAddedIcon);
	_removeUselessSeparator();
	return FALSE;
}

gboolean _reset_count_animation (void) {
	myData.iNbAnimation = 0;
	myData.iSidTimer = 0;
	return FALSE;
}

//A optimiser comme shortcuts - C'est fait M. Fab ^^
void cd_stacks_update (CairoDockFMEventType iEventType, const gchar *cRawURI, Icon *pIcon) {
	gchar *cURI = g_strdup (cRawURI);
	cairo_dock_remove_html_spaces (cURI);
	cd_debug("%s (%d on %s)", __func__, iEventType, cURI);
	
	GList *pStacksIconList = (myDock ? myIcon->pSubDock->icons : myDesklet->icons);
	
	if (iEventType == CAIRO_DOCK_FILE_CREATED) {
		cairo_dock_fm_manage_event_on_file (iEventType, cURI, myIcon, 35); //On la rajoute, personne n'aura 33 dossiers listés dans son stacks
		cd_debug ("On a ajouté un fichier");
		Icon *pAddedIcon = cairo_dock_get_icon_with_base_uri (pStacksIconList, cURI);
		if (!myConfig.bHiddenFiles && pAddedIcon != NULL) { //On ne veut pas des fichiers cachés!
			if (*pAddedIcon->acName == '.') {
				_stacks_remove_one_icon (pAddedIcon);
				return;
			}
		}
		_sort_my_new_icon (cURI, pAddedIcon);
		if (myDock && pAddedIcon != NULL) {
			cairo_dock_show_subdock (myIcon, FALSE, myDock);
			if (myData.iNbAnimation < 10) /*Le dock n'est pas prévu pour gérer autant d'animations, au dela il freeze.
			/ ---> pardon ???
			Avec > 20 icônes les animations se figent et c'est pas très jolie d'ou la limite
			Avec 10 j'ai juste la monté et je doit bougé la souris dans le sous dock pour que l'animation continue
			L'openGL reglèrera surment ca.*/
			/// oki je vois, effectivement sans acceleration materielle animer 20 icones ca devient limite.
				//cairo_dock_animate_icon (pAddedIcon, myIcon->pSubDock, CAIRO_DOCK_BOUNCE, 2);
				cairo_dock_request_icon_animation (pAddedIcon, myIcon->pSubDock, "bounce", 2);
			if (myData.iSidTimer != 0) {
				g_source_remove (myData.iSidTimer);
				myData.iSidTimer = 0;
			}
			myData.iSidTimer = g_timeout_add (2000, (GSourceFunc) _reset_count_animation, NULL);
			myData.iNbAnimation++;
		}
	}
	else if (iEventType == CAIRO_DOCK_FILE_MODIFIED) { //L'icône a déja été rajouter au dock
		cd_debug ("On a modifié un fichier");
		Icon *pModifiedIcon = cairo_dock_get_icon_with_base_uri (pStacksIconList, cURI);
		
		if (pModifiedIcon != NULL) {
			pModifiedIcon->cWorkingDirectory = NULL;
			cairo_dock_fm_manage_event_on_file (iEventType, cURI, myIcon, (pModifiedIcon != NULL ? pModifiedIcon->iType : 35)); 
		}
		/*Le dock se chargera tout seul de mettre a jour l'icône
		TODO fixer le bug qui fait planter le dock quand on télécharge un fichier avec FF dans un dossier surveillé.*/
		if (!myConfig.bHiddenFiles && pModifiedIcon != NULL) { //On ne veut pas des fichiers cachés!
			if (*pModifiedIcon->acName == '.') {
				_stacks_remove_one_icon (pModifiedIcon);
				return;
			}
		}
	}
	else { 
		cd_debug ("On a retiré un fichier"); //TODO On a un petit problème avec les espaces ici.
		Icon *pDeletedIcon = cairo_dock_get_icon_with_base_uri (pStacksIconList, cURI);
		if (myDock && pDeletedIcon != NULL) {
			pDeletedIcon->cWorkingDirectory = NULL;
			cairo_dock_show_subdock (myIcon, FALSE, myDock);
			//cairo_dock_animate_icon (pDeletedIcon, myIcon->pSubDock, CAIRO_DOCK_BLINK, 2);
			cairo_dock_request_icon_animation (pDeletedIcon, myIcon->pSubDock, "blink", 2);
			//Il faut attendre que l'animation se termine pour virer l'icône du sous-dock - 1.5sec est-ce suffisant?
			g_timeout_add (1500, (GSourceFunc) _on_animation_complete, (gpointer) pDeletedIcon);
		}
		if (myDesklet)
			_on_animation_complete (pDeletedIcon);
	}
	
	myData.iIconOrder = 1; //On rétablie le fOrdre normal des icônes (de 1 en 1)
	g_list_foreach (pStacksIconList, (GFunc) cd_stacks_debug_icon, NULL);
	
}

void cd_stacks_reload (void) {
	cd_debug ("Reloading stacks");
	cd_stacks_destroy_icons();
	cd_stacks_build_icons();
}

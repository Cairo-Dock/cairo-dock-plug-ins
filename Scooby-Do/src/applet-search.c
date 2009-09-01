/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-session.h"
#include "applet-listing.h"
#include "applet-search.h"

gboolean cd_do_fill_default_entry (CDEntry *pEntry)
{
	if (pEntry->cIconName && pEntry->pIconSurface == NULL)
	{
		cairo_t* pSourceContext = cairo_dock_create_context_from_container (CAIRO_CONTAINER (g_pMainDock));
		pEntry->pIconSurface = cairo_dock_create_surface_from_icon (pEntry->cIconName,
			pSourceContext,
			myDialogs.dialogTextDescription.iSize + 2,
			myDialogs.dialogTextDescription.iSize + 2);
		cairo_destroy (pSourceContext);
		return TRUE;
	}
	return FALSE;
}

static inline void _discard_results (CDBackend *pBackend)
{
	g_list_foreach (pBackend->pSearchResults, (GFunc) cd_do_free_entry, NULL);
	g_list_free (pBackend->pSearchResults);
	pBackend->pSearchResults = NULL;
	pBackend->iNbSearchResults = 0;
}

static void _launch_async_search (CDBackend *pBackend)
{
	pBackend->pSearchResults = pBackend->search (pBackend->cCurrentLocateText,
		pBackend->iLocateFilter,
		pBackend->pData,
		&pBackend->iNbSearchResults);
}

static gboolean _update_entries (CDBackend *pBackend)
{
	g_print ("%s ()\n", __func__);
	pBackend->bFoundNothing = FALSE;
	pBackend->bTooManyResults = FALSE;
	
	if (! cd_do_session_is_waiting_for_input () || myData.pListingHistory != NULL)  // on a quitte la session ou on a choisi une entree en cours de route.
	{
		g_print (" on a quitte la session ou on a choisi une entree en cours de route\n");
		_discard_results (pBackend);
		pBackend->cCurrentLocateText = NULL;
		pBackend->iLocateFilter = 0;
		return FALSE;
	}
	
	if (pBackend->iLocateFilter != myData.iCurrentFilter ||
		cairo_dock_strings_differ (pBackend->cCurrentLocateText, myData.sCurrentText->str))  // la situation a change entre le lancement de la tache et la mise a jour, on va relancer la recherche immediatement.
	{
		if ((pBackend->iLocateFilter & myData.iCurrentFilter) == pBackend->iLocateFilter &&
			pBackend->cCurrentLocateText &&
			strncmp (pBackend->cCurrentLocateText, myData.sCurrentText->str, strlen (pBackend->cCurrentLocateText)) == 0)  // c'est une sous-recherche.
		{
			g_print (" c'est une sous-recherche\n");
			if (pBackend->pSearchResults == NULL)  // on n'a rien trouve, on vide le listing et on ne la relance pas.
			{
				pBackend->bFoundNothing = TRUE;
				cd_do_remove_entries_from_listing (pBackend);
				return FALSE;
			}
			else  // la recherche a ete fructueuse, on regarde si on peut la filtrer ou s'il faut la relancer.
			{
				if (pBackend->iNbSearchResults < myConfig.iNbResultMax)  // on a des resultats mais pas trop, on les charge et on leur applique le filtre.
				{
					cd_do_filter_entries (pBackend->pSearchResults, pBackend->iNbSearchResults);
					cd_do_append_entries_to_listing (pBackend->pSearchResults, pBackend->iNbSearchResults);
					return FALSE;
				}
				else  // on a trop de resultats, on bache tout et on relance.
				{
					_discard_results (pBackend);
				}
			}
		}
		else  // c'est une nouvelle recherche, on bache tout et on relance.
		{
			g_print (" c'est une nouvelle recherche, on bache tout et on relance\n");
			_discard_results (pBackend);
			
			if (myData.pMatchingIcons != NULL || myData.sCurrentText->len == 0)  // avec le texte courant on a des applis, on quitte.
			{
				cd_do_hide_listing ();
				g_free (pBackend->cCurrentLocateText);
				pBackend->cCurrentLocateText = NULL;
				pBackend->iLocateFilter = 0;
				return FALSE;
			}
		}
		
		// on relance.
		g_print (" on relance\n");
		cd_do_set_status (D_("Searching ..."));
		pBackend->iLocateFilter = myData.iCurrentFilter;
		g_free (pBackend->cCurrentLocateText);
		pBackend->cCurrentLocateText = g_strdup (myData.sCurrentText->str);
		cairo_dock_relaunch_task_immediately (pBackend->pTask, 0);
	}
	else  // la situation n'a pas change, on montre les resultats.
	{
		cd_do_remove_entries_from_listing (pBackend);
		cd_do_append_entries_to_listing (pBackend->pSearchResults, pBackend->iNbSearchResults);
		pBackend->bTooManyResults = (pBackend->iNbSearchResults >= myConfig.iNbResultMax);
		pBackend->pLastShownResults = pBackend->pSearchResults;
		pBackend->iNbLastShownResults = pBackend->iNbSearchResults;
		pBackend->pSearchResults = NULL;
		pBackend->iNbSearchResults = 0;
	}
	return FALSE;
}
void cd_do_launch_backend (CDBackend *pBackend)
{
	g_print ("%s (%s)\n", __func__, pBackend->cName);
	// On initialise le backend si c'est son 1er appel.
	if (pBackend->iState == 0)
	{
		if (pBackend->init)
			pBackend->iState = (pBackend->init (&pBackend->pData) ? 1 : -1);
		else
			pBackend->iState = 1;
		
		if (pBackend->bIsThreaded && pBackend->search != NULL)
		{
			pBackend->pTask = cairo_dock_new_task (0,
				(CairoDockGetDataAsyncFunc) _launch_async_search,
				(CairoDockUpdateSyncFunc) _update_entries,
				pBackend);
		}
	}
	
	// On le lance.
	if (pBackend->pTask != NULL)  // asynchrone
	{
		if (cairo_dock_task_is_running (pBackend->pTask))  // on la laisse se finir, et lorsqu'elle aura fini, on la relancera avec le nouveau texte/filtre.
		{
			g_print (" on laisse la tache courante se finir\n");
			return ;
		}
		
		if (myData.pListingHistory != NULL || 
			((pBackend->iLocateFilter & myData.iCurrentFilter) == pBackend->iLocateFilter
			&& pBackend->cCurrentLocateText != NULL
			&& strncmp (pBackend->cCurrentLocateText,
					myData.sCurrentText->str,
					strlen (pBackend->cCurrentLocateText)) == 0
			&& ! pBackend->bTooManyResults))  // c'est une sous-recherche de la precedente qui etait fructueuse, ou un filtre sur un sous-listing.
		{
			g_print ("filtrage de la recherche\n");
			cd_do_filter_entries (pBackend->pLastShownResults, pBackend->iNbLastShownResults);
			
			
			
			
			cairo_dock_redraw_container (CAIRO_CONTAINER (myData.pListing));
		}
		else
		{
			pBackend->cCurrentLocateText = g_strdup (myData.sCurrentText->str);
			pBackend->iLocateFilter = myData.iCurrentFilter;
			cairo_dock_launch_task (pBackend->pTask);
		}
	}
	else if (! pBackend->bStaticResults || pBackend->pLastShownResults == NULL)  // synchrone
	{
		// on enleve ses precedents resultats.
		cd_do_remove_entries_from_listing (pBackend);
		
		// on lance la nouvelle recherche.
		int iNbEntries;
		GList *pEntries = pBackend->search (myData.sCurrentText->str,
			myData.iCurrentFilter,
			pBackend->pData,
			&iNbEntries);
		
		// on rajoute les nouveaux resultats.
		cd_do_append_entries_to_listing (pEntries, iNbEntries);
		pBackend->pLastShownResults = pEntries;
		pBackend->iNbLastShownResults = iNbEntries;
		pBackend->bTooManyResults = (iNbEntries >= myConfig.iNbResultMax);
	}
}

void cd_do_launch_all_backends (void)
{
	g_print ("%s ()\n", __func__);
	cd_do_show_listing ();
	g_list_foreach (myData.pBackends, (GFunc) cd_do_launch_backend, NULL);
}

void cd_do_stop_backend (CDBackend *pBackend)
{
	if (pBackend->pTask != NULL)
	{
		cairo_dock_stop_task (pBackend->pTask);
	}
	pBackend->pLastShownResults = NULL;
	pBackend->iNbLastShownResults = 0;
	g_free (pBackend->cCurrentLocateText);
	pBackend->cCurrentLocateText = NULL;
	pBackend->iLocateFilter = 0;
	pBackend->bFoundNothing = FALSE;
	pBackend->bTooManyResults = FALSE;
}

void cd_do_stop_all_backends (void)
{
	g_list_foreach (myData.pBackends, (GFunc) cd_do_stop_backend, NULL);
}



void cd_do_append_entries_to_listing (GList *pEntries, gint iNbEntries)
{
	g_print ("%s (%d)\n", __func__, iNbEntries);
	if (myData.pListing == NULL)
		return ;
	cd_do_show_listing ();
	
	myData.pListing->pEntries = g_list_concat (myData.pListing->pEntries, pEntries);
	myData.pListing->iNbEntries += iNbEntries;
	myData.pListing->iNbVisibleEntries += iNbEntries;
	
	cd_do_fill_listing_entries (myData.pListing);
}


void cd_do_remove_entries_from_listing (CDBackend *pBackend)
{
	g_print ("%s (%s)\n", __func__, pBackend->cName);
	g_return_if_fail (myData.pListing != NULL);
	if (pBackend->pLastShownResults == NULL)
		return ;
	
	GList *e;
	for (e = myData.pListing->pEntries; e != NULL; e = e->next)
	{
		if (e == pBackend->pLastShownResults)
			break ;
	}
	if (e == NULL)
	{
		return ;
	}
	
	GList *pLeftLink = NULL, *pRightLink = NULL;
	
	pLeftLink = pBackend->pLastShownResults->prev;
	if (pLeftLink)
	{
		pLeftLink->next = NULL;
		pBackend->pLastShownResults->prev = NULL;
	}
	
	CDEntry *pEntry;
	int i, j = 0;
	for (e = pBackend->pLastShownResults, i = 0; e != NULL && i < pBackend->iNbLastShownResults; e = e->next, i ++)
	{
		pEntry = e->data;
		if (! pEntry->bHidden)
			j ++;
	}
	myData.pListing->iNbEntries -= i;
	myData.pListing->iNbVisibleEntries -= j;
	
	pRightLink = e;
	if (pRightLink != NULL)
	{
		if (pLeftLink)
		{
			pLeftLink->next = pRightLink;
		}
		pRightLink->prev = pLeftLink;
	}
	
	pBackend->pLastShownResults = NULL;
	pBackend->iNbLastShownResults = 0;
	
	if (myData.pListing->iNbVisibleEntries > 0)
	{
		if (myData.pListing->iNbVisibleEntries >= myConfig.iNbResultMax)
			cd_do_set_status_printf ("> %d results", myConfig.iNbResultMax);
		else
			cd_do_set_status_printf ("%d %s", myData.pListing->iNbVisibleEntries, myData.pListing->iNbVisibleEntries > 1 ? D_("results") : D_("result"));
	}
	else
	{
		cd_do_set_status (D_("No result"));
	}
	
	cd_do_rewind_current_entry ();
	myData.pListing->iScrollAnimationCount = 0;
	myData.pListing->fAimedOffset = 0;
	myData.pListing->fPreviousOffset = myData.pListing->fCurrentOffset = 0;
	myData.pListing->sens = 1;
	myData.pListing->iTitleOffset = 0;
	myData.pListing->iTitleWidth = 0;
}


int cd_do_filter_entries (GList *pEntries, gint iNbEntries)
{
	g_print ("%s (%d)\n", __func__, iNbEntries);
	CDEntry *pEntry;
	int i, j = 0;
	GList *e;
	gchar *ext, *cHayStack;
	gchar *cPattern = g_ascii_strdown (myData.sCurrentText->str, -1);
	for (e = pEntries, i = 0; e != NULL && i < iNbEntries; e = e->next, i ++)
	{
		pEntry = e->data;
		if (pEntry->cName == NULL)
		{
			cd_warning ("l'entree n°%d/%d est vide !", i, iNbEntries);
			continue ;
		}
		ext = strrchr (pEntry->cName, '.');
		if (ext)
			ext ++;
		if (myData.iCurrentFilter & DO_MATCH_CASE)
		{
			cHayStack = pEntry->cName;
		}
		else
		{
			if (pEntry->cCaseDownName == NULL)
				pEntry->cCaseDownName = g_ascii_strdown (pEntry->cName, -1);
			cHayStack = pEntry->cCaseDownName;
		}
		if (g_strstr_len (cHayStack, -1, cPattern) != NULL &&
			(!(myData.iCurrentFilter & DO_TYPE_MUSIC)
			|| (ext
				&& (g_ascii_strcasecmp (ext, "mp3") == 0
				|| g_ascii_strcasecmp (ext, "ogg") == 0
				|| g_ascii_strcasecmp (ext, "wav") == 0))) &&
			(!(myData.iCurrentFilter & DO_TYPE_IMAGE)
			|| (ext
				&& (g_ascii_strcasecmp (ext, "jpg") == 0
				|| g_ascii_strcasecmp (ext, "jpeg") == 0
				|| g_ascii_strcasecmp (ext, "png") == 0))) &&
			(!(myData.iCurrentFilter & DO_TYPE_VIDEO)
			|| (ext
				&& (g_ascii_strcasecmp (ext, "avi") == 0
				|| g_ascii_strcasecmp (ext, "mkv") == 0
				|| g_ascii_strcasecmp (ext, "ogv") == 0
				|| g_ascii_strcasecmp (ext, "wmv") == 0
				|| g_ascii_strcasecmp (ext, "mov") == 0))) &&
			(!(myData.iCurrentFilter & DO_TYPE_TEXT)
			|| (ext
				&& (g_ascii_strcasecmp (ext, "txt") == 0
				|| g_ascii_strcasecmp (ext, "odt") == 0
				|| g_ascii_strcasecmp (ext, "doc") == 0))) &&
			(!(myData.iCurrentFilter & DO_TYPE_HTML)
			|| (ext
				&& (g_ascii_strcasecmp (ext, "html") == 0
				|| g_ascii_strcasecmp (ext, "htm") == 0))) &&
			(!(myData.iCurrentFilter & DO_TYPE_SOURCE)
			|| (ext
				&& (g_ascii_strcasecmp (ext, "c") == 0
				|| g_ascii_strcasecmp (ext, "h") == 0
				|| g_ascii_strcasecmp (ext, "cpp") == 0))))
		{
			pEntry->bHidden = FALSE;
			j ++;
		}
		else
		{
			pEntry->bHidden = TRUE;
		}
	}
	g_free (cPattern);
	
	return j;
}


void cd_do_activate_filter_option (int iNumOption)
{
	g_print ("%s (%d)\n", __func__, iNumOption);
	int iMaskOption = (1 << iNumOption);
	if (myData.iCurrentFilter & iMaskOption)  // on enleve l'option => ca fait (beaucoup) plus de resultats.
	{
		myData.iCurrentFilter &= (~iMaskOption);
	}
	else  // on active l'option => ca filtre les resultats courants.
	{
		myData.iCurrentFilter |= iMaskOption;
		if (myData.pListing && myData.pListing->pEntries == NULL)  // on rajoute une contrainte sur une recherche qui ne fournit aucun resultat => on ignore.
		{
			g_print ("useless\n");
			return ;
		}
	}
	g_print ("myData.iCurrentFilter  <- %d\n", myData.iCurrentFilter);
	
	// on cherche les nouveaux resultats correpondants.
	cd_do_launch_all_backends ();  // relance le locate seulement si necessaire.
}



void cd_do_show_current_sub_listing (void)
{
	g_print ("%s ()\n", __func__);
	if (myData.pListing->pCurrentEntry == NULL)
		return ;
	// on construit la liste des sous-entrees de l'entree courante.
	CDEntry *pEntry = myData.pListing->pCurrentEntry->data;
	GList *pNewEntries = NULL;
	int iNbNewEntries = 0;
	if (pEntry->list)
		pNewEntries = pEntry->list (pEntry, &iNbNewEntries);
	if (pNewEntries == NULL)
		return ;
	
	// on enleve le listing courant et on le conserve dans l'historique.
	CDListingBackup *pBackup = g_new0 (CDListingBackup, 1);
	pBackup->pEntries = myData.pListing->pEntries;
	pBackup->iNbEntries = myData.pListing->iNbEntries;
	pBackup->pCurrentEntry = myData.pListing->pCurrentEntry;
	
	if (myData.pListingHistory == NULL)  // on sauvegarde aussi le texte de la recherche principale.
	{
		myData.cSearchText = g_strdup (myData.sCurrentText->str);
	}
	g_string_assign (myData.sCurrentText, "");
	myData.iNbValidCaracters = 0;
	//cd_do_delete_invalid_caracters ();
	cd_do_free_char_list (myData.pCharList);
	myData.pCharList = NULL;
	myData.iTextWidth = 0;
	myData.iTextHeight = 0;
	cairo_dock_redraw_container (CAIRO_CONTAINER (g_pMainDock));
	
	myData.pListingHistory = g_list_prepend (myData.pListingHistory, pBackup);
	
	myData.pListing->pEntries = NULL;
	myData.pListing->iNbEntries = 0;
	myData.pListing->iNbVisibleEntries = 0;
	myData.pListing->pCurrentEntry = NULL;
	myData.pListing->iAppearanceAnimationCount = 0;
	myData.pListing->iCurrentEntryAnimationCount = 0;
	myData.pListing->iScrollAnimationCount = 0;
	myData.pListing->fAimedOffset = myData.pListing->fPreviousOffset = myData.pListing->fCurrentOffset = 0;
	
	// on montre les nouveaux resultats.
	cd_do_load_entries_into_listing (pNewEntries, iNbNewEntries);
}

void cd_do_show_previous_listing (void)
{
	g_print ("%s ()\n", __func__);
	if (myData.pListingHistory == NULL)  // on n'est pas dans un sous-listing.
		return ;
	
	// on recupere le precedent sous-listing.
	CDListingBackup *pBackup = myData.pListingHistory->data;
	myData.pListingHistory = g_list_delete_link (myData.pListingHistory, myData.pListingHistory);
	
	// on enleve le sous-listing courant.
	g_list_foreach (myData.pListing->pEntries, (GFunc)cd_do_free_entry, NULL);
	g_list_free (myData.pListing->pEntries);
	myData.pListing->pEntries = NULL;
	myData.pListing->iNbEntries = 0;
	myData.pListing->pCurrentEntry = NULL;
	myData.pListing->iAppearanceAnimationCount = 0;
	myData.pListing->iCurrentEntryAnimationCount = 0;
	myData.pListing->iScrollAnimationCount = 0;
	myData.pListing->fAimedOffset = myData.pListing->fPreviousOffset = myData.pListing->fCurrentOffset = 0;
	
	// on charge le nouveau sous-listing.
	cd_do_load_entries_into_listing (pBackup->pEntries, pBackup->iNbEntries);  // les entrees du backup appartiennent desormais au listing.
	g_free (pBackup);
	
	if (myData.pListingHistory == NULL)  // retour a la recherche principale.
	{
		g_string_assign (myData.sCurrentText, myData.cSearchText);
		g_free (myData.cSearchText);
		myData.cSearchText = NULL;
	}
}


void cd_do_filter_current_listing (void)
{
	g_print ("%s ()\n", __func__);
	if (myData.pListing == NULL || myData.pListing->pEntries == NULL)
		return ;
	
	myData.pListing->iNbVisibleEntries = cd_do_filter_entries (myData.pListing->pEntries, myData.pListing->iNbEntries);
	
	cd_do_fill_listing_entries (myData.pListing);  // on reprend a 0.
	
	cd_do_rewind_current_entry ();
	
	myData.pListing->iScrollAnimationCount = 0;
	myData.pListing->fAimedOffset = 0;
	myData.pListing->fPreviousOffset = myData.pListing->fCurrentOffset = 0;
	myData.pListing->sens = 1;
	myData.pListing->iTitleOffset = 0;
	myData.pListing->iTitleWidth = 0;
	cairo_dock_redraw_container (CAIRO_CONTAINER (myData.pListing));
}

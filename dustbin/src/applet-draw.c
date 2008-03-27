/**********************************************************************************

This file is a part of the cairo-dock project, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

**********************************************************************************/
#include <string.h>
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>
#include "cairo-dock.h"

#include "applet-notifications.h"
#include "applet-trashes-manager.h"
#include "applet-draw.h"

CD_APPLET_INCLUDE_MY_VARS


static int _cd_dustbin_compare_dustbins (const CdDustbin *pDustbin, const gchar *cDustbinPath)
{
	if (pDustbin->cPath == NULL)
		return -1;
	if (cDustbinPath == NULL)
		return 1;
	return (strcmp (pDustbin->cPath, cDustbinPath));
}
CdDustbin *cd_dustbin_find_dustbin_from_uri (const gchar *cDustbinPath)
{
	GList *pElement = g_list_find_custom (myData.pDustbinsList, cDustbinPath, (GCompareFunc) _cd_dustbin_compare_dustbins);
	if (pElement != NULL)
		return pElement->data;
	else
		return NULL;
}


void cd_dustbin_on_file_event (CairoDockFMEventType iEventType, const gchar *cURI, CdDustbin *pDustbin)
{
	g_return_if_fail (pDustbin != NULL);
	cd_message ("%s (%d,%d)", __func__, myData.iNbFiles, myData.iSize);
	gchar *cQuickInfo = NULL;
	switch (iEventType)
	{
		case CAIRO_DOCK_FILE_DELETED :
			cd_message ("1 dechet de moins");
			
			g_atomic_int_add (&pDustbin->iNbTrashes, -1);
			
			if (g_atomic_int_dec_and_test (&myData.iNbTrashes))  // devient nul.
			{
				cd_message ("la poubelle se vide");
				cd_dustbin_remove_all_messages ();
				g_atomic_int_set (&myData.iNbFiles, 0);  // inutile de calculer dans ce cas.
				g_atomic_int_set (&myData.iSize, 0);  // inutile de calculer dans ce cas.
				pDustbin->iNbFiles = 0;
				pDustbin->iSize = 0;
				cd_dustbin_draw_quick_info (FALSE);  // on redessine juste en-dessous.
				CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pEmptyBinSurface)
			}
			else if (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_NB_TRASHES)
			{
				cd_dustbin_draw_quick_info (TRUE);
			}
			else if (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_NB_FILES || myConfig.iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT)
			{
				cd_dustbin_add_message (NULL, pDustbin);
			}
		break ;
		
		case CAIRO_DOCK_FILE_CREATED :
			cd_message ("1 dechet de plus");
			
			g_atomic_int_add (&pDustbin->iNbTrashes, 1);
			
			if (g_atomic_int_exchange_and_add (&myData.iNbTrashes, 1) == 0)  // il etait nul avant l'incrementation.
			{
				cd_message ("la poubelle se remplit");
				CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pFullBinSurface)
			}
			if (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_NB_TRASHES)
			{
				cd_dustbin_draw_quick_info (TRUE);
			}
			else if (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_NB_FILES || myConfig.iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT)
			{
				cd_dustbin_add_message (g_strdup(cURI), pDustbin);
			}
		break ;
		
		default :
			break;
	}
	cd_message (" -> myData.iNbTrashes = %d", myData.iNbTrashes);
}


gboolean cd_dustbin_check_trashes (Icon *icon)
{
	//g_print ("%s ()\n", __func__);
	
	GDir *dir;
	int i = 0, iNewState = 0;
	const gchar *cFirstFileInBin;
	GError *erreur = NULL;
	CdDustbin *pDustbin;
	GList *pElement;
	for (pElement = myData.pDustbinsList; pElement != NULL; pElement = pElement->next)
	{
		pDustbin = pElement->data;
		iNewState += cd_dustbin_count_trashes (pDustbin->cPath);
	}
	//g_print (" myData.iNbTrashes : %d -> %d\n", myData.iNbTrashes, iNewState);
	if ((myData.iNbTrashes == -1) || (myData.iNbTrashes == 0 && iNewState != 0) || (myData.iNbTrashes != 0 && iNewState == 0))
	{
		myData.iNbTrashes = iNewState;
		double fMaxScale = (myDock != NULL ? 1 + g_fAmplitude : 1);
		cairo_save (myDrawContext);
		
		if (iNewState == 0)
		{
			cd_message (" -> on a vide la corbeille");
			g_return_val_if_fail (myData.pEmptyBinSurface != NULL, TRUE);
			CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pEmptyBinSurface)
		}
		else
		{
			cd_message (" -> on a rempli la corbeille");
			g_return_val_if_fail (myData.pFullBinSurface != NULL, TRUE);
			CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pFullBinSurface)
		}
		cairo_restore (myDrawContext);  // retour a l'etat initial.
		
		CD_APPLET_REDRAW_MY_ICON
	}
	
	return TRUE;
}


void cd_dustbin_draw_quick_info (gboolean bRedraw)
{
	if (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_NONE)
		return ;
	cd_message ("%s (%d)", __func__, myData.iNbTrashes);
	if (cd_dustbin_is_calculating ())
	{
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%s...", (myDesklet != NULL ? D_("calculating") : ""))
	}
	else if (myData.iNbTrashes == 0)
	{
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL)
	}
	else
	{
		if (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_NB_TRASHES)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%d%s", myData.iNbTrashes, (myDesklet != NULL ? D_(" trashes") : ""))
		}
		else if (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_NB_FILES)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%d%s", myData.iNbFiles, (myDesklet != NULL ? D_(" files") : ""))
		}
		else if (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT)
		{
			if (myData.iSize < 1024)
			{
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%db", myData.iSize)
			}
			else if (myData.iSize < (1 << 20))
			{
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%dK", (int) (myData.iSize>>10))
			}
			else if (myData.iSize < (1 << 30))
			{
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%dM", (int) (myData.iSize>>20))
			}
			else
			{
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%dG", (int) (myData.iSize>>30))
			}
		}
	}
	
	if (bRedraw)
	{
		CD_APPLET_REDRAW_MY_ICON
	}
}


void cd_dustbin_signal_full_dustbin (void)
{
	cd_message ("%s (%d,%d)", __func__, myConfig.iSizeLimit, myConfig.iGlobalSizeLimit);
	gboolean bOneDustbinFull = FALSE;
	CdDustbin *pDustbin;
	GList *pElement;
	for (pElement = myData.pDustbinsList; pElement != NULL; pElement = pElement->next)
	{
		pDustbin = pElement->data;
		if (myConfig.iSizeLimit != 0 && pDustbin->iSize > myConfig.iSizeLimit)
		{
			cairo_dock_show_temporary_dialog_with_icon ("%s is full !", myIcon, myDock, CD_DUSTBIN_DIALOG_DURATION, NULL, pDustbin->cPath);
			bOneDustbinFull = TRUE;
		}
	}
	if (! bOneDustbinFull && myConfig.iGlobalSizeLimit != 0 && myData.iSize > myConfig.iGlobalSizeLimit)
	{
		cairo_dock_show_temporary_dialog_with_icon ("I'm full !", myIcon, myDock, CD_DUSTBIN_DIALOG_DURATION, NULL);
	}
}

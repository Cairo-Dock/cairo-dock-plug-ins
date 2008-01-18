/**********************************************************************************

This file is a part of the cairo-dock project, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

**********************************************************************************/
#include <string.h>
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>
#include "cairo-dock.h"

#include "applet-notifications.h"
#include "applet-trashes-manager.h"
#include "applet-draw.h"

CD_APPLET_INCLUDE_MY_VARS

extern GList *my_pTrashDirectoryList;
extern int *my_pTrashState;
extern cairo_surface_t *my_pEmptyBinSurface;
extern cairo_surface_t *my_pFullBinSurface;
extern int my_iState;
extern gboolean my_bDisplayNbTrashes;
extern int my_iNbFiles, my_iSize;
extern int my_iQuickInfoType;
extern int my_iNbTrashes;
extern int my_iSizeLimit, my_iGlobalSizeLimit;

//static int s_iSidDelayMeasure = 0;


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
	GList *pElement = g_list_find_custom (my_pTrashDirectoryList, cDustbinPath, (GCompareFunc) _cd_dustbin_compare_dustbins);
	if (pElement != NULL)
		return pElement->data;
	else
		return NULL;
}


/*static gboolean _cd_dustbin_launch_measure_delayed (gpointer *data)
{
	cd_dustbin_add_message (data[0], data[1]);
	g_free (data);
	return FALSE;
}*/
void cd_dustbin_on_file_event (CairoDockFMEventType iEventType, const gchar *cURI, CdDustbin *pDustbin)
{
	g_return_if_fail (pDustbin != NULL);
	g_print ("%s (%d,%d)\n", __func__, my_iNbFiles, my_iSize);
	gchar *cQuickInfo = NULL;
	switch (iEventType)
	{
		case CAIRO_DOCK_FILE_DELETED :
			g_print ("1 dechet de moins\n");
			/*if (s_iSidDelayMeasure != 0)
			{
				g_source_remove (s_iSidDelayMeasure);
				s_iSidDelayMeasure = 0;
			}*/
			
			g_atomic_int_add (&pDustbin->iNbTrashes, -1);
			
			if (g_atomic_int_dec_and_test (&my_iNbTrashes))  // devient nul.
			{
				g_print ("la poubelle se vide\n");
				cd_dustbin_remove_all_messages ();
				g_atomic_int_set (&my_iNbFiles, 0);  // inutile de calculer dans ce cas.
				g_atomic_int_set (&my_iSize, 0);  // inutile de calculer dans ce cas.
				pDustbin->iNbFiles = 0;
				pDustbin->iSize = 0;
				cd_dustbin_draw_quick_info (FALSE);  // on redessine juste en-dessous.
				CD_APPLET_SET_SURFACE_ON_MY_ICON (my_pEmptyBinSurface)
			}
			else if (my_iQuickInfoType == CD_DUSTBIN_INFO_NB_TRASHES)
			{
				cd_dustbin_draw_quick_info (TRUE);
			}
			else if (my_iQuickInfoType == CD_DUSTBIN_INFO_NB_FILES || my_iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT)
			{
				/*gpointer *data = g_new (gpointer, 2);
				data[0] = NULL;
				data[1] = pDustbin;
				s_iSidDelayMeasure = g_timeout_add (500, (GSourceFunc) _cd_dustbin_launch_measure_delayed, data);  // on retarde le recalcul de 1 seconde, car il y'a probablement d'autres fichiers qui vont arriver.*/
				cd_dustbin_add_message (NULL, pDustbin);
			}
		break ;
		
		case CAIRO_DOCK_FILE_CREATED :
			g_print ("1 dechet de plus\n");
			
			g_atomic_int_add (&pDustbin->iNbTrashes, 1);
			
			if (g_atomic_int_exchange_and_add (&my_iNbTrashes, 1) == 0)  // il etait nul avant l'incrementation.
			{
				g_print ("la poubelle se remplit\n");
				CD_APPLET_SET_SURFACE_ON_MY_ICON (my_pFullBinSurface)
			}
			if (my_iQuickInfoType == CD_DUSTBIN_INFO_NB_TRASHES)
			{
				cd_dustbin_draw_quick_info (TRUE);
			}
			else if (my_iQuickInfoType == CD_DUSTBIN_INFO_NB_FILES || my_iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT)
			{
				cd_dustbin_add_message (g_strdup(cURI), pDustbin);
			}
		break ;
		
		default :
			break;
	}
	g_print (" -> my_iNbTrashes = %d\n", my_iNbTrashes);
}


gboolean cd_dustbin_check_trashes (Icon *icon)
{
	//g_print ("%s ()\n", __func__);
	
	GDir *dir;
	int i = 0, iNewState = 0;
	const gchar *cFirstFileInBin;
	GError *erreur = NULL;
	gchar *cOneDustbinPath;
	GList *pElement;
	for (pElement = my_pTrashDirectoryList; pElement != NULL; pElement = pElement->next)
	{
		cOneDustbinPath = pElement->data;
		dir = g_dir_open (cOneDustbinPath, 0, &erreur);
		if (erreur != NULL)
		{
			g_error_free (erreur);
			erreur = NULL;
		}
		else
		{
			cFirstFileInBin = g_dir_read_name (dir);
			if (cFirstFileInBin != NULL)
			{
				iNewState = CD_DUSTBIN_FULL;
				break ;
			}
			g_dir_close (dir);
		}
	}
	
	if (my_iState != iNewState)
	{
		my_iState = iNewState;
		double fMaxScale = 1 + g_fAmplitude;
		cairo_save (myDrawContext);
		
		if (iNewState == CD_DUSTBIN_EMPTY)
		{
			//g_print (" -> on a vide la corbeille\n");
			g_return_val_if_fail (my_pEmptyBinSurface != NULL, TRUE);
			cairo_dock_set_icon_surface_with_reflect (myDrawContext, my_pEmptyBinSurface, icon, myDock);
		}
		else
		{
			//g_print (" -> on a rempli la corbeille\n");
			g_return_val_if_fail (my_pFullBinSurface != NULL, TRUE);
			cairo_dock_set_icon_surface_with_reflect (myDrawContext, my_pFullBinSurface, icon, myDock);
		}
		cairo_restore (myDrawContext);  // retour a l'etat initial.
		
		cairo_dock_redraw_my_icon (icon, myDock);
	}
	
	return TRUE;
}


void cd_dustbin_draw_quick_info (gboolean bRedraw)
{
	if (my_iQuickInfoType == CD_DUSTBIN_INFO_NONE)
		return ;
	g_print ("%s (%d)\n", __func__, my_iNbTrashes);
	if (my_iNbTrashes == 0)
	{
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL)
	}
	else if (my_iNbTrashes < 0)
	{
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("...")
	}
	else
	{
		if (my_iQuickInfoType == CD_DUSTBIN_INFO_NB_TRASHES)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%d", my_iNbTrashes)
		}
		else if (my_iQuickInfoType == CD_DUSTBIN_INFO_NB_FILES)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%d", my_iNbFiles)
		}
		else if (my_iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT)
		{
			if (my_iSize < 1024)
			{
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%db", my_iSize)
			}
			else if (my_iSize < 1024*1024)
			{
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%dK", (int) (my_iSize>>10))
			}
			else if (my_iSize < 1024*1024*1024)
			{
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%dM", (int) (my_iSize>>20))
			}
			else
			{
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%dG", (int) (my_iSize>>30))
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
	g_print ("%s (%d,%d)\n", __func__, my_iSizeLimit, my_iGlobalSizeLimit);
	gboolean bOneDustbinFull = FALSE;
	CdDustbin *pDustbin;
	GList *pElement;
	for (pElement = my_pTrashDirectoryList; pElement != NULL; pElement = pElement->next)
	{
		pDustbin = pElement->data;
		if (my_iSizeLimit != 0 && pDustbin->iSize > my_iSizeLimit)
		{
			cairo_dock_show_temporary_dialog_with_icon ("%s is full !", myIcon, myDock, CD_DUSTBIN_DIALOG_DURATION, NULL, pDustbin->cPath);
			bOneDustbinFull = TRUE;
		}
	}
	if (! bOneDustbinFull && my_iGlobalSizeLimit != 0 && my_iSize > my_iGlobalSizeLimit)
	{
		cairo_dock_show_temporary_dialog_with_icon ("I'm full !", myIcon, myDock, CD_DUSTBIN_DIALOG_DURATION, NULL);
	}
}

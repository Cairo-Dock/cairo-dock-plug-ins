/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

**********************************************************************************/
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>
#include "cairo-dock.h"

#include "applet-struct.h"
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
extern int my_iQuickInfoValue;
extern int my_iQuickInfoType;
extern int my_iNbTrashes;

static int s_iSidCalculateDustbin = 0;

void cd_dustbin_on_file_event (CairoDockFMEventType iEventType, const gchar *cURI, Icon *pIcon)
{
	g_print ("%s (%d)\n", __func__, my_iQuickInfoValue);
	gchar *cQuickInfo = NULL;
	switch (iEventType)
	{
		case CAIRO_DOCK_FILE_DELETED :
			g_print ("1 dechet de moins\n");
			if (g_atomic_int_dec_and_test (&my_iNbTrashes))  // devient nul.
			{
				g_print ("la poubelle se vide\n");
				cd_dustbin_draw_quick_info (FALSE);
				CD_APPLET_SET_SURFACE_ON_MY_ICON (my_pEmptyBinSurface)
				cd_dustbin_draw_quick_info (0);
			}
			else
			{
				cd_dustbin_measure_all_dustbins (NULL);
				cd_dustbin_draw_quick_info (TRUE);
			}
		break ;
		
		case CAIRO_DOCK_FILE_CREATED :
			g_print ("1 dechet de plus\n");
			if (g_atomic_int_exchange_and_add (&my_iNbTrashes, 1) == 0)  // il etait nul avant l'incrementation.
			{
				g_print ("la poubelle se remplit\n");
				CD_APPLET_SET_SURFACE_ON_MY_ICON (my_pFullBinSurface)
			}
			my_iQuickInfoValue += cd_dustbin_measure_one_file (cURI, my_iQuickInfoType);
			cd_dustbin_draw_quick_info (TRUE);
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
	if (my_iQuickInfoValue == 0 || my_iQuickInfoType == CD_DUSTBIN_INFO_NONE)
	{
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL)
	}
	else
	{
		if (my_iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT)
		{
			g_print ("my_iQuickInfoValue : %db\n", my_iQuickInfoValue);
			if (my_iQuickInfoValue < 1e3)
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%db", my_iQuickInfoValue)
			else if (my_iQuickInfoValue < 1e6)
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%dK", (int) (my_iQuickInfoValue/1e3))
			else if (my_iQuickInfoValue < 1e9)
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%dM", (int) (my_iQuickInfoValue/1e6))
			else
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%dG", (int) (my_iQuickInfoValue/1e9))
		}
		else
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%d", my_iQuickInfoValue)
		}
	}
	
	if (bRedraw)
	{
		CD_APPLET_REDRAW_MY_ICON
	}
}

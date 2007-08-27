
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>
#include "cairo-dock.h"

#include "dustbin-struct.h"
#include "dustbin-menu-functions.h"
#include "dustbin-draw.h"


extern CairoDock *my_dustbin_pDock;
extern gchar **my_dustbin_cTrashDirectoryList;
extern int *my_dustbin_pTrashState;
extern GtkWidget *my_dustbin_pWidget;
extern cairo_t *my_dustbin_pCairoContext;
extern cairo_surface_t *my_dustbin_pEmptyBinSurface;
extern cairo_surface_t *my_dustbin_pFullBinSurface;
extern GtkWidget **my_dustbin_pShowToggleList;
extern GtkWidget **my_dustbin_pDeleteToggleList;
extern int my_dustbin_iState;


gboolean cd_dustbin_check_trashes (Icon *icon)
{
	//g_print ("%s ()\n", __func__);
	
	GDir *dir;
	int i = 0, iNewState = 0;
	const gchar *cFirstFileInBin;
	GError *erreur = NULL;
	while (my_dustbin_cTrashDirectoryList[i] != NULL)
	{
		dir = g_dir_open (my_dustbin_cTrashDirectoryList[i], 0, &erreur);
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
				//g_print ("%s est rempli\n", my_dustbin_cTrashDirectoryList[i]);
				if (my_dustbin_pTrashState[i] != CD_DUSTBIN_FULL)
				{
					my_dustbin_pTrashState[i] = CD_DUSTBIN_FULL;
					g_signal_handlers_block_matched (G_OBJECT (my_dustbin_pShowToggleList[i]), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (void*) cd_dustbin_show_trash, NULL);
					g_signal_handlers_block_matched (G_OBJECT (my_dustbin_pDeleteToggleList[i]), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (void*) cd_dustbin_delete_trash, NULL);
					gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (my_dustbin_pShowToggleList[i]), TRUE);
					gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (my_dustbin_pDeleteToggleList[i]), TRUE);
					g_signal_handlers_unblock_matched (G_OBJECT (my_dustbin_pShowToggleList[i]), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (void*) cd_dustbin_show_trash, NULL);
					g_signal_handlers_unblock_matched (G_OBJECT (my_dustbin_pDeleteToggleList[i]), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (void*) cd_dustbin_delete_trash, NULL);
				}
			}
			else
			{
				//g_print ("%s est vide\n", my_dustbin_cTrashDirectoryList[i]);
				if (my_dustbin_pTrashState[i] != CD_DUSTBIN_EMPTY)
				{
					my_dustbin_pTrashState[i] = CD_DUSTBIN_EMPTY;
					g_signal_handlers_block_matched (G_OBJECT (my_dustbin_pShowToggleList[i]), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (void*) cd_dustbin_show_trash, NULL);
					g_signal_handlers_block_matched (G_OBJECT (my_dustbin_pDeleteToggleList[i]), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (void*) cd_dustbin_delete_trash, NULL);
					gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (my_dustbin_pShowToggleList[i]), FALSE);
					gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (my_dustbin_pDeleteToggleList[i]), FALSE);
					g_signal_handlers_unblock_matched (G_OBJECT (my_dustbin_pShowToggleList[i]), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (void*) cd_dustbin_show_trash, NULL);
					g_signal_handlers_unblock_matched (G_OBJECT (my_dustbin_pDeleteToggleList[i]), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (void*) cd_dustbin_delete_trash, NULL);
				}
			}
			iNewState += my_dustbin_pTrashState[i];
			
			g_dir_close (dir);
		}
		i ++;
	}
	
	if (my_dustbin_iState != iNewState)
	{
		my_dustbin_iState = iNewState;
		double fMaxScale = 1 + g_fAmplitude;
		cairo_save (my_dustbin_pCairoContext);
		
		cairo_set_source_rgba (my_dustbin_pCairoContext, 0.0, 0.0, 0.0, 0.0);
		cairo_set_operator (my_dustbin_pCairoContext, CAIRO_OPERATOR_SOURCE);
		cairo_paint (my_dustbin_pCairoContext);
		
		cairo_set_operator (my_dustbin_pCairoContext, CAIRO_OPERATOR_OVER);
		
		if (iNewState == CD_DUSTBIN_EMPTY)
		{
			//g_print (" -> on a vide la corbeille\n");
			g_return_val_if_fail (my_dustbin_pEmptyBinSurface != NULL, TRUE);
			cairo_set_source_surface (my_dustbin_pCairoContext,
				my_dustbin_pEmptyBinSurface,
				0,
				0);
		}
		else
		{
			//g_print (" -> on a rempli la corbeille\n");
			g_return_val_if_fail (my_dustbin_pFullBinSurface != NULL, TRUE);
			cairo_set_source_surface (my_dustbin_pCairoContext,
				my_dustbin_pFullBinSurface,
				0,
				0);
		}
		cairo_paint (my_dustbin_pCairoContext);
		cairo_restore (my_dustbin_pCairoContext);
		if (! g_pMainDock->bAtBottom || ! g_bAutoHide)
			cairo_dock_redraw_my_icon (icon, my_dustbin_pDock);
	}
	
	return TRUE;
}


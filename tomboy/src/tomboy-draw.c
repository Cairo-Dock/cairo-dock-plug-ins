#include "string.h"
#include <glib/gi18n.h>

#include "tomboy-struct.h"
#include "tomboy-draw.h"


void load_all_surfaces(void)
{
	if (myData.pSurfaceDefault != NULL)
		cairo_surface_destroy (myData.pSurfaceDefault);
	if (myData.pSurfaceNote != NULL)
		cairo_surface_destroy (myData.pSurfaceNote);
	
	if (myDock)  // en mode desklet, on ne peut pas charger les surfaces car on n'a pas leur encore leur taille.
	{
		//Chargement de default.svg
		if (myConfig.cIconDefault != NULL)
		{
			gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cIconDefault);
			myData.pSurfaceDefault = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
			g_free (cUserImagePath);
		}
		else
		{
			myData.pSurfaceDefault = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (MY_APPLET_SHARE_DATA_DIR"/default.svg");
		}
		
		//Chargement de note.svg
		myData.pSurfaceNote = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (MY_APPLET_SHARE_DATA_DIR"/note.svg");
	}
	else
	{
		myData.pSurfaceDefault = NULL;
		myData.pSurfaceNote = NULL;
	}
}

void update_icon(void)
{
	if (myDesklet)
		return ;
	if(myData.opening)
	{
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", g_hash_table_size (myData.hNoteTable));
		CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceDefault);
	}
	else
	{
		CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cIconClose, "close.svg");
	}
}


static gboolean _cd_tomboy_reset_quick_info (gpointer data)
{
	CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", g_hash_table_size (myData.hNoteTable));
	CD_APPLET_REDRAW_MY_ICON;
	myData.iSidResetQuickInfo = 0;
	return FALSE;
}

void cd_tomboy_show_results (GList *pIconsList)
{
	//\_______________ On marque les icones du resultat.
	cd_tomboy_reset_icon_marks (FALSE);
	
	int iNbResults = 0;
	Icon *icon;
	GList *ic;
	for (ic = pIconsList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		icon->bHasIndicator = TRUE;
		iNbResults ++;
	}
	
	//\_______________ On les montre.
	if (myDock)
	{
		cairo_dock_show_subdock (myIcon, myDock, FALSE);
		cairo_dock_redraw_container (CAIRO_CONTAINER (myIcon->pSubDock));
	}
	else
		cairo_dock_redraw_container (myContainer);
	
	//\_______________ On affiche le resultat.
	if (myDock)
	{
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d %s", iNbResults, iNbResults > 1 ? D_("results") : D_("result"));
		if (myData.iSidResetQuickInfo != 0)
			g_source_remove (myData.iSidResetQuickInfo);
		myData.iSidResetQuickInfo = g_timeout_add_seconds (5, _cd_tomboy_reset_quick_info, NULL);
	}
	else
	{
		cairo_dock_show_temporary_dialog_with_icon ("%d %s", myDesklet->icons->data, myContainer, 3000, MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE, iNbResults, iNbResults > 1 ? D_("results") : D_("result"));
	}
}

void cd_tomboy_reset_icon_marks (gboolean bForceRedraw)
{
	GList *pIconsList = CD_APPLET_MY_ICONS_LIST;
	Icon *icon;
	GList *ic;
	for (ic = pIconsList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		icon->bHasIndicator = FALSE;
	}
	
	if (bForceRedraw)
	{
		if (myDock)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", g_hash_table_size (myData.hNoteTable));
			CD_APPLET_REDRAW_MY_ICON;
		}
		cairo_dock_redraw_container (CD_APPLET_MY_ICONS_LIST_CONTAINER);
	}
}


void cd_tomboy_draw_content_on_icon (cairo_t *pIconContext, Icon *pIcon)
{
	int w, h;
	cairo_dock_get_icon_extent (pIcon, CD_APPLET_MY_ICONS_LIST_CONTAINER, &w, &h);
	const int iNeedleOffset = 40./200*h;  // on laisse de la place pour l'aiguille de la punaise.
	gchar **cLines = g_strsplit (pIcon->cClass, "\n", -1);
	if (cLines == NULL)
		return ;
	
	cairo_set_operator (pIconContext, CAIRO_OPERATOR_OVER);
	cairo_set_source_rgb (pIconContext, myConfig.fTextColor[0], myConfig.fTextColor[1], myConfig.fTextColor[2]);
	//cairo_set_line_width (pIconContext, 4.);

	cairo_select_font_face (pIconContext,
		"sans",
		CAIRO_FONT_SLANT_NORMAL,
		CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (pIconContext, 12.);  // police 12 au zoom maximal.
	cairo_text_extents_t textExtents;
	cairo_text_extents (pIconContext, cLines[0], &textExtents);
	g_print (" + %s...\n", cLines[0]);
	
	int i = 1, j = 1;
	while (cLines[i] != NULL && iNeedleOffset+j*textExtents.height < h)
	{
		if (*cLines[i] != '\0')  // on saute les lignes vides.
		{
			cairo_move_to (pIconContext,
				0,
				iNeedleOffset+j*textExtents.height);
			cairo_show_text (pIconContext, cLines[i]);
			j ++;
		}
		i ++;
	}
	g_strfreev (cLines);
	
	if (g_bUseOpenGL)
		cairo_dock_update_icon_texture (pIcon);
	else if (myDock)
		cairo_dock_add_reflection_to_icon (pIconContext, pIcon, CD_APPLET_MY_ICONS_LIST_CONTAINER);
}


void cd_tomboy_draw_content_on_all_icons (void)
{
	g_print ("%s ()\n", __func__);
	Icon *icon;
	GList *ic, *pList = CD_APPLET_MY_ICONS_LIST;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->cClass != NULL)
		{
			cairo_t *pIconContext = cairo_create (icon->pIconBuffer);
			cd_tomboy_draw_content_on_icon (pIconContext, icon);
			if (g_bUseOpenGL)
				cairo_dock_update_icon_texture (icon);
			else if (myDock)
				cairo_dock_add_reflection_to_icon (pIconContext, icon, CD_APPLET_MY_ICONS_LIST_CONTAINER);
			cairo_destroy (pIconContext);
		}
	}
}

void cd_tomboy_reload_desklet_renderer (void)
{
	CD_APPLET_SET_DESKLET_RENDERER ("Slide");
	
	cd_tomboy_draw_content_on_all_icons ();
}

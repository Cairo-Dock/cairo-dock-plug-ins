#include "string.h"
#include <glib/gi18n.h>

#include "tomboy-struct.h"
#include "tomboy-draw.h"



void load_all_surfaces(void)
{
	//Chargement de default.svg
	if (myData.pSurfaceDefault != NULL)
		cairo_surface_destroy (myData.pSurfaceDefault);
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
	if (myData.pSurfaceNote != NULL)
		cairo_surface_destroy (myData.pSurfaceNote);
	myData.pSurfaceNote = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (MY_APPLET_SHARE_DATA_DIR"/note.svg");
}

void update_icon(void)
{
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


void cd_tomboy_mark_icons (GList *pIconsList, gboolean bForceRedraw)
{
	Icon *icon;
	GList *ic;
	for (ic = pIconsList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		icon->bHasIndicator = TRUE;
	}
	if (bForceRedraw)
		gtk_widget_queue_draw (myContainer->pWidget);
}

void cd_tomboy_reset_icon_marks (gboolean bForceRedraw)
{
	GList *pIconsList = (myDock ? (myIcon->pSubDock ? myIcon->pSubDock->icons : NULL) : myDesklet->icons);
	Icon *icon;
	GList *ic;
	for (ic = pIconsList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		icon->bHasIndicator = FALSE;
	}
	if (bForceRedraw)
		gtk_widget_queue_draw (myContainer->pWidget);
}


void cd_tomboy_draw_content_on_icon (cairo_t *pIconContext, Icon *pIcon, gchar *cNoteContent)
{
	const int iNeedleOffset = 8 * (1+g_fAmplitude);  // on laisse 8 pixels pour l'aiguille.
	gchar **cLines = g_strsplit (cNoteContent, "\n", -1);
	
	cairo_set_operator (pIconContext, CAIRO_OPERATOR_OVER);
	cairo_set_source_rgb (pIconContext, 1.0f, 0.5f, 0.0f);
	cairo_set_line_width (pIconContext, 4.);
	
	cairo_select_font_face (pIconContext,
		"sans",
		CAIRO_FONT_SLANT_NORMAL,
		CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (pIconContext, 12.0f);  // police 14 au zoom maximal.
	cairo_text_extents_t textExtents;
	cairo_text_extents (pIconContext, cLines[0], &textExtents);
	
	int i = 1, j = 1;
	while (cLines[i] != NULL && iNeedleOffset+j*textExtents.height < myIcon->fHeight * (1+g_fAmplitude))
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
	cairo_dock_add_reflection_to_icon (pIconContext, pIcon, (myDock ? CAIRO_CONTAINER (myIcon->pSubDock) : myContainer));
	if (g_bUseOpenGL)
	  cairo_dock_update_icon_texture (pIcon);
}

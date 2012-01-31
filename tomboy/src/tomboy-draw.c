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

#include "string.h"
#include <glib/gi18n.h>

#include "tomboy-struct.h"
#include "tomboy-draw.h"


void cd_tomboy_load_note_surface (int iWidth, int iHeight)
{
	if (myData.pSurfaceNote != NULL)
	{
		if (myData.iNoteWidth != iWidth || myData.iNoteHeight != iHeight)
		{
			cairo_surface_destroy (myData.pSurfaceNote);
			myData.pSurfaceNote = NULL;
		}
	}
	if (myData.pSurfaceNote == NULL)
	{
		myData.pSurfaceNote = cairo_dock_create_surface_from_image_simple (myConfig.cNoteIcon != NULL ? myConfig.cNoteIcon : MY_APPLET_SHARE_DATA_DIR"/note.svg",
			iWidth,
			iHeight);
	}
}

void cd_tomboy_update_icon (void)
{
	if (myDesklet)
		return ;
	if(myData.bIsRunning)
	{
		if (myData.iIconState != 1)
		{
			myData.iIconState = 1;
			CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cIconDefault, "default.svg");
		}
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", g_hash_table_size (myData.hNoteTable));
	}
	else if (myData.dbus_enable)
	{
		if (myData.iIconState != 2)
		{
			myData.iIconState = 2;
			CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cIconClose, "close.svg");
		}
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
	}
	else
	{
		if (myData.iIconState != 3)
		{
			myData.iIconState = 3;
			CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cIconBroken, "broken.svg");
		}
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
	}
	CD_APPLET_REDRAW_MY_ICON;
}


void cd_tomboy_draw_content_on_icon (cairo_t *pIconContext, Icon *pIcon)
{
	if (pIcon->cClass == NULL || *pIcon->cClass == '\0')  // note vide => rien a afficher.
		return ;
	int w, h;
	cairo_dock_get_icon_extent (pIcon, &w, &h);
	const int iNeedleOffset = 42./200*h;  // on laisse de la place pour l'aiguille de la punaise.
	gchar **cLines = g_strsplit (pIcon->cClass, "\n", -1);
	
	cairo_set_operator (pIconContext, CAIRO_OPERATOR_OVER);
	cairo_set_source_rgb (pIconContext, myConfig.fTextColor[0], myConfig.fTextColor[1], myConfig.fTextColor[2]);

	cairo_select_font_face (pIconContext,
		"sans",
		CAIRO_FONT_SLANT_NORMAL,
		CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (pIconContext, (myDock ? 14. : 12.));  // police 12 au zoom maximal.
	cairo_text_extents_t textExtents;
	cairo_text_extents (pIconContext, cLines[0], &textExtents);  // on recupere la hauteur d'une ligne.
	
	int i = 1, j = 1;
	while (cLines[i] != NULL && iNeedleOffset+j*textExtents.height < h)
	{
		if (*cLines[i] != '\0')  // on saute les lignes vides.
		{
			cairo_move_to (pIconContext,
				12./200*h,  // on laisse un peu de place pour ne pas deborder sur la gauche.
				iNeedleOffset+j*(textExtents.height+2));
			cairo_show_text (pIconContext, cLines[i]);
			j ++;
		}
		i ++;
	}
	g_strfreev (cLines);
	
	if (g_bUseOpenGL)
		cairo_dock_update_icon_texture (pIcon);
	/**else if (myDock)
		cairo_dock_add_reflection_to_icon (pIcon, CD_APPLET_MY_ICONS_LIST_CONTAINER);*/
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
		cairo_dock_show_subdock (myIcon, myDock);
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
		cairo_dock_show_temporary_dialog_with_icon_printf ("%d %s",
			pIconsList ? pIconsList->data : myDesklet->icons->data,
			myContainer,
			myConfig.iDialogDuration,
			"same icon",
			iNbResults,
			iNbResults > 1 ? D_("results") : D_("result"));
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

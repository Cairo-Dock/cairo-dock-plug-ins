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

#include <string.h>
#include <math.h>
#include <cairo-dock.h>

#include "applet-renderer-text.h"


CDTextParameters *rendering_configure_text (CairoDialog *pDialog, gpointer *pConfig)
{
	cd_debug ("");
	CDTextParameters *pText = g_new0 (CDTextParameters, 1);
	
	gchar *cInitialText = NULL;
	if (pConfig != NULL)
	{
		cairo_dock_copy_label_description (&pText->textDescription, (CairoDockLabelDescription *) pConfig[0]);
		cInitialText = (gchar *) pConfig[1];
	}
	
	int iTextWidth, iTextHeight;
	double fTextXOffset, fTextYOffset;
	if (cInitialText != NULL)
	{
		pText->pTextSurface = cairo_dock_create_surface_from_text (cInitialText,
			&pText->textDescription,
			&iTextWidth, &iTextHeight);
	}
	
	
	return pText;
}


void rendering_free_text_data (CairoDialog *pDialog)
{
	cd_debug ("");
	CDTextParameters *pText = (CDTextParameters *) pDialog->pRendererData;
	if (pText == NULL)
		return ;
	
	cairo_surface_destroy (pText->pTextSurface);
	
	g_free (pText);
	pDialog->pRendererData = NULL;
}


void rendering_draw_text_in_dialog (cairo_t *pCairoContext, CairoDialog *pDialog, double fAlpha)
{
	CDTextParameters *pText = (CDTextParameters *) pDialog->pRendererData;
	if (pText == NULL)
		return ;
	
	cairo_set_source_surface (pCairoContext,
		pText->pTextSurface,
		pDialog->iLeftMargin,
		(pDialog->container.bDirectionUp ? 
			pDialog->iTopMargin + pDialog->iMessageHeight :
			pDialog->container.iHeight - (pDialog->iTopMargin + pDialog->iBubbleHeight) + pDialog->iMessageHeight));
	if (fAlpha != 0)
		cairo_paint_with_alpha (pCairoContext, fAlpha);
	else
		cairo_paint (pCairoContext);
}


void rendering_update_text (CairoDialog *pDialog, gpointer *pNewData)
{
	CDTextParameters *pText = (CDTextParameters *) pDialog->pRendererData;
	if (pText == NULL)
		return ;
	
	gchar *cNewText = (gchar *) pNewData;
	
	int iTextWidth, iTextHeight;
	double fTextXOffset, fTextYOffset;
	cairo_surface_destroy (pText->pTextSurface);
	pText->pTextSurface = NULL;
	
	pText->pTextSurface = cairo_dock_create_surface_from_text (cNewText,
		&pText->textDescription,
		&iTextWidth, &iTextHeight);
	
	if (iTextWidth > pDialog->iInteractiveWidth || iTextHeight > pDialog->iInteractiveHeight)
		gtk_widget_set_size_request (pDialog->pInteractiveWidget, iTextWidth, iTextHeight);
}


void rendering_register_text_dialog_renderer (void)
{
	CairoDialogRenderer *pRenderer = g_new0 (CairoDialogRenderer, 1);
	pRenderer->render = rendering_draw_text_in_dialog ;
	pRenderer->configure = (CairoDialogConfigureRendererFunc)rendering_configure_text;
	pRenderer->free_data = rendering_free_text_data;
	pRenderer->update = rendering_update_text;
	
	cairo_dock_register_dialog_renderer (MY_APPLET_TEXT_DIALOG_RENDERER_NAME, pRenderer);
}



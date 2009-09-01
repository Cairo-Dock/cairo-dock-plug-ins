
#ifndef __DIALOG_RENDERING_DIALOG_TEXT__
#define __DIALOG_RENDERING_DIALOG_TEXT__

#include "cairo.h"

#define MY_APPLET_TEXT_DIALOG_RENDERER_NAME "Text"


typedef struct {
	CairoDockLabelDescription textDescription;
	cairo_surface_t *pTextSurface;
	gdouble fImageWidth;
	gdouble fImageHeight;
	} CDTextParameters;


CDTextParameters *rendering_configure_text (CairoDialog *pDialog, cairo_t *pSourceContext, gpointer *pConfig);

void rendering_free_text_data (CairoDialog *pDialog);

void rendering_draw_text_in_dialog (cairo_t *pCairoContext, CairoDialog *pDialog, double fAlpha);

void rendering_update_text (CairoDialog *pDialog, gpointer *pNewData);

void rendering_register_text_dialog_renderer (void);


#endif

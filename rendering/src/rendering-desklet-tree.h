
#ifndef __RENDERING_DESKLET_TREE__
#define  __RENDERING_DESKLET_TREE__

#include "cairo.h"

#define MY_APPLET_TREE_DESKLET_RENDERER_NAME "Tree"


typedef struct {
	gint iNbIconsInTree;
	gint iNbBranches;
	gdouble fTreeWidthFactor;
	gdouble fTreeHeightFactor;
	cairo_surface_t *pBrancheSurface[2];
	} CDTreeParameters;

CDTreeParameters *rendering_load_tree_data (CairoDockDesklet *pDesklet, cairo_t *pSourceContext, gpointer *pConfig);

void rendering_free_tree_data (CairoDockDesklet *pDesklet);

void rendering_load_icons_for_tree (CairoDockDesklet *pDesklet, cairo_t *pSourceContext);


void rendering_draw_tree_in_desklet (cairo_t *pCairoContext, CairoDockDesklet *pDesklet, gboolean bRenderOptimized);

void rendering_register_tree_desklet_renderer (void);


#endif

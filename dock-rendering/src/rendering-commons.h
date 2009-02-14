
#ifndef __RENDERING_COMMONS__
#define  __RENDERING_COMMONS__

#include "cairo-dock.h"

#define RENDERING_INTERPOLATION_NB_PTS 1000


typedef enum {
	CD_NORMAL_SEPARATOR = 0,
	CD_FLAT_SEPARATOR,
	CD_PHYSICAL_SEPARATOR,
	CD_NB_SEPARATORS
	} CDSpeparatorType;


cairo_surface_t *cd_rendering_create_flat_separator_surface (cairo_t *pSourceContext, int iWidth, int iHeight);

void cd_rendering_load_flat_separator (CairoContainer *pContainer);


double cd_rendering_interpol (double x, double *fXValues, double *fYValues);


#endif

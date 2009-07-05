
#ifndef __DIALOG_RENDERING_STRUCT__
#define  __DIALOG_RENDERING_STRUCT__

#include <cairo-dock.h>

struct _AppletConfig {
	gint iComicsRadius;
	gint iComicsLineWidth;
	gdouble fComicsLineColor[4];
	
	gint iModernRadius;
	gint iModernLineWidth;
	gdouble fModernLineColor[4];
	gint iModernLineSpacing;
	
	gint iPlaneRadius;
	gint iPlaneLineWidth;
	gdouble fPlaneLineColor[4];
	gdouble fPlaneColor[4];
	
	gint iTooltipRadius;
	gint iTooltipLineWidth;
	gdouble fTooltipLineColor[4];
	gdouble fTooltipMarginColor[4];
	
	gint iCurlyRadius;
	gint iCurlyLineWidth;
	gdouble fCurlyLineColor[4];
	gdouble fCurlyCurvature;
	gboolean bCulrySideToo;
} ;

struct _AppletData {
	gint no_data_yet;
} ;

#endif

/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.
The analogic display comes from Cairo-Clock.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

**********************************************************************************/
#include <stdlib.h>
#define __USE_POSIX
#include <time.h>
#include <signal.h>

#include "applet-struct.h"
#include "applet-theme.h"

static char s_cFileNames[CLOCK_ELEMENTS][30] = {
	"clock-drop-shadow.svg",
	"clock-face.svg",
	"clock-marks.svg",
	"clock-hour-hand-shadow.svg",
	"clock-minute-hand-shadow.svg",
	"clock-second-hand-shadow.svg",
	"clock-hour-hand.svg",
	"clock-minute-hand.svg",
	"clock-second-hand.svg",
	"clock-face-shadow.svg",
	"clock-glass.svg",
	"clock-frame.svg" };


void cd_clock_load_theme (CairoDockModuleInstance *myApplet)
{
	cd_message ("%s (%s)", __func__, myConfig.cThemePath);
	//\_______________ On charge le theme choisi (on n'a pas besoin de connaitre les dimmensions de l'icone).
	if (myConfig.cThemePath != NULL)
	{
		GString *sElementPath = g_string_new ("");
		int i;
		for (i = 0; i < CLOCK_ELEMENTS; i ++)
		{
			g_string_printf (sElementPath, "%s/%s", myConfig.cThemePath, s_cFileNames[i]);
			myData.pSvgHandles[i] = rsvg_handle_new_from_file (sElementPath->str, NULL);
		}
		rsvg_handle_get_dimensions (myData.pSvgHandles[CLOCK_DROP_SHADOW], &myData.DimensionData);
		rsvg_handle_get_dimensions (myData.pSvgHandles[CLOCK_HOUR_HAND], &myData.needleDimension);
		g_print ("clock bg dimension : %dx%d\n", (int) myData.DimensionData.width, (int) myData.DimensionData.height);
		g_print ("clock needle dimension : %dx%d\n", (int) myData.needleDimension.width, (int) myData.needleDimension.height);
		
		// recuperation des parametres des aiguilles.
		g_string_printf (sElementPath, "%s/%s", myConfig.cThemePath, "theme.conf");
		if (g_file_test (sElementPath->str, G_FILE_TEST_EXISTS))
		{
			GError *erreur = NULL;
			GKeyFile *pKeyFile = cairo_dock_open_key_file (sElementPath->str);
			if (pKeyFile != NULL)
			{
				myData.iNeedleRealHeight = g_key_file_get_integer (pKeyFile, "Needle", "height", &erreur);
				if (erreur != NULL)
				{
					cd_warning (erreur->message);
					g_error_free (erreur);
					erreur = NULL;
				}
				myData.iNeedleOffsetX = g_key_file_get_double (pKeyFile, "Needle", "offset x", &erreur);
				if (erreur != NULL)
				{
					cd_warning (erreur->message);
					g_error_free (erreur);
					erreur = NULL;
				}
				g_key_file_free (pKeyFile);
			}
			else  // on prend des valeurs par defaut assez larges.
			{
				myData.iNeedleRealHeight = .2*myData.needleDimension.height;
				myData.iNeedleOffsetX = .3 * myData.needleDimension.width;
			}
			myData.iNeedleRealWidth = myData.needleDimension.width/2 + myData.iNeedleOffsetX;
			myData.iNeedleOffsetY = .5 * myData.iNeedleRealHeight;
			cd_debug ("clock needle : H=%d; dx=%d\n", myData.iNeedleRealHeight, myData.iNeedleOffsetX);
		}
		
		g_string_free (sElementPath, TRUE);
	}
	else
	{
		myData.DimensionData.width = 48;  // valeurs par defaut si aucun theme trouve.
		myData.DimensionData.height = 48;
		myData.needleDimension.width = 48;
		myData.needleDimension.height = 48;
	}
}


static void paint_background (CairoDockModuleInstance *myApplet, cairo_t* pDrawingContext)
{
	if (myData.pSvgHandles[CLOCK_DROP_SHADOW] != NULL)
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_DROP_SHADOW], pDrawingContext);
	if (myData.pSvgHandles[CLOCK_FACE] != NULL)
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_FACE], pDrawingContext);
	if (myData.pSvgHandles[CLOCK_MARKS] != NULL)
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_MARKS], pDrawingContext);
}
static void paint_foreground (CairoDockModuleInstance *myApplet, cairo_t* pDrawingContext)
{
	if (myData.pSvgHandles[CLOCK_FACE_SHADOW] != NULL)
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_FACE_SHADOW], pDrawingContext);
	if (myData.pSvgHandles[CLOCK_GLASS] != NULL)
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_GLASS], pDrawingContext);
	if (myData.pSvgHandles[CLOCK_FRAME] != NULL)
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_FRAME], pDrawingContext);
}
static cairo_surface_t* cd_clock_create_bg_surface (CairoDockModuleInstance *myApplet, cairo_t* pSourceContext, int iWidth, int iHeight, SurfaceKind kind)
{
	//g_print ("%s (%dx%d)\n", __func__, iWidth, iHeight);
	cairo_surface_t* pNewSurface =_cairo_dock_create_blank_surface (pSourceContext, iWidth, iHeight);
	g_return_val_if_fail (cairo_surface_status (pNewSurface) == CAIRO_STATUS_SUCCESS, NULL);
	
	cairo_t* pDrawingContext = cairo_create (pNewSurface);
	g_return_val_if_fail (cairo_status (pDrawingContext) == CAIRO_STATUS_SUCCESS, NULL);
	
	cairo_set_operator (pDrawingContext, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba (pDrawingContext, 1.0f, 1.0f, 1.0f, 0.0f);
	cairo_paint (pDrawingContext);
	
	cairo_set_operator (pDrawingContext, CAIRO_OPERATOR_OVER);
	cairo_scale (pDrawingContext,
		(double) iWidth / (double) myData.DimensionData.width,
		(double) iHeight / (double) myData.DimensionData.height);
	
	switch (kind)
	{
		case KIND_BACKGROUND :
			paint_background (myApplet, pDrawingContext);
		break;
		
		case KIND_FOREGROUND :
			paint_foreground (myApplet, pDrawingContext);
		break;
		
		default :
		return NULL;
	}
	
	cairo_destroy (pDrawingContext);
	
	return pNewSurface;
}

static void paint_hour (CairoDockModuleInstance *myApplet, cairo_t* pDrawingContext)
{
	double fShadowOffsetX = -0.75f;
	double fShadowOffsetY = 0.75f;
	cairo_save (pDrawingContext);
	cairo_translate(pDrawingContext, fShadowOffsetX, fShadowOffsetY);
	if (myData.pSvgHandles[CLOCK_HOUR_HAND_SHADOW] != NULL)
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_HOUR_HAND_SHADOW], pDrawingContext);
	cairo_restore (pDrawingContext);
	if (myData.pSvgHandles[CLOCK_HOUR_HAND] != NULL)
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_HOUR_HAND], pDrawingContext);
}
static void paint_minute (CairoDockModuleInstance *myApplet, cairo_t* pDrawingContext)
{
	double fShadowOffsetX = -0.75f;
	double fShadowOffsetY = 0.75f;
	cairo_save (pDrawingContext);
	cairo_translate(pDrawingContext, fShadowOffsetX, fShadowOffsetY);
	if (myData.pSvgHandles[CLOCK_MINUTE_HAND_SHADOW] != NULL)
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_MINUTE_HAND_SHADOW], pDrawingContext);
	cairo_restore (pDrawingContext);
	if (myData.pSvgHandles[CLOCK_MINUTE_HAND] != NULL)
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_MINUTE_HAND], pDrawingContext);
}
static void paint_second (CairoDockModuleInstance *myApplet, cairo_t* pDrawingContext)
{
	double fShadowOffsetX = -0.75f;
	double fShadowOffsetY = 0.75f;
	cairo_save (pDrawingContext);
	cairo_translate(pDrawingContext, fShadowOffsetX, fShadowOffsetY);
	if (myData.pSvgHandles[CLOCK_SECOND_HAND_SHADOW] != NULL)
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_SECOND_HAND_SHADOW], pDrawingContext);
	cairo_restore (pDrawingContext);
	if (myData.pSvgHandles[CLOCK_SECOND_HAND] != NULL)
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_SECOND_HAND], pDrawingContext);
}
static cairo_surface_t *create_needle_surface (CairoDockModuleInstance *myApplet, cairo_t* pSourceContext, SurfaceKind kind)
{
	cairo_surface_t* pNewSurface =_cairo_dock_create_blank_surface (pSourceContext, myData.iNeedleWidth, myData.iNeedleHeight + 0);  // +1 pour les ombres.
	g_return_val_if_fail (cairo_surface_status (pNewSurface) == CAIRO_STATUS_SUCCESS, NULL);
	
	cairo_t* pDrawingContext = cairo_create (pNewSurface);
	g_return_val_if_fail (cairo_status (pDrawingContext) == CAIRO_STATUS_SUCCESS, NULL);
	
	cairo_set_operator (pDrawingContext, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba (pDrawingContext, 1.0f, 1.0f, 1.0f, 0.0f);
	cairo_paint (pDrawingContext);
	
	cairo_set_operator (pDrawingContext, CAIRO_OPERATOR_OVER);
	
	cairo_scale (pDrawingContext, myData.fNeedleScale, myData.fNeedleScale);
	cairo_translate (pDrawingContext, myData.iNeedleOffsetX, myData.iNeedleOffsetY);
	switch (kind)
	{
		case KIND_HOUR :
			paint_hour (myApplet, pDrawingContext);
		break;
		
		case KIND_MINUTE :
			paint_minute (myApplet, pDrawingContext);
		break;
		
		case KIND_SECOND :
			paint_second (myApplet, pDrawingContext);
		break;
		
		default :
		return NULL;
	}
	
	cairo_destroy (pDrawingContext);
	return pNewSurface;
}

void cd_clock_load_back_and_fore_ground (CairoDockModuleInstance *myApplet)
{
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);

	//\_______________ On construit les surfaces d'arriere-plan et d'avant-plan une bonne fois pour toutes.
	myData.pBackgroundSurface = cd_clock_create_bg_surface (myApplet,
		myDrawContext,
		iWidth,
		iHeight,
		KIND_BACKGROUND);
	myData.pForegroundSurface = cd_clock_create_bg_surface (myApplet,
		myDrawContext,
		iWidth,
		iHeight,
		KIND_FOREGROUND);
}

void cd_clock_load_textures (CairoDockModuleInstance *myApplet)
{
	if (myData.pBackgroundSurface != NULL)
		myData.iBgTexture = cairo_dock_create_texture_from_surface (myData.pBackgroundSurface);
	if (myData.pForegroundSurface != NULL)
		myData.iFgTexture = cairo_dock_create_texture_from_surface (myData.pForegroundSurface);
	
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	
	int iSize = MIN (iWidth, iHeight);
	myData.fNeedleScale = (double) iSize / (double) myData.needleDimension.width;  // car l'aiguille est a l'horizontale dans le fichier svg.
	myData.iNeedleWidth = (double) myData.iNeedleRealWidth * myData.fNeedleScale;
	myData.iNeedleHeight = (double) myData.iNeedleRealHeight * myData.fNeedleScale;
	
	cairo_surface_t *pNeedleSurface = create_needle_surface (myApplet,
		myDrawContext,
		KIND_HOUR);
	if (pNeedleSurface != NULL)
	{
		myData.iHourNeedleTexture = cairo_dock_create_texture_from_surface (pNeedleSurface);
		cairo_surface_destroy (pNeedleSurface);
	}
	
	pNeedleSurface = create_needle_surface (myApplet,
		myDrawContext,
		KIND_MINUTE);
	if (pNeedleSurface != NULL)
	{
		myData.iMinuteNeedleTexture = cairo_dock_create_texture_from_surface (pNeedleSurface);
		cairo_surface_destroy (pNeedleSurface);
	}
	
	pNeedleSurface = create_needle_surface (myApplet,
		myDrawContext,
		KIND_SECOND);
	if (pNeedleSurface != NULL)
	{
		myData.iSecondNeedleTexture = cairo_dock_create_texture_from_surface (pNeedleSurface);
		cairo_surface_destroy (pNeedleSurface);
	}
}



void cd_clock_clear_theme (CairoDockModuleInstance *myApplet, gboolean bClearAll)
{
	if (myData.pBackgroundSurface != NULL)
		cairo_surface_destroy (myData.pBackgroundSurface);
	if (myData.pForegroundSurface != NULL)
		cairo_surface_destroy (myData.pForegroundSurface);
	
	if (myData.iBgTexture != 0)
		glDeleteTextures (1, &myData.iBgTexture);
	if (myData.iFgTexture != 0)
		glDeleteTextures (1, &myData.iFgTexture);
	if (myData.iHourNeedleTexture != 0)
		glDeleteTextures (1, &myData.iHourNeedleTexture);
	if (myData.iMinuteNeedleTexture != 0)
		glDeleteTextures (1, &myData.iMinuteNeedleTexture);
	if (myData.iSecondNeedleTexture != 0)
		glDeleteTextures (1, &myData.iSecondNeedleTexture);
	
	if (bClearAll)
	{
		int i;
		for (i = 0; i < CLOCK_ELEMENTS; i ++)
		{
			if (myData.pSvgHandles[i] != NULL)
			{
				rsvg_handle_free (myData.pSvgHandles[i]);
			}
		}
	}
	else
	{
		myData.pBackgroundSurface = NULL;
		myData.pForegroundSurface = NULL;
		myData.iBgTexture = 0;
		myData.iFgTexture = 0;
		myData.iHourNeedleTexture = 0;
		myData.iMinuteNeedleTexture = 0;
		myData.iSecondNeedleTexture = 0;
	}
}

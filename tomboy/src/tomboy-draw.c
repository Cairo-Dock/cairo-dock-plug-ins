#include "string.h"
#include <glib/gi18n.h>

#include "tomboy-struct.h"
#include "tomboy-draw.h"

CD_APPLET_INCLUDE_MY_VARS


void load_surface(cairo_surface_t *pSurface, gchar *default_image, gchar *user_image)
{
	cd_message("tomboy : Chargement de la surface (%s)",user_image);
	
	GString *sImagePath = g_string_new ("");
	if (pSurface != NULL)
		cairo_surface_destroy (pSurface);
	if (user_image != NULL)
	{
		gchar *cUserImagePath = cairo_dock_generate_file_path (user_image);
		pSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else
	{
		g_string_printf (sImagePath, "%s/%s",MY_APPLET_SHARE_DATA_DIR, default_image);
		pSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	g_string_free (sImagePath, TRUE);
}

void load_all_surfaces(void)
{
	GString *sImagePath = g_string_new ("");
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
		g_string_printf (sImagePath, "%s/default.svg",MY_APPLET_SHARE_DATA_DIR);
		myData.pSurfaceDefault = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	//Chargement de close.svg
	if (myData.pSurfaceClose != NULL)
		cairo_surface_destroy (myData.pSurfaceClose);
	if (myConfig.cIconClose != NULL)
	{
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cIconClose);
		myData.pSurfaceClose = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else
	{
		g_string_printf (sImagePath, "%s/close.svg",MY_APPLET_SHARE_DATA_DIR);
		myData.pSurfaceClose = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	//Chargement de broken.svg
	if (myData.pSurfaceBroken != NULL)
		cairo_surface_destroy (myData.pSurfaceBroken);
	if (myConfig.cIconBroken != NULL)
	{
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cIconBroken);
		myData.pSurfaceBroken = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else
	{
		g_string_printf (sImagePath, "%s/broken.svg",MY_APPLET_SHARE_DATA_DIR);
		myData.pSurfaceBroken = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	g_string_free (sImagePath, TRUE);
}

void update_icon(void)
{
	if(myData.opening)
	{
		cd_message("tomboy : L'application est ouverte");
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", g_hash_table_size (myData.hNoteTable))
		CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceDefault)
	}
	else
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceClose)
	}
}

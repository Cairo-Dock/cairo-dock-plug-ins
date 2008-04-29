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

void update_icon(void)  /// je pense que ca meriterait d'etre traite comme les bookmarks de 'shortcuts', dont l'ajout/suppression sont optimises. Ca ne presse pas ceci dit ^_^
{
	if(myData.opening)
	{
		cd_message("tomboy : L'application est ouverte");
		CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceDefault)
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d",myData.countNotes)
		
		GList *pIconList = NULL;
	
		Icon *pIcon;
		int i;
		TomBoyNote *pNote;
		GList *pElement;
		for (pElement = myData.noteList; pElement != NULL; pElement = pElement->next)
		{	
			pNote = pElement->data;
			pIcon = g_new0 (Icon, 1);
			pIcon->acName = g_strdup (pNote->title);
			cd_message("tomboy : Création de l'icône pour %s [%s]",pNote->name, pNote->title);
			//pIcon->cBaseURI =  g_strdup (pNote->name);  /// je pense que le mettre la est pas tres heureux, il vaudrait mieux associer l'icone via une table de hachage avec 'name' comme cle, puisque celui-ci est unique. ...
			pIcon->fOrder = i;
			pIcon->fScale = 1.;
			pIcon->fAlpha = 1.;
			pIcon->fWidth = 48;  /// inutile je pense ...
			pIcon->fHeight = 48;
			pIcon->fWidthFactor = 1.;
			pIcon->fHeightFactor = 1.;
			pIcon->acCommand = g_strdup (pNote->name);  /// avec g_strdup_printf ("tomboy --open-note %s", pNote->name), ca devient un vrai lanceur.
			pIcon->cParentDockName = g_strdup (myIcon->acName);
			pIcon->acFileName = g_strdup_printf ("%s/note.svg",MY_APPLET_SHARE_DATA_DIR);
			pIconList = g_list_append (pIconList, pIcon);
		}
		
		//On supprime les icones du sous-dock
		if (myIcon->pSubDock != NULL)
		{
			g_list_foreach (myIcon->pSubDock->icons, (GFunc) cairo_dock_free_icon, NULL);
			g_list_free (myIcon->pSubDock->icons);
			myIcon->pSubDock->icons = NULL;
		}
		
		if (myIcon->pSubDock == NULL)
		{
			if (pIconList != NULL)
			{
				myIcon->pSubDock = cairo_dock_create_subdock_from_scratch (pIconList, myIcon->acName);
				cairo_dock_set_renderer (myIcon->pSubDock, 0);
				cairo_dock_update_dock_size (myIcon->pSubDock);
			}
		}
		else
		{
			//On remplace les icônes du sous-dock
			if (pIconList == NULL)  //Il n'y a rien à afficher. On supprime le Sous-Dock.
			{
				cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
				myIcon->pSubDock = NULL;
			}
			else
			{
				myIcon->pSubDock->icons = pIconList;
				cairo_dock_load_buffers_in_one_dock (myIcon->pSubDock);
				cairo_dock_update_dock_size (myIcon->pSubDock);
			}
		}
	}
	else
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceClose)
	}
}

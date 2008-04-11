/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@hollowproject.org)

******************************************************************************/
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-load-icon.h"
#include "applet-compiz.h"
  
CD_APPLET_INCLUDE_MY_VARS

static gchar *s_iconName[3] = {N_("Configure Compiz"), N_("Emerald Manager"), N_("Reload WM")};
static gchar *s_iconFile[3] = {N_("default"), N_("broken"), N_("other")};

static GList * _list_icons (void) {
	GList *pIconList = NULL;
	
	Icon *pIcon;
	int i;
	for (i = 0; i < 3; i ++) {
		pIcon = g_new0 (Icon, 1);
	  pIcon->acName = g_strdup_printf ("%s", D_(s_iconName[i]));
	  if (myConfig.cUserImage[i+3] != NULL) {
	    pIcon->acFileName = cairo_dock_generate_file_path (myConfig.cUserImage[i+3]);
	  }
	  else {
	    pIcon->acFileName = g_strdup_printf ("%s/%d.svg", MY_APPLET_SHARE_DATA_DIR, i);
	  }
	  pIcon->fOrder = 2*i;
	  pIcon->fScale = 1.;
	  pIcon->fAlpha = 1.;
	  pIcon->fWidthFactor = 1.;
	  pIcon->fHeightFactor = 1.;
	  pIcon->acCommand = g_strdup ("none");
	  pIcon->cParentDockName = g_strdup (myIcon->acName);
	  pIconList = g_list_append (pIconList, pIcon);
	}
	
	return pIconList;
}

void _compiz_draw (void) {
	g_return_if_fail (myDrawContext != NULL);
  cd_message ("  chargement de l'icone compiz");
	cd_message("Compiz: Mode de rendering: %s - WM: %d", myConfig.cRenderer, myConfig.iWM);
	
	g_free (myIcon->acFileName);
	if (!myData.bAcquisitionOK) {
	  if (myConfig.cUserImage[COMPIZ_BROKEN] != NULL) {
	    myIcon->acFileName = cairo_dock_generate_file_path (myConfig.cUserImage[COMPIZ_BROKEN]);
	  }
	  else {
		  myIcon->acFileName = g_strdup_printf ("%s/broken.png", MY_APPLET_SHARE_DATA_DIR);
	  }
	}
	else {
	  if (myConfig.cUserImage[myData.iCompizIcon] != NULL) {
	    myIcon->acFileName = cairo_dock_generate_file_path (myConfig.cUserImage[myData.iCompizIcon]);
	  }
	  else {
		  myIcon->acFileName = g_strdup_printf ("%s/%s.svg", MY_APPLET_SHARE_DATA_DIR, s_iconFile[myData.iCompizIcon]);
		}
	}
	CD_APPLET_SET_IMAGE_ON_MY_ICON (myIcon->acFileName)

}


gboolean _cd_compiz_check_for_redraw (void) {
	if (myIcon == NULL) {
		g_print ("annulation du chargement de compiz\n");
		return FALSE;
	}
		
  if (myData.bNeedRedraw) {
  	//\_______________________ On cree la liste des icones de prevision.
  	GList *pIconList = _list_icons ();
  	cd_message ("Compiz: On redessine le sous-dock/desklet");
  	
  	//\_______________________ On efface l'ancienne liste.
  	if (myDesklet && myDesklet->icons != NULL) {
  		g_list_foreach (myDesklet->icons, (GFunc) cairo_dock_free_icon, NULL);
  		g_list_free (myDesklet->icons);
  		myDesklet->icons = NULL;
  	}
  	if (myIcon->pSubDock != NULL) {
  		g_list_foreach (myIcon->pSubDock->icons, (GFunc) cairo_dock_free_icon, NULL);
  		g_list_free (myIcon->pSubDock->icons);
  		myIcon->pSubDock->icons = NULL;
  	}
  	
  	//\_______________________ On charge la nouvelle liste.
  	if (myDock) {
  		if (myIcon->pSubDock == NULL) {
  			if (pIconList != NULL) {
  				cd_message ("  creation du sous-dock compiz");
  				myIcon->pSubDock = cairo_dock_create_subdock_from_scratch (pIconList, myIcon->acName);
  				cairo_dock_set_renderer (myIcon->pSubDock, myConfig.cRenderer);
  				cairo_dock_update_dock_size (myIcon->pSubDock);
  			}
  		}
  		else {
  			cd_message ("  rechargement du sous-dock compiz");
  			if (pIconList == NULL) {
  				cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
  				myIcon->pSubDock = NULL;
  			}
  			else {
  				myIcon->pSubDock->icons = pIconList;
  				cairo_dock_set_renderer (myIcon->pSubDock, myConfig.cRenderer);
  				cairo_dock_load_buffers_in_one_dock (myIcon->pSubDock);
  				cairo_dock_update_dock_size (myIcon->pSubDock);
  			}
  		}
  	}
  	else {
  		if (myIcon->pSubDock != NULL) {
  			cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
  			myIcon->pSubDock = NULL;
  		}
  		myDesklet->icons = pIconList;
  		gpointer pConfig[2] = {GINT_TO_POINTER (FALSE), GINT_TO_POINTER (FALSE)};
  		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Caroussel", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, pConfig);
  		myDrawContext = cairo_create (myIcon->pIconBuffer);
  		gtk_widget_queue_draw (myDesklet->pWidget);
  	}
  		
  	//\_______________________ On recharge l'icone principale.
  	_compiz_draw ();  // ne lance pas le redraw.
  	if (myDesklet)
  		gtk_widget_queue_draw (myDesklet->pWidget);
  	else
  		CD_APPLET_REDRAW_MY_ICON
		
		cd_compiz_check_my_wm(); // On cherche s'il y a eu un changement non désiré		
		myData.bNeedRedraw = FALSE;
	}
	return TRUE;
}

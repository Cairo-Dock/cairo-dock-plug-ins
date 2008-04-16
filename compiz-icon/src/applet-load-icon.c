/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@hollowproject.org)
Fabrice Rey <fabounet@users.berlios.de>

******************************************************************************/
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-load-icon.h"
#include "applet-compiz.h"
 
CD_APPLET_INCLUDE_MY_VARS

#define MY_NB_SUB_ICONS 3
static gchar *s_iconName[MY_NB_SUB_ICONS] = {N_("Configure Compiz"), N_("Emerald Manager"), N_("Reload WM")};
static gchar *s_iconFile[MY_NB_SUB_ICONS] = {N_("default"), N_("broken"), N_("other")};
static gchar *s_iconClass[MY_NB_SUB_ICONS] = {N_("ccsm"), N_("emerald-theme-manager"), NULL};

static GList * _list_icons (void) {
	GList *pIconList = NULL;
	
	Icon *pIcon;
	int i;
	for (i = 0; i < MY_NB_SUB_ICONS; i ++) {
		pIcon = g_new0 (Icon, 1);
	  pIcon->acName = g_strdup (D_(s_iconName[i]));
	  if (myConfig.cUserImage[i+3] != NULL) {
	    pIcon->acFileName = cairo_dock_generate_file_path (myConfig.cUserImage[i+3]);
	  }
	  else {
	    pIcon->acFileName = g_strdup_printf ("%s/%d.svg", MY_APPLET_SHARE_DATA_DIR, i);
	  }
	  pIcon->fOrder = 2*i;
	  pIcon->iType = 2*i;
	  pIcon->fScale = 1.;
	  pIcon->fAlpha = 1.;
	  pIcon->fWidthFactor = 1.;
	  pIcon->fHeightFactor = 1.;
	  pIcon->acCommand = g_strdup ("none");
	  pIcon->cParentDockName = g_strdup (myIcon->acName);
	  pIconList = g_list_append (pIconList, pIcon);
	  if (s_iconClass[i] != NULL) {
	    if (myConfig.bStealTaskBarIcon) {
	      cairo_dock_inhibate_class (s_iconClass[i], pIcon);
	    }
	    else {
	      cairo_dock_deinhibate_class (s_iconClass[i], pIcon);
	    }
	  }
	}
	
	return pIconList;
}

void _compiz_draw (void) {
	g_return_if_fail (myDrawContext != NULL);
  cd_message ("  chargement de l'icone compiz");
	
	g_free (myIcon->acFileName);
	if (!myData.bAcquisitionOK) {
	  if (myConfig.cUserImage[COMPIZ_BROKEN] != NULL) {
	    myIcon->acFileName = cairo_dock_generate_file_path (myConfig.cUserImage[COMPIZ_BROKEN]);
	  }
	  else {
		  myIcon->acFileName = g_strdup_printf ("%s/broken.svg", MY_APPLET_SHARE_DATA_DIR);
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

void cd_compiz_update_main_icon (void) {
	gboolean bNeedsRedraw = FALSE;
	if (myData.bAcquisitionOK) {
		if (myData.bCompizIsRunning && myData.iCompizIcon != COMPIZ_DEFAULT) {
			g_print ("COMPIZ_DEFAULT\n");
			myData.iCompizIcon = COMPIZ_DEFAULT;
			CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cUserImage[COMPIZ_DEFAULT], "default.svg");
			bNeedsRedraw = TRUE;
			g_print ("myIcon->acFileName <- %s\n", myIcon->acFileName);
		}
		else if (! myData.bCompizIsRunning && myData.iCompizIcon != COMPIZ_OTHER) {
			g_print ("COMPIZ_OTHER\n");
			myData.iCompizIcon = COMPIZ_OTHER;
			CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cUserImage[COMPIZ_OTHER], "other.svg");
			bNeedsRedraw = TRUE;
		}
	}
	else {
		if (myData.iCompizIcon != COMPIZ_BROKEN) {
			g_print ("COMPIZ_BROKEN\n");
			myData.iCompizIcon = COMPIZ_BROKEN;
			CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cUserImage[COMPIZ_OTHER], "broken.svg");
			bNeedsRedraw = TRUE;
		}
	}
	if (bNeedsRedraw)
		CD_APPLET_REDRAW_MY_ICON
}


void cd_compiz_build_icons (void) {
	GList *pIconList = _list_icons ();
	if (myDock) {
		myIcon->pSubDock = cairo_dock_create_subdock_from_scratch (pIconList, myIcon->acName);
		cairo_dock_set_renderer (myIcon->pSubDock, myConfig.cRenderer);
		cairo_dock_update_dock_size (myIcon->pSubDock);
	}
	else {
		myDesklet->icons = pIconList;
  		gpointer pConfig[2] = {GINT_TO_POINTER (FALSE), GINT_TO_POINTER (FALSE)};
  		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Caroussel", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, pConfig);
  		myDrawContext = cairo_create (myIcon->pIconBuffer);
  		gtk_widget_queue_draw (myDesklet->pWidget);
	}
}

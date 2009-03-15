/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)
Fabrice Rey <fabounet@users.berlios.de>

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"



CD_APPLET_GET_CONFIG_BEGIN
	
	//myConfig.iWM = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "WM", COMPIZ_FUSION);
	myConfig.lBinding = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "binding", FALSE);
	myConfig.iRendering = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "irendering", FALSE);
	myConfig.uLocalScreen = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "ulocalscreen", FALSE);
	myConfig.forceConfig = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "force", FALSE);
	//myConfig.protectDecorator = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "protect", TRUE);
	myConfig.cUserWMCommand = CD_CONFIG_GET_STRING ("Configuration", "ccmd");
	myConfig.cRenderer = CD_CONFIG_GET_STRING ("Configuration", "renderer");
	
	myConfig.bAutoReloadCompiz = CD_CONFIG_GET_BOOLEAN ("Configuration", "auto reload compiz");
	myConfig.bAutoReloadDecorator = CD_CONFIG_GET_BOOLEAN ("Configuration", "auto reload decorator");
	
	myConfig.cWindowDecorator = CD_CONFIG_GET_STRING ("Configuration", "system decorator");
	if (myConfig.cWindowDecorator == NULL)
		myConfig.cWindowDecorator = g_strdup ("emerald");
	myConfig.cDecorators[DECORATOR_EMERALD] = "emerald";
	myConfig.cDecorators[DECORATOR_GTK] = "gtk-window-decorator";
	myConfig.cDecorators[DECORATOR_KDE] = "kde-window-decorator";
	myConfig.cDecorators[DECORATOR_HELIODOR] = "heliodor";
	compizDecorator i;
	for (i = 0; i < DECORATOR_USER; i ++) { // on cherche si le decorateur choisi est dans la liste.
		if (strcmp (myConfig.cDecorators[i], myConfig.cWindowDecorator) == 0)
			break ;
	}
	if (i == DECORATOR_USER)  // on ne l'a pas trouve, on le rajoute donc a la fin.
		myConfig.cDecorators[DECORATOR_USER] = myConfig.cWindowDecorator;
	else
		myConfig.cDecorators[DECORATOR_USER] = NULL;
	
	
	myConfig.cUserImage[COMPIZ_DEFAULT] 		= CD_CONFIG_GET_STRING ("Configuration", "default icon");
	myConfig.cUserImage[COMPIZ_BROKEN] 		  = CD_CONFIG_GET_STRING ("Configuration", "broken icon");
	myConfig.cUserImage[COMPIZ_OTHER] 		  = CD_CONFIG_GET_STRING ("Configuration", "other icon");
	myConfig.cUserImage[COMPIZ_SETTING] 		= CD_CONFIG_GET_STRING ("Configuration", "setting icon");
	myConfig.cUserImage[COMPIZ_EMERALD] 		= CD_CONFIG_GET_STRING ("Configuration", "emerald icon");
	myConfig.cUserImage[COMPIZ_RELOAD] 		  = CD_CONFIG_GET_STRING ("Configuration", "reload icon");
	myConfig.cUserImage[COMPIZ_EXPOSITION] 	= CD_CONFIG_GET_STRING ("Configuration", "expo icon");
	myConfig.cUserImage[COMPIZ_WLAYER] 		  = CD_CONFIG_GET_STRING ("Configuration", "wlayer icon");
	
	myConfig.iActionOnMiddleClick = CD_CONFIG_GET_INTEGER ("Configuration", "middle click");
	myConfig.bStealTaskBarIcon = CD_CONFIG_GET_BOOLEAN ("Configuration", "inhibate appli");
	myConfig.bScriptSubDock = CD_CONFIG_GET_BOOLEAN ("Configuration", "script");
	myConfig.bEmeraldIcon = CD_CONFIG_GET_BOOLEAN ("Configuration", "emerald");
CD_APPLET_GET_CONFIG_END

CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cRenderer);
	g_free (myConfig.cUserWMCommand);
	g_free (myConfig.cWindowDecorator);
	int i;
	for (i = 0; i < COMPIZ_NB_ITEMS; i ++)
		g_free (myConfig.cUserImage[i]);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cairo_dock_free_measure_timer (myData.pMeasureTimer);
	
	if (myIcon->pSubDock != NULL) {
		CD_APPLET_DESTROY_MY_SUBDOCK;
	}
CD_APPLET_RESET_DATA_END

/************************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

************************************************************************************/
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-read-data.h"
#include "applet-load-icons.h"

CD_APPLET_INCLUDE_MY_VARS


static int s_iThreadIsRunning = 0;
static int s_iSidTimerRedraw = 0;


#define _add_icon(i, j)\
	if (myData.days[i].cName != NULL)\
	{\
		pIcon = g_new0 (Icon, 1);\
		pIcon->acName = g_strdup_printf ("%s", myData.days[i].cName);\
		pIcon->acFileName = g_strdup_printf ("%s/%s.png", myConfig.cThemePath, myData.days[i].part[j].cIconNumber);\
		if (! g_file_test (pIcon->acFileName, G_FILE_TEST_EXISTS))\
		{\
			g_free (pIcon->acFileName);\
			pIcon->acFileName = g_strdup_printf ("%s/%s.svg", myConfig.cThemePath, myData.days[i].part[j].cIconNumber);\
		}\
		if (myConfig.bDisplayTemperature)\
			pIcon->cQuickInfo = g_strdup_printf ("%s/%s", _display (myData.days[i].cTempMin), _display (myData.days[i].cTempMax));\
		pIcon->fOrder = 2*i+j;\
		pIcon->fScale = 1.;\
		pIcon->fAlpha = 1.;\
		pIcon->fWidthFactor = 1.;\
		pIcon->fHeightFactor = 1.;\
		pIcon->acCommand = g_strdup ("none");\
		pIcon->cParentDockName = g_strdup (myIcon->acName);\
		cd_debug (" + %s (%s , %s)", pIcon->acName, myData.days[i].part[j].cWeatherDescription, pIcon->acFileName);\
		pIconList = g_list_append (pIconList, pIcon);\
	}

static GList * _list_icons (void)
{
	GList *pIconList = NULL;
	
	Icon *pIcon;
	int i;
	for (i = 0; i < myConfig.iNbDays; i ++)
	{
		_add_icon (i, 0);
		
		if (myConfig.bDisplayNights)
		{
			_add_icon (i, 1);
		}
	}
	
	return pIconList;
}


static void _weather_draw_current_conditions (void)
{
	g_return_if_fail (myDrawContext != NULL);
	if (myConfig.bCurrentConditions)
	{
		cd_message ("  chargement de l'icone meteo");
		if (myConfig.bDisplayTemperature && myData.currentConditions.cTemp != NULL)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%s%s", myData.currentConditions.cTemp, myData.units.cTemp)
		}
		else
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF (NULL)
		}
		
		g_free (myIcon->acFileName);
		if (myData.bErrorRetrievingData)
			myIcon->acFileName = g_strdup_printf ("%s/broken.png", MY_APPLET_SHARE_DATA_DIR);
		else
		{
			myIcon->acFileName = g_strdup_printf ("%s/%s.png", myConfig.cThemePath, myData.currentConditions.cIconNumber);
			if (! g_file_test (myIcon->acFileName, G_FILE_TEST_EXISTS))
			{
				g_free (myIcon->acFileName);
				myIcon->acFileName = g_strdup_printf ("%s/%s.svg", myConfig.cThemePath, myData.currentConditions.cIconNumber);
			}
		}
		CD_APPLET_SET_IMAGE_ON_MY_ICON (myIcon->acFileName)
	}
}

void cd_weather_update_from_data (void)
{
	//\_______________________ On cree la liste des icones de prevision.
	GList *pIconList = _list_icons ();  // ne nous appartiendra plus, donc ne pas desallouer.
	
	//\_______________________ On efface l'ancienne liste.
	if (myDesklet && myDesklet->icons != NULL)
	{
		g_list_foreach (myDesklet->icons, (GFunc) cairo_dock_free_icon, NULL);
		g_list_free (myDesklet->icons);
		myDesklet->icons = NULL;
	}
	if (myIcon->pSubDock != NULL)
	{
		g_list_foreach (myIcon->pSubDock->icons, (GFunc) cairo_dock_free_icon, NULL);
		g_list_free (myIcon->pSubDock->icons);
		myIcon->pSubDock->icons = NULL;
	}
	
	//\_______________________ On charge la nouvelle liste.
	if (myDock)  // en mode 'dock', on affiche la meteo dans un sous-dock.
	{
		if (myIcon->pSubDock == NULL)
		{
			if (pIconList != NULL)  // l'applet peut montrer les conditions courantes.
			{
				cd_message ("  creation du sous-dock meteo");
				CD_APPLET_CREATE_MY_SUBDOCK (pIconList, myConfig.cRenderer)
			}
		}
		else  // on a deja notre sous-dock, on remplace juste ses icones.
		{
			cd_message ("  rechargement du sous-dock meteo");
			if (pIconList == NULL)  // inutile de le garder.
			{
				CD_APPLET_DESTROY_MY_SUBDOCK
			}
			else
			{
				CD_APPLET_LOAD_ICONS_IN_MY_SUBDOCK (pIconList)
			}
		}
	}
	else
	{
		if (myIcon->pSubDock != NULL)
		{
			cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
			myIcon->pSubDock = NULL;
		}
		myDesklet->icons = pIconList;
		gpointer pConfig[2] = {GINT_TO_POINTER (myConfig.bDesklet3D), GINT_TO_POINTER (FALSE)};
		CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Caroussel", pConfig);
		gtk_widget_queue_draw (myDesklet->pWidget);  /// utile ?...
	}
	
	//\_______________________ On recharge l'icone principale.
	_weather_draw_current_conditions ();  // ne lance pas le redraw.
	if (myDesklet)
		gtk_widget_queue_draw (myDesklet->pWidget);
	else
		CD_APPLET_REDRAW_MY_ICON
	
}

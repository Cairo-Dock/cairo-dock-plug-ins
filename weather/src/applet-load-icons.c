/************************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

************************************************************************************/
#include <string.h>

#include "applet-struct.h"
#include "applet-read-data.h"
#include "applet-load-icons.h"

char *cMonthsWeeks[19] = { N_("Monday") , N_("Tuesday") , N_("Wednesday") , N_("Thursday") , N_("Friday") , N_("Saturday") , N_("Sunday") , N_("Jan") , N_("Feb") , N_("Mar") , N_("Apr") , N_("May") ,N_("Jun") , N_("Jui") , N_("Aug") , N_("Sep") , N_("Oct") , N_("Nov") , N_("Dec") };

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
		cd_debug (" + %s (%s , %s)", pIcon->acName, myData.days[i].part[j].cWeatherDescription, pIcon->acFileName);\
		pIconList = g_list_append (pIconList, pIcon);\
	}

static GList * _list_icons (CairoDockModuleInstance *myApplet)
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


static void _weather_draw_current_conditions (CairoDockModuleInstance *myApplet)
{
	g_return_if_fail (myDrawContext != NULL);
	if (myConfig.bCurrentConditions)
	{
		cd_message ("  chargement de l'icone meteo (%x)", myApplet);
		if (myConfig.bDisplayTemperature && myData.currentConditions.cTemp != NULL/** && myConfig.iDeskletRenderer != MY_DESKLET_MAIN_ICON*/)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%s%s", myData.currentConditions.cTemp, myData.units.cTemp);
		}
		else
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		}
		
		g_free (myIcon->acFileName);
		if (myData.bErrorRetrievingData)
		{
			myIcon->acFileName = g_strdup_printf ("%s/na.png", myConfig.cThemePath);
			if (! g_file_test (myIcon->acFileName, G_FILE_TEST_EXISTS))
			{
				g_free (myIcon->acFileName);
				myIcon->acFileName = g_strdup_printf ("%s/na.svg", myConfig.cThemePath);
				if (! g_file_test (myIcon->acFileName, G_FILE_TEST_EXISTS))
				{
					g_free (myIcon->acFileName);
					myIcon->acFileName = g_strdup (MY_APPLET_SHARE_DATA_DIR"/broken.png");
				}
			}
		}
		else
		{
			myIcon->acFileName = g_strdup_printf ("%s/%s.png", myConfig.cThemePath, myData.currentConditions.cIconNumber);
			if (! g_file_test (myIcon->acFileName, G_FILE_TEST_EXISTS))
			{
				g_free (myIcon->acFileName);
				myIcon->acFileName = g_strdup_printf ("%s/%s.svg", myConfig.cThemePath, myData.currentConditions.cIconNumber);
			}
		}
		CD_APPLET_SET_IMAGE_ON_MY_ICON (myIcon->acFileName);
	}
	else
	{
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;
	}
	
	/**if (myConfig.iDeskletRenderer == MY_DESKLET_MAIN_ICON)
	{ 
	  char *cTemp = NULL;
	  if (myConfig.bDisplayTemperature && myData.currentConditions.cTemp != NULL)
	    cTemp = g_strdup_printf ("%s%s", myData.currentConditions.cTemp, myData.units.cTemp);
	  char *cConditions = NULL;
	  if (myData.currentConditions.cWeatherDescription != NULL)
	     cConditions = g_strdup (myData.currentConditions.cWeatherDescription);
	     
    gpointer data[2] = {cConditions, cTemp};
    cairo_dock_render_desklet_with_new_data (myDesklet, (CairoDeskletRendererDataPtr) data);
    
    if (cConditions != NULL)
      g_free (cConditions);
    if (cTemp != NULL)
      g_free (cTemp);
  }*/
}

gboolean cd_weather_update_from_data (CairoDockModuleInstance *myApplet)
{
	g_return_val_if_fail (myIcon != NULL, FALSE);  // paranoia
	
	//\_______________________ On etablit le nom de l'icone.
	if (myIcon->acName == NULL && myDock)
	{
		CD_APPLET_SET_NAME_FOR_MY_ICON (myData.cLocation != NULL ? myData.cLocation : WEATHER_DEFAULT_NAME);
	}
	
	//\_______________________ On cree la liste des icones de prevision.
	GList *pIconList = _list_icons (myApplet);  // ne nous appartiendra plus, donc ne pas desallouer.
	
	//\_______________________ On efface l'ancienne liste.
	CD_APPLET_DELETE_MY_ICONS_LIST;
	
	//\_______________________ On charge la nouvelle liste.
	///if (myConfig.iDeskletRenderer == MY_DESKLET_CAROUSSEL)
	{
	  gpointer pConfig[2] = {GINT_TO_POINTER (myConfig.bDesklet3D), GINT_TO_POINTER (FALSE)};
	  CD_APPLET_LOAD_MY_ICONS_LIST (pIconList, myConfig.cRenderer, "Caroussel", pConfig);
	}
	
	//\_______________________ On recharge l'icone principale.
	_weather_draw_current_conditions (myApplet);  // ne lance pas le redraw.
	CD_APPLET_REDRAW_MY_ICON;
	
	return TRUE;
}

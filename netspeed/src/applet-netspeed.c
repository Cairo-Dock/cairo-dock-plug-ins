#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib/gi18n.h>
#include <glib.h>
#include <glib/gprintf.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-netspeed.h"
#include "cairo-dock.h"

CD_APPLET_INCLUDE_MY_VARS

#define NETSPEED_DATA_PIPE "/proc/net/dev"


// Prend un debit en octet par seconde et le transforme en une chaine de la forme : xxx yB/s
void cd_netspeed_formatRate(unsigned long long rate, gchar* debit) {
	int smallRate;
	if(rate > 1000000000000000)
	{
		if (myDesklet)
			g_sprintf(debit, "999+ TB/s");
		else
			g_sprintf(debit, "###");
 	}
 	else if (rate > 1000000000000)
 	{
 		smallRate = (int) (rate / 1000000000000);
 		if (myDesklet)
			g_sprintf(debit, "%i %s/s", smallRate, D_("TB"));
		else
			g_sprintf(debit, "%iT", smallRate);
 	}
 	else if (rate > 1000000000)
 	{
 		smallRate = (int) (rate / 1000000000);
 		if (myDesklet)
			g_sprintf(debit, "%i %s/s", smallRate, D_("GB"));
		else
			g_sprintf(debit, "%iG", smallRate);
 	}
 	else if (rate > 1000000)
 	{
 		smallRate = (int) (rate / 1000000);
  		if (myDesklet)
			g_sprintf(debit, "%i %s/s", smallRate, D_("MB"));
		else
			g_sprintf(debit, "%iM", smallRate);
 	}
 	else if (rate > 1000)
 	{
 		smallRate = (int) (rate / 1000);
  		if (myDesklet)
			g_sprintf(debit, "%i %s/s", smallRate, D_("KB"));
		else
			g_sprintf(debit, "%iK", smallRate);
 	}
 	else if (rate > 0)
 	{
 		smallRate = rate;
		if (myDesklet)
			g_sprintf(debit, "%i %s/s", smallRate, D_("B"));
		else
			g_sprintf(debit, "%io", smallRate);
 	}
	else
	{
		if (myDesklet)
			g_sprintf(debit, "0 %s/s", D_("B"));
		else
			g_sprintf(debit, "0");
	}
}


/*void cd_netspeed_acquisition (void)
{
	gchar *cCommand = g_strdup_printf ("cat /proc/net/dev > %s", NETSPEED_TMP_FILE);
	system (cCommand);
	g_free (cCommand);
}*/

void cd_netspeed_read_data (void)
{
	g_timer_stop (myData.pClock);
	double fTimeElapsed = g_timer_elapsed (myData.pClock, NULL);
	g_return_if_fail (fTimeElapsed != 0);
	g_timer_start (myData.pClock);
	
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents (NETSPEED_DATA_PIPE, &cContent, &length, &erreur);
	if (erreur != NULL)
	{
		cd_warning("Attention : %s", erreur->message);
		g_error_free(erreur);
		erreur = NULL;
		myData.bAcquisitionOK = FALSE;
	}
	else
	{
		int iNumLine = 1;
		gchar *tmp = cContent;
		int iReceivedBytes, iTransmittedBytes;
		while (TRUE)
		{
			if (iNumLine > 3)  // les 2 premieres lignes sont les noms des champs, la 3eme est la loopback.
			{
				while (*tmp == ' ')  // on saute les espaces.
					tmp ++;
				
				if (strncmp (tmp, myConfig.cInterface, myConfig.iStringLen) == 0 && *(tmp+myConfig.iStringLen) == ':')  // c'est l'interface qu'on veut.
				{
					tmp += myConfig.iStringLen+1;  // on saute le ':' avec.
					
					iReceivedBytes = atoi (tmp);
					
					int i = 0;
					for (i = 0; i < 8; i ++)  // on saute les 8 valeurs suivantes.
					{
						while (*tmp != ' ')  // saute le chiffre courant.
							tmp ++;
						while (*tmp == ' ')  // saute les espaces.
							tmp ++;
					}
					iTransmittedBytes = atoi (tmp);
					
					if (myData.bInitialized)  // la 1ere iteration on ne peut pas calculer le debit.
					{
						myData.iDownloadSpeed = (iReceivedBytes - myData.iReceivedBytes) / fTimeElapsed;
						myData.iUploadSpeed = (iTransmittedBytes - myData.iTransmittedBytes) / fTimeElapsed;
					}
					
					myData.iReceivedBytes = iReceivedBytes;
					myData.iTransmittedBytes = iTransmittedBytes;
					break ;
				}
			}
			tmp = strchr (tmp+1, '\n');
			if (tmp == NULL)
				break;
			tmp ++;
			iNumLine ++;
		}
		myData.bAcquisitionOK = (tmp != NULL);
		g_free (cContent);
	}
}

void cd_netspeed_update_from_data (void)
{
	if ( ! myData.bAcquisitionOK)
	{
		if (myConfig.iInfoDisplay == NETSPEED_INFO_ON_LABEL)
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle)
		else if (myConfig.iInfoDisplay == NETSPEED_INFO_ON_ICON)
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON("N/A");
		make_cd_Gauge(myDrawContext,myDock,myIcon,myData.pGauge,(double) 0);
		
		cairo_dock_downgrade_frequency_state (myData.pMeasureTimer);
	}
	else
	{
		cairo_dock_set_normal_frequency_state (myData.pMeasureTimer);
		
		if (! myData.bInitialized)
		{
			if (myConfig.iInfoDisplay == NETSPEED_INFO_ON_ICON)
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON(myDock ? "..." : D_("Loading"));
			make_cd_Gauge(myDrawContext,myDock,myIcon,myData.pGauge,(double) 0);
			myData.bInitialized = TRUE;
		}
		else
		{
			if (myConfig.iInfoDisplay != NETSPEED_NO_INFO)
			{
				gchar upRateFormatted[11];
				gchar downRateFormatted[11];
				cd_netspeed_formatRate(myData.iUploadSpeed, upRateFormatted);
				cd_netspeed_formatRate(myData.iDownloadSpeed, downRateFormatted);
				if (myConfig.iInfoDisplay == NETSPEED_INFO_ON_ICON)
				{
					CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("↑%s\n↓%s", upRateFormatted, downRateFormatted)
				}
				else
				{
					gchar * cInfoTitle = g_strdup_printf ("↑%s\n↓%s", upRateFormatted, downRateFormatted);
					CD_APPLET_SET_NAME_FOR_MY_ICON (cInfoTitle)
					g_free (cInfoTitle);
				}
			}
			
			if(myData.iUploadSpeed > myData.iMaxUpRate) {
				myData.iMaxUpRate = myData.iUploadSpeed;
			}
			if(myData.iDownloadSpeed > myData.iMaxDownRate) {
				myData.iMaxDownRate = myData.iDownloadSpeed;
			}
			
			if((myData.iMaxUpRate != 0) && (myData.iMaxDownRate != 0))
			{
				GList *pList = NULL;  /// il faudrait passer un tableau plutot ...
				double *pValue = g_new (double, 1);
				*pValue = (double) myData.iUploadSpeed / myData.iMaxUpRate;
				pList = g_list_append (pList, pValue);
				pValue = g_new (double, 1);
				*pValue = (double) myData.iDownloadSpeed / myData.iMaxDownRate;
				pList = g_list_append (pList, pValue);
				make_cd_Gauge_multiValue(myDrawContext,myDock,myIcon,myData.pGauge,pList);
				g_list_foreach (pList, g_free, NULL);
				g_list_free (pList);
			}
			else
			{
				if(myData.iMaxUpRate != 0)
				{
					make_cd_Gauge(myDrawContext,myDock,myIcon,myData.pGauge,(double) myData.iUploadSpeed / myData.iMaxUpRate);
				}
				else if(myData.iMaxDownRate != 0)
				{
					make_cd_Gauge(myDrawContext,myDock,myIcon,myData.pGauge,(double) myData.iDownloadSpeed / myData.iMaxDownRate);
				}
				else
					make_cd_Gauge(myDrawContext,myDock,myIcon,myData.pGauge,(double) 0);
			}
		}
	}
	CD_APPLET_REDRAW_MY_ICON
}

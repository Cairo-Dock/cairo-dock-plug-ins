/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <fcntl.h>
#include <unistd.h>
#include <unistd.h>

#include "applet-struct.h"
#include "applet-rame.h"

#define RAME_DATA_PIPE CD_SYSMONITOR_PROC_FS"/meminfo"

#define goto_next_line \
	str = strchr (str, '\n'); \
	if (str == NULL) { \
		myData.bAcquisitionOK = FALSE; \
		return; \
	} \
	str ++;
#define get_value(iValue) \
	str = strchr (str, ':'); \
	if (str == NULL) { \
		myData.bAcquisitionOK = FALSE; \
		g_free (cContent); \
		return; \
	} \
	str ++; \
	while (*str == ' ') \
		str ++; \
	iValue = atoll (str);
void cd_sysmonitor_get_ram_data (GldiModuleInstance *myApplet)
{
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents (RAME_DATA_PIPE, &cContent, &length, &erreur);
	if (erreur != NULL)
	{
		cd_warning("ram : %s", erreur->message);
		g_error_free(erreur);
		erreur = NULL;
		myData.bAcquisitionOK = FALSE;
	}
	else
	{
		gchar *str = cContent;
		
		get_value (myData.ramTotal)  // MemTotal
		cd_debug ("ramTotal : %lld", myData.ramTotal);
		
		goto_next_line
		get_value (myData.ramFree)  // MemFree
		cd_debug ("ramFree : %lld", myData.ramFree);
		
		myData.ramUsed = myData.ramTotal - myData.ramFree;
		goto_next_line
		get_value (myData.ramBuffers)  // Buffers.
		
		goto_next_line
		get_value (myData.ramCached)  // Cached.
		cd_debug ("ramCached : %lld", myData.ramCached);
		
		myData.fRamPercent = 100. * (myConfig.bShowFreeMemory ? myData.ramFree + myData.ramCached + myData.ramBuffers : myData.ramUsed - myData.ramCached - myData.ramBuffers) / myData.ramTotal;
		if (fabs (myData.fRamPercent - myData.fPrevRamPercent) > 1)
		{
			myData.fPrevRamPercent = myData.fRamPercent;
			myData.bNeedsUpdate = TRUE;
		}
		
		if (myConfig.bShowSwap)
		{
			goto_next_line  // SwapCached:
			goto_next_line  // Active:
			goto_next_line  // Inactive:
			
			while (strncmp (str, "SwapTotal", 9) != 0)  // apres, suivant la version su noyau, les lignes ne sont pas les memes, on fait donc une recherche.
			{
				goto_next_line
			}
			get_value (myData.swapTotal)  // SwapTotal.
			cd_debug ("swapTotal : %lld", myData.swapTotal);
			goto_next_line
			get_value (myData.swapFree)  // SwapFree.
			cd_debug ("swapFree : %lld", myData.swapFree);
			
			myData.swapUsed = myData.swapTotal - myData.swapFree;
			
			myData.fSwapPercent = 100. * myData.swapUsed / myData.swapTotal;  // que faire de SwapCached ?...
			if (fabs (myData.fSwapPercent - myData.fPrevSwapPercent) > 1)
			{
				myData.fPrevSwapPercent = myData.fSwapPercent;
				myData.bNeedsUpdate = TRUE;
			}
		}
		
		g_free (cContent);
	}
}


#define _convert_from_kb(s) (((s >> 20) == 0) ? (s / 1024.) : (s / (1048576.)))
#define _unit(s) (((s >> 20) == 0) ? D_("Mb") : D_("Gb"))
#define _append_value_from_kb(pInfo, s) t = _convert_from_kb (s); g_string_append_printf (pInfo, t<10 ? "%.1f" : "%.0f", t); g_string_append (pInfo, _unit(s));

void cd_sysmonitor_get_ram_info (GldiModuleInstance *myApplet, GString *pInfo)
{
	if (!myConfig.bShowRam && ! myConfig.bShowSwap)
		cd_sysmonitor_get_ram_data (myApplet);  // le thread ne passe pas par la => pas de conflit.
	if (myData.ramTotal == 0)
		return;
	
	unsigned long long ram = myData.ramFree + myData.ramCached + myData.ramBuffers;
	double t;
	g_string_append_printf (pInfo, "\n%s : ", D_("Memory"));
	_append_value_from_kb (pInfo, myData.ramTotal);
	
	g_string_append_printf (pInfo, " - %s : ", D_("Available"));
	_append_value_from_kb (pInfo, ram);
	
	g_string_append_printf (pInfo, "\n  %s : ", D_("Cached"));
	_append_value_from_kb (pInfo, myData.ramCached);
	
	g_string_append_printf (pInfo, " - %s : ", D_("Buffers"));
	_append_value_from_kb (pInfo, myData.ramBuffers);
}

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

#define get_value(cNeedle, iValue) \
	str = strstr (str, cNeedle); \
	if (str == NULL) { \
		myData.bAcquisitionOK = FALSE; \
		g_free (cContent); \
		return; \
	} \
	str += strlen (cNeedle); \
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
		
		get_value ("MemTotal:", myData.ramTotal)  // MemTotal
		
		get_value ("MemFree:", myData.ramFree)  // MemFree
		
		myData.ramUsed = myData.ramTotal - myData.ramFree;
		get_value ("Buffers:", myData.ramBuffers)  // Buffers.
		
		get_value ("Cached:", myData.ramCached)  // Cached.
		
		myData.fRamPercent = 100. * (myData.ramUsed - myData.ramCached - myData.ramBuffers) / myData.ramTotal;
		
		if (fabs (myData.fRamPercent - myData.fPrevRamPercent) > 1)
		{
			myData.fPrevRamPercent = myData.fRamPercent;
			myData.bNeedsUpdate = TRUE;
		}
		
		if (myConfig.bShowSwap)
		{
			get_value ("SwapTotal:", myData.swapTotal)  // SwapTotal.
			
			get_value ("SwapFree:", myData.swapFree)  // SwapFree.
			
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

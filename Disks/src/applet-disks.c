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
#include <glib/gi18n.h>
#include <glib.h>
#include <glib/gprintf.h>

#include <sys/statvfs.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-disks.h"
#include "cairo-dock.h"

#define DISK_BLOCK_SIZE 512
#define DISKS_SPEED_DATA_PIPE "/proc/diskstats"


// Prend un debit en octet par seconde et le transforme en une chaine de la forme : xxx yB/s
static void _cd_speed_formatRate (unsigned long long rate, gchar* debit, int iBufferSize, gboolean bLong)
{
	int smallRate;
	if (rate <= 0)
	{
		if (bLong)
			snprintf (debit, iBufferSize, "0 %s/s", D_("B"));
		else
			snprintf (debit, iBufferSize, "0");
	}
	else if (rate < 1024)
	{
		smallRate = rate;
		if (bLong)
			snprintf (debit, iBufferSize, "%i %s/s", smallRate, D_("B"));
		else
			snprintf (debit, iBufferSize, "%iB", smallRate);
	}
	else if (rate < (1<<20))
	{
		smallRate = rate >> 10;
		if (bLong)
			snprintf (debit, iBufferSize, "%i %s/s", smallRate, D_("KB"));
		else
			snprintf (debit, iBufferSize, "%iK", smallRate);
	}
	else if (rate < (1<<30))
	{
		smallRate = rate >> 20;
		if (bLong)
			snprintf (debit, iBufferSize, "%i %s/s", smallRate, D_("MB"));
		else
			snprintf (debit, iBufferSize, "%iM", smallRate);
	}
	else if (rate < ((unsigned long long)1<<40))
	{
		smallRate = rate >> 30;
		if (bLong)
			snprintf (debit, iBufferSize, "%i %s/s", smallRate, D_("GB"));
		else
			snprintf (debit, iBufferSize, "%iG", smallRate);
	}
	else  // c'est vraiment pour dire qu'on est exhaustif :-)
	{
		smallRate = rate >> 40;
		if (bLong)
			snprintf (debit, iBufferSize, "%i %s/s", smallRate, D_("TB"));
		else
			snprintf (debit, iBufferSize, "%iT", smallRate);
	}
}

void cd_disks_format_value_on_icon (CairoDataRenderer *pRenderer, int iNumValue, gchar *cFormatBuffer, int iBufferLength, CairoDockModuleInstance *myApplet)
{
	if (iNumValue < (int) myConfig.iNumberParts)
		{
			double *pSize;
			pSize = g_list_nth_data (myData.lParts, iNumValue);
			snprintf (cFormatBuffer, iBufferLength,
				"%.f%%",
				*pSize * 100);
		}
	else
	{
		static gchar s_upRateFormatted[11];
		double fValue = cairo_data_renderer_get_normalized_current_value_with_latency (pRenderer, iNumValue);
		int i = iNumValue / 2;
		CDDiskSpeedData *pSpeed = g_list_nth_data (myData.lDisks, i);
		fValue *= (iNumValue == i * 2 ? pSpeed->uMaxReadRate : pSpeed->uMaxWriteRate);
	
		_cd_speed_formatRate (fValue, s_upRateFormatted, 11, FALSE);
		snprintf (cFormatBuffer, iBufferLength,
			"%s%s",
			cairo_data_renderer_can_write_values (pRenderer) ? (iNumValue == i * 2 ?"↑" : "↓") : "",
			s_upRateFormatted);
	}
}


gboolean cd_disks_update_from_data (CairoDockModuleInstance *myApplet)
{
	static gchar s_readRateFormatted[11], s_writeRateFormatted[11];
	static double s_fValues[CD_DISKS_NB_MAX_VALUES];
		//~ memset (s_fValues, 0, sizeof (s_fValues));
	GString *sLabel = g_string_new ("");
	gsize i;

	CD_APPLET_ENTER;
	if (myConfig.iNumberParts > 0)
	{
		double *pSize;
		for (i = 0; i < myConfig.iNumberParts; i++)
		{
			pSize = g_list_nth_data (myData.lParts, i);
			s_fValues[i] = *pSize;
			//cd_warning("Partition %d = %s : %.1f%%", i, myConfig.cParts[i], *pSize * 100);

			if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL)
			{
				if (i > 0) 
					g_string_append (sLabel, " - ");

				g_string_append_printf (sLabel, "%s : %.1f%%", myConfig.cParts[i], *pSize * 100);

			}
		}
	}
	
	if (myData.iNumberDisks > 0)
	{
		CDDiskSpeedData *pSpeed;
		for (i = 0; i < myData.iNumberDisks; i++)
		{
			pSpeed = g_list_nth_data (myData.lDisks, i);
	
			if(pSpeed->uReadSpeed > pSpeed->uMaxReadRate)
				pSpeed->uMaxReadRate = pSpeed->uReadSpeed;
			
			if(pSpeed->uWriteSpeed > pSpeed->uMaxWriteRate)
				pSpeed->uMaxWriteRate = pSpeed->uWriteSpeed;
				
			s_fValues[i * 2] = (pSpeed->uMaxReadRate != 0 ? (double) pSpeed->uReadSpeed / pSpeed->uMaxReadRate : 0. );
			s_fValues[i * 2 + 1] = (pSpeed->uMaxWriteRate != 0 ? (double) pSpeed->uWriteSpeed / pSpeed->uMaxWriteRate : 0. );

			if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL)
			{
				if (i > 0) 
					g_string_append (sLabel, " - ");
				if  (pSpeed->bAcquisitionOK)
				{
					_cd_speed_formatRate (pSpeed->uReadSpeed, s_readRateFormatted, 11, myDesklet != NULL);
					_cd_speed_formatRate (pSpeed->uWriteSpeed, s_writeRateFormatted, 11, myDesklet != NULL);
					g_string_append_printf (sLabel, "%s : %s %s / %s %s", pSpeed->cName, D_("r"), s_readRateFormatted, D_("w"), s_writeRateFormatted);
				}
				else
				{
					g_string_append_printf (sLabel, "%s : %s", pSpeed->cName, D_("N/A"));
				}
			}

		}
	}

	if (myConfig.iNumberParts + myData.iNumberDisks > 0)
	{
		/// Display data
		CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (s_fValues);
		if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL)
			CD_APPLET_SET_NAME_FOR_MY_ICON (sLabel->str);
	}

	g_string_free (sLabel, TRUE);
	CD_APPLET_LEAVE (TRUE);
}

/* unused atm, will be needed for 'all disks' monitoring option
void _cd_disks_set_data (CDDiskSpeedData *pSpeed, long long unsigned uReadBlocks, long long unsigned uWriteBlocks, double fTimeElapsed)
{
	if (pSpeed->bInitialized)  // la 1ere iteration on ne peut pas calculer le debit.
	{
		pSpeed->uReadSpeed = (uReadBlocks - pSpeed->uReadBlocks) * DISK_BLOCK_SIZE / fTimeElapsed;
		pSpeed->uWriteSpeed = (uWriteBlocks - pSpeed->uWriteBlocks) * DISK_BLOCK_SIZE / fTimeElapsed;
	}
	else
		pSpeed->bInitialized = TRUE;

	pSpeed->uReadBlocks = uReadBlocks;
	pSpeed->uWriteBlocks = uWriteBlocks;
	 //~ cd_warning("%s %u %u", pSpeed->cName, pSpeed->uReadSpeed, pSpeed->uWriteSpeed);

	pSpeed->bAcquisitionOK = TRUE;
}
*/

#define BUFFSIZE (64*1024)
static char buff[BUFFSIZE];

void cd_disks_get_data (CairoDockModuleInstance *myApplet)
{
	g_timer_stop (myData.pClock);
	double fTimeElapsed = g_timer_elapsed (myData.pClock, NULL);
	g_timer_start (myData.pClock);

	if (!(myConfig.iNumberParts + myData.iNumberDisks > 0))
	{
		cairo_dock_downgrade_task_frequency (myData.pPeriodicTask);
		cd_warning("Disks : No disk defined");
	}
	g_return_if_fail ((fTimeElapsed > 0.1) || (myConfig.iNumberParts + myData.iNumberDisks > 0));
	
	if (myConfig.iNumberParts > 0)
	{
		gsize i;
		struct statvfs buffer;
		int            status;
		double *pSize;
		for (i = 0; i < myConfig.iNumberParts; i++)
		{
			pSize = g_list_nth_data (myData.lParts, i);
			status = statvfs(myConfig.cParts[i], &buffer);
			*pSize = 1. - (double) buffer.f_bfree / (double) buffer.f_blocks;
		}
	}

	if (myData.iNumberDisks > 0) 
	{
		char disk_name [16];
		//~ int disk_major;
	
		FILE* fd;
		buff[BUFFSIZE-1] = 0; 
		fd = fopen (DISKS_SPEED_DATA_PIPE, "rb");
		if (!fd) 
		{
			cairo_dock_downgrade_task_frequency (myData.pPeriodicTask);
			cd_warning("Disks : Your kernel doesn't support diskstat. (2.5.70 or above required)");
		}
		else
		{
			//~ cairo_dock_set_normal_task_frequency (myData.pPeriodicTask);
			gsize i;
			CDDiskSpeedData* pSpeed;
			long long unsigned uReadBlocks, uWriteBlocks;
			gboolean bFound;
			for (;;)
			{
				if (!fgets(buff,BUFFSIZE-1,fd))
				{
					fclose(fd);
					break;
				}
		
				/// Using Linux iostat : http://www.kernel.org/doc/Documentation/iostats.txt
				/// gathering fields 3, 6 and 10
				sscanf(buff,  "   %*d    %*d %15s %*u %*u %llu %*u %*u %*u %llu %*u %*u %*u %*u",
					//&disk_major, // field 1
					disk_name,
					&uReadBlocks,
					&uWriteBlocks 
					);
	
				bFound = FALSE;
				if (strlen (disk_name) == 3)
					for (i = 0; i < myData.iNumberDisks; i++)
					{
						pSpeed = g_list_nth_data (myData.lDisks, i);
						if (strcmp (pSpeed->cName, disk_name) == 0)
						{
							if (pSpeed->bInitialized)  // la 1ere iteration on ne peut pas calculer le debit.
							{
								pSpeed->uReadSpeed = (uReadBlocks - pSpeed->uReadBlocks) * DISK_BLOCK_SIZE / fTimeElapsed;
								pSpeed->uWriteSpeed = (uWriteBlocks - pSpeed->uWriteBlocks) * DISK_BLOCK_SIZE / fTimeElapsed;
							}
							else
								pSpeed->bInitialized = TRUE;
							
							pSpeed->uReadBlocks = uReadBlocks;
							pSpeed->uWriteBlocks = uWriteBlocks;
							 //~ cd_warning("%s %u %u", pSpeed->cName, pSpeed->uReadSpeed, pSpeed->uWriteSpeed);
							
							pSpeed->bAcquisitionOK = TRUE;
							bFound = TRUE;
							break;
						}
					}

					// if (myConfig.bListAll && !bFound) // need monitor
						/// create new
				//~ cd_warning("creation");
						//~ pSpeed = g_new0 (CDDiskSpeedData, 1);
						//~ pSpeed->cName = g_strdup (disk_name);
						//~ myData.lDisks = g_list_append (myData.lDisks, pSpeed);
						// speed_set_data (pDisk, new_reads, new_writes);
				//~ } 
			}
		}
	}

}

#if (GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION < 28)
void _reset_parts_list (double *pSize)
{
	g_free (pSize);
}
#endif

void cd_disks_reset_parts_list (CairoDockModuleInstance *myApplet)
{
	if (myConfig.iNumberParts > 0)
	{
#if (GLIB_MAJOR_VERSION > 2) || (GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION >= 28)
		g_list_free_full (myData.lParts, g_free);
#else
		g_list_foreach (myData.lParts, (GFunc) _reset_parts_list, NULL);
#endif
		myData.lParts = NULL;
	}
}


void _reset_one_disk (CDDiskSpeedData *pSpeed)
{
	if (pSpeed->cName != NULL)
		g_free (pSpeed->cName);
	g_free (pSpeed);
}

void cd_disks_reset_disks_list (CairoDockModuleInstance *myApplet)
{
	if (myData.iNumberDisks > 0)
	{
		g_list_foreach (myData.lDisks, (GFunc) _reset_one_disk, NULL);
		g_list_free (myData.lDisks);
		myData.lDisks = NULL;
	}
}



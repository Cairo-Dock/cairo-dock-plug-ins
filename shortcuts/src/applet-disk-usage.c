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

#include <string.h>
#include <mntent.h>
#include <sys/types.h>
#include <sys/statfs.h>
#include <mntent.h>
#include <math.h>

#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-disk-usage.h"


void cd_shortcuts_get_fs_stat (const gchar *cDiskURI, CDDiskUsage *pDiskUsage)
{
	static struct statfs sts;
	const gchar *cMountPath = (strncmp (cDiskURI, "file://", 7) == 0 ? cDiskURI + 7 : cDiskURI);
	//g_print ("checking device on '%s'...\n", cMountPath);
	
	if (statfs (cMountPath, &sts) == 0)
	{
		if (pDiskUsage->iType == 0)
			pDiskUsage->iType = sts.f_type;
		pDiskUsage->iAvail = (long long)sts.f_bavail * sts.f_bsize;  // Blocs libres pour utilisateurs
		pDiskUsage->iFree  = (long long)sts.f_bfree  * sts.f_bsize;  // Blocs libres
		pDiskUsage->iTotal = (long long)sts.f_blocks * sts.f_bsize;  // Nombre total de blocs
		pDiskUsage->iUsed  = pDiskUsage->iTotal - pDiskUsage->iAvail;
		//g_print ("%lld / %lld\n", pDiskUsage->iAvail, pDiskUsage->iTotal);
	}
	else
	{
		pDiskUsage->iTotal = 0;
	}
}


static gboolean _cd_shortcuts_update_disk_usage (CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	CairoContainer *pContainer = CD_APPLET_MY_ICONS_LIST_CONTAINER;
	CDDiskUsage *pDiskUsage;
	Icon *pIcon;
	double fValue;
	GList *ic;
	GList *pIconsList = CD_APPLET_MY_ICONS_LIST;
	for (ic = pIconsList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (pIcon->iType != 6)
			break;
		if (pIcon->cCommand != NULL)
		{
			// get data
			pDiskUsage = CD_APPLET_GET_MY_ICON_DATA (pIcon);
			if (pDiskUsage == NULL)
				continue;
			cd_shortcuts_get_fs_stat (pIcon->cCommand, pDiskUsage);
			
			// update from data
			if (pDiskUsage->iTotal != 0 && (pDiskUsage->iPrevAvail == 0 || (double)fabs (pDiskUsage->iPrevAvail - pDiskUsage->iAvail) / pDiskUsage->iTotal > .001))  // .1 % d'ecart ou info encore non renseignee (un disque n'est jamais totalement vide).
			{
				pDiskUsage->iPrevAvail = pDiskUsage->iAvail;
				switch (myConfig.iDisplayType)
				{
					case CD_SHOW_FREE_SPACE :
						fValue = (double) pDiskUsage->iAvail / pDiskUsage->iTotal;
						cairo_dock_set_size_as_quick_info (pIcon, pContainer, pDiskUsage->iAvail);
					break ;
					case CD_SHOW_USED_SPACE :
						fValue = (double) - pDiskUsage->iUsed / pDiskUsage->iTotal;  // <0 => du vert au rouge.
						cairo_dock_set_size_as_quick_info (pIcon, pContainer, pDiskUsage->iUsed);
					break ;
					case CD_SHOW_FREE_SPACE_PERCENT :
						fValue = (double) pDiskUsage->iAvail / pDiskUsage->iTotal;
						cairo_dock_set_quick_info_printf (pIcon, pContainer, "%.1f%%", 100.*fValue);
					break ;
					case CD_SHOW_USED_SPACE_PERCENT :
						fValue = (double) - pDiskUsage->iUsed / pDiskUsage->iTotal;  // <0 => du vert au rouge.
						cairo_dock_set_quick_info_printf (pIcon, pContainer, "%.1f%%", -100.*fValue);
					break ;
					default:
						fValue = 0.;
					break;
				}
				
				if (myConfig.bDrawBar && pIcon->pIconBuffer != NULL)
				{
					int iWidth, iHeight;
					cairo_dock_get_icon_extent (pIcon, pContainer, &iWidth, &iHeight);
					cairo_surface_t *pSurface = cairo_dock_create_surface_for_icon (pIcon->cFileName, iWidth, iHeight);
					cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
					
					cairo_dock_set_icon_surface_with_bar (pIconContext, pSurface, fValue, pIcon);
					
					cairo_destroy (pIconContext);
					cairo_surface_destroy (pSurface);
				}
				
				cairo_dock_redraw_icon (pIcon, pContainer);
			}
		}
	}
	
	if (myDesklet)
		cairo_dock_redraw_container (myContainer);
	
	CD_APPLET_LEAVE (TRUE);
}


void cd_shortcuts_launch_disk_periodic_task (CairoDockModuleInstance *myApplet)
{
	if (myConfig.iDisplayType != CD_SHOW_NOTHING && myConfig.bListDrives)
	{
		if (myData.pDiskTask == NULL)
		{
			myData.pDiskTask = cairo_dock_new_task (myConfig.iCheckInterval,
				(CairoDockGetDataAsyncFunc) NULL,
				(CairoDockUpdateSyncFunc) _cd_shortcuts_update_disk_usage,
				myApplet);
		}
		cairo_dock_launch_task (myData.pDiskTask);
	}
}

void cd_shortcuts_stop_disk_periodic_task (CairoDockModuleInstance *myApplet)
{
	cairo_dock_stop_task (myData.pDiskTask);
}

void cd_shortcuts_free_disk_periodic_task (CairoDockModuleInstance *myApplet)
{
	cd_shortcuts_stop_disk_periodic_task (myApplet);
	cairo_dock_free_task (myData.pDiskTask);
	myData.pDiskTask = NULL;
}



static void _cd_shortcuts_get_fs_info (const gchar *cDiskURI, GString *sInfo)
{
	const gchar *cMountPath = (strncmp (cDiskURI, "file://", 7) == 0 ? cDiskURI + 7 : cDiskURI);
	struct mntent *me;
	FILE *mtab = setmntent ("/etc/mtab", "r");
	char *search_path;
	int match;
	char *slash;

	if (mtab == NULL)
	{
		cd_warning ("couldn't open /etc/mtab");
		return ;
	}
	
	gchar *cFsInfo = NULL;
	while ((me = getmntent (mtab)) != NULL)
	{
		if (me->mnt_dir && strcmp (me->mnt_dir, cMountPath) == 0)
		{
			g_string_append_printf (sInfo, "Mount point : %s\nFile system : %s\nDevice : %s\nMount options : %s",
				me->mnt_dir,
				me->mnt_type,
				me->mnt_fsname,
				me->mnt_opts);
			if (me->mnt_freq != 0)
				g_string_append_printf (sInfo, "\nBackup frequency : %d days", me->mnt_freq);
			break ;
		}
	}
	
	endmntent (mtab);
}

gchar *cd_shortcuts_get_disk_info (const gchar *cDiskURI, const gchar *cDiskName)
{
	GString *sInfo = g_string_new ("");
	// on recupere les infos de taille.
	CDDiskUsage diskUsage;
	cd_shortcuts_get_fs_stat (cDiskURI, &diskUsage);
	
	// on recupere les infos du file system.
	if (diskUsage.iTotal > 0)  // le disque est monte.
	{
		gchar *cFreeSpace = cairo_dock_get_human_readable_size (diskUsage.iAvail);
		gchar *cCapacity = cairo_dock_get_human_readable_size (diskUsage.iTotal);
		g_string_append_printf (sInfo, "Name : %s\nCapacity : %s\nFree space : %s\n", cDiskName, cCapacity, cFreeSpace);
		g_free (cCapacity);
		g_free (cFreeSpace);
		_cd_shortcuts_get_fs_info (cDiskURI, sInfo);
	}
	else  // disque non monte.
	{
		g_string_append_printf (sInfo, "Name : %s\nNot mounted", cDiskName);
	}
	
	gchar *cInfo = sInfo->str;
	g_string_free (sInfo, FALSE);
	return cInfo;
}

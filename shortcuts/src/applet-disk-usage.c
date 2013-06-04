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
#include <sys/types.h>
#if defined(__FreeBSD__)
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#else
#include <mntent.h>
#include <sys/statfs.h>
#endif

#include <math.h>

#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-disk-usage.h"


void cd_shortcuts_get_fs_stat (const gchar *cDiskURI, CDDiskUsage *pDiskUsage)
{
	static struct statfs sts;
	const gchar *cMountPath = (strncmp (cDiskURI, "file://", 7) == 0 ? cDiskURI + 7 : cDiskURI);
	
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
		pDiskUsage->iAvail = 0;
	}
}

static void _display_disk_usage (Icon *pIcon, GldiContainer *pContainer, CDDiskUsage *pDiskUsage, GldiModuleInstance *myApplet)
{
	double fValue;
	if (pDiskUsage->iTotal != 0 && (pDiskUsage->iPrevAvail == -1 || (double)fabs (pDiskUsage->iPrevAvail - pDiskUsage->iAvail) / pDiskUsage->iTotal > .001))  // .1 % d'ecart ou info encore non renseignee.
	{
		pDiskUsage->iPrevAvail = pDiskUsage->iAvail;
		switch (myConfig.iDisplayType)
		{
			case CD_SHOW_FREE_SPACE :
				fValue = (double) pDiskUsage->iAvail / pDiskUsage->iTotal;
				cairo_dock_set_size_as_quick_info (pIcon, pDiskUsage->iAvail);
			break ;
			case CD_SHOW_USED_SPACE :
				fValue = (double) pDiskUsage->iUsed / pDiskUsage->iTotal;
				cairo_dock_set_size_as_quick_info (pIcon, pDiskUsage->iUsed);
			break ;
			case CD_SHOW_FREE_SPACE_PERCENT :
				fValue = (double) pDiskUsage->iAvail / pDiskUsage->iTotal;
				gldi_icon_set_quick_info_printf (pIcon, "%.1f%%", 100.*fValue);
			break ;
			case CD_SHOW_USED_SPACE_PERCENT :
				fValue = (double) pDiskUsage->iUsed / pDiskUsage->iTotal;
				gldi_icon_set_quick_info_printf (pIcon, "%.1f%%", 100.*fValue);
			break ;
			default:
				fValue = CAIRO_DATA_RENDERER_UNDEF_VALUE;
			break;
		}
		
		if (myConfig.bDrawBar)
			cairo_dock_render_new_data_on_icon (pIcon, pContainer, myDrawContext, &fValue);
		else  // just trigger the redraw for the quick-info
			cairo_dock_redraw_icon (pIcon);
	}
}

void cd_shortcuts_display_disk_usage (Icon *pIcon, GldiModuleInstance *myApplet)
{
	GldiContainer *pContainer = pIcon->pContainer;
	g_return_if_fail (pContainer != NULL);
	CDDiskUsage *pDiskUsage = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	g_return_if_fail (pDiskUsage != NULL);
	_display_disk_usage (pIcon, pContainer, pDiskUsage, myApplet);
}

static gboolean _cd_shortcuts_update_disk_usage (GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	GldiContainer *pContainer = CD_APPLET_MY_ICONS_LIST_CONTAINER;
	CDDiskUsage *pDiskUsage;
	Icon *pIcon;
	GList *ic;
	GList *pIconsList = CD_APPLET_MY_ICONS_LIST;
	for (ic = pIconsList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (pIcon->cCommand != NULL)  // skip separators
		{
			// get data
			pDiskUsage = CD_APPLET_GET_MY_ICON_DATA (pIcon);
			if (pDiskUsage == NULL)  // not a drive (eg, network or bookmark)
			{
				if (pIcon->iGroup == (CairoDockIconGroup) CD_BOOKMARK_GROUP)  // drives are listed first, and Home is always the first bookmark (and the only one to have disk data), so if we got a bookmark with no disk data, we can quit the loop.
					break;
				else
					continue;
			}
			cd_shortcuts_get_fs_stat (pIcon->cCommand, pDiskUsage);
			
			// draw
			_display_disk_usage (pIcon, pContainer, pDiskUsage, myApplet);
		}
	}
	
	if (myDesklet)
		cairo_dock_redraw_container (myContainer);
	
	CD_APPLET_LEAVE (TRUE);
}


void cd_shortcuts_launch_disk_periodic_task (GldiModuleInstance *myApplet)
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

void cd_shortcuts_free_disk_periodic_task (GldiModuleInstance *myApplet)
{
	cairo_dock_free_task (myData.pDiskTask);
	myData.pDiskTask = NULL;
}



static void _cd_shortcuts_get_fs_info (const gchar *cDiskURI, GString *sInfo)
{
	const gchar *cMountPath = (strncmp (cDiskURI, "file://", 7) == 0 ? cDiskURI + 7 : cDiskURI);
	
	#if defined(__FreeBSD__)
	struct statfs *me;
	int i, count = getfsstat(me, NULL, MNT_WAIT);
	if (count>0)
	{
		for (i=0; i<count; i++)
		{
			if (me->f_mntonname && strcmp (me->f_mntonname, cMountPath) == 0)
			{
				g_string_append_printf (sInfo, "%s %s\n%s %s\n%s %s\n%s %s",
					D_("Mount point:"), me->f_mntonname,
					D_("File system:"), me->f_mntfromname,
					D_("Device:"), me->f_fstypename,
					D_("Mount options:"), me->f_charspare);
				// if (me->mnt_freq != 0)
				// g_string_append_printf (sInfo, "\nBackup frequency : %d days", me->mnt_freq);
				break ;
			}
		}
	}
	else
	{
		cd_warning ("error getfsstat...");
		return ;
	}
	#else
	struct mntent *me;
	FILE *mtab = setmntent ("/etc/mtab", "r");
	if (mtab == NULL)
	{
		cd_warning ("couldn't open /etc/mtab");
		return ;
	}
	
	while ((me = getmntent (mtab)) != NULL)
	{
		if (me->mnt_dir && strcmp (me->mnt_dir, cMountPath) == 0)
		{
			g_string_append_printf (sInfo, "%s %s\n%s %s\n%s %s\n%s %s",
				D_("Mount point:"), me->mnt_dir,
				D_("File system:"), me->mnt_type,
				D_("Device:"), me->mnt_fsname,
				D_("Mount options:"), me->mnt_opts);
			if (me->mnt_freq != 0)
				g_string_append_printf (sInfo, "\n%s %d %s",
					D_("Backup frequency:"), me->mnt_freq, D_("days"));
			break ;
		}
	}
	
	endmntent (mtab);
	#endif
}

gchar *cd_shortcuts_get_disk_info (const gchar *cDiskURI, const gchar *cDiskName)
{
	GString *sInfo = g_string_new ("");
	// on recupere les infos de taille.
	CDDiskUsage diskUsage;
	memset (&diskUsage, 0, sizeof (CDDiskUsage));
	cd_shortcuts_get_fs_stat (cDiskURI, &diskUsage);
	
	// on recupere les infos du file system.
	if (diskUsage.iTotal > 0)  // info are available
	{
		gchar *cFreeSpace = cairo_dock_get_human_readable_size (diskUsage.iAvail);
		gchar *cCapacity = cairo_dock_get_human_readable_size (diskUsage.iTotal);
		g_string_append_printf (sInfo, "%s %s\n%s %s\n%s %s\n", // added '\n' -> _cd_shortcuts_get_fs_info
			D_("Name:"), cDiskName,
			D_("Capacity:"), cCapacity,
			D_("Free space:"), cFreeSpace);
		g_free (cCapacity);
		g_free (cFreeSpace);
		_cd_shortcuts_get_fs_info (cDiskURI, sInfo);
	}
	else if (strncmp (cDiskURI, "computer:/", 10) == 0 || strncmp (cDiskURI, "file:/", 6) == 0)  // no info on a local mount point => it's not mounted
	{
		g_string_append_printf (sInfo, "%s %s\n%s",
			D_("Name:"), cDiskName, D_("Not mounted"));
	}
	else  // not a local mount point => a distant connection
	{
		g_string_append_printf (sInfo, "%s %s\n%s %s",
			D_("Name:"), cDiskName,
			D_("URL:"), cDiskURI);
	}
	
	gchar *cInfo = sInfo->str;
	g_string_free (sInfo, FALSE);
	return cInfo;
}

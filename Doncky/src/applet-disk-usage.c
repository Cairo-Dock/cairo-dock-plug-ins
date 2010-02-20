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

#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-disk-usage.h"
#include "applet-xml.h"
#include "applet-draw.h"


static void cd_doncky_get_fs_stat (const gchar *cDiskURI, CDDiskUsage *pDiskUsage)
{
	static struct statfs sts;
	const gchar *cMountPath = (strncmp (cDiskURI, "file://", 7) == 0 ? cDiskURI + 7 : cDiskURI);
	//g_print ("checking device on '%s'...\n", cMountPath);
	
	if (statfs (cMountPath, &sts) == 0)
	{
		if (pDiskUsage->iType == 0)
			pDiskUsage->iType = sts.f_type;
		pDiskUsage->iPrevAvail = pDiskUsage->iAvail;
		pDiskUsage->iAvail = (long long)sts.f_bavail * sts.f_bsize;  // Blocs libres pour utilisateurs
		pDiskUsage->iFree  = (long long)sts.f_bfree  * sts.f_bsize;  // Blocs libres
		pDiskUsage->iTotal = (long long)sts.f_blocks * sts.f_bsize;  // Nombre total de blocs
		pDiskUsage->iUsed  = pDiskUsage->iTotal - pDiskUsage->iAvail;
	}
	else
	{
		pDiskUsage->iTotal = 0;
	}
}


static void cd_doncky_get_fs_info (const gchar *cDiskURI, GString *sInfo, const int iType)
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
			switch (iType)
			{
				case 0 : // fs_type				
					g_string_append_printf (sInfo, "%s", me->mnt_type);
				break ;
				case 1 : // fs_device
					g_string_append_printf (sInfo, "%s", me->mnt_fsname);
				break ;							
			}			

			break ;
		}
	}
	endmntent (mtab);
}



gchar *cd_doncky_get_disk_info (const gchar *cDiskURI, const int iType)
{
	gchar *cReturn = "";	
	GString *sInfo = g_string_new ("");
	
	// on recupere les infos de taille.
	CDDiskUsage diskUsage;
	cd_doncky_get_fs_stat (cDiskURI, &diskUsage);
	
	// on recupere les infos du file system.
	if (diskUsage.iTotal > 0)  // le disque est monte.
	{
		gdouble fFreeSpace;
		gdouble fUsedSpace;		
		fFreeSpace = (double) diskUsage.iAvail / diskUsage.iTotal;;
		fUsedSpace = (double) - diskUsage.iUsed / diskUsage.iTotal;
		gdouble fFreeSpacePerc = 100.*fFreeSpace;
		gdouble fUsedSpacePerc = -100.*fUsedSpace;
		
		switch (iType)
		{
			case 0 : // fs_size				
				cReturn = cairo_dock_get_human_readable_size (diskUsage.iTotal);
				rtrim(cReturn, "G" ); // On supprime le G
			break ;
			case 1 : // fs_free
				cReturn = cairo_dock_get_human_readable_size (diskUsage.iAvail);
				rtrim(cReturn, "G" ); // On supprime le G
			break ;
			case 2 : // fs_used
				cReturn = cairo_dock_get_human_readable_size (diskUsage.iUsed);
				rtrim(cReturn, "G" ); // On supprime le G
			break ;
			case 3 : // fs_freeperc
				cReturn = g_strdup_printf ("%.0f", fFreeSpacePerc);
			break ;
			case 4 : // fs_usedperc
				cReturn = g_strdup_printf ("%.0f", fUsedSpacePerc);
			break ;
			case 5 : // fs_type			
				cd_doncky_get_fs_info (cDiskURI, sInfo, 0);				
				cReturn = sInfo->str;
			break ;
			case 6 : // fs_device
				cd_doncky_get_fs_info (cDiskURI, sInfo, 1);			
				cReturn = sInfo->str; // On obtient un résultat du type /dev/sda1
				ltrim(cReturn, "/dev/" ); // On supprime de /dev/
			break ;				
		}
	}
	else  // disque non monte.
	{
		cReturn = g_strdup_printf ("-");
	}	
	g_string_free (sInfo, FALSE);
	return cReturn;
}


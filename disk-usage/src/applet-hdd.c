#include "applet-hdd.h"
#include "applet-struct.h"

gchar *cd_human_readable(long long num) {
	gint cpt = 0;
	gdouble fnum = 0;
	const gchar *SUFFIXT[] = {"o","ko","Mo","Go","To"};
	gchar **suffix = SUFFIXT;
	
	if (num < 1024LL)
		return g_strdup_printf("%lld%s\n",num, *suffix);
	
	while (num / 1024 >= 1000LL && **(suffix + 2)) {
		num /= 1024;
		suffix++;
	}

	suffix++;
	fnum = num / 1024.0;
	
	return g_strdup_printf("%.2f%s\n",fnum,*suffix);
}


gchar *cd_get_fs_type(const char *path) {

	struct mntent *me;
	FILE *mtab = setmntent("/etc/mtab", "r");
	char *search_path;
	int match;
	char *slash;

	if (mtab == NULL) {
		return "unknown";
	}

	me = getmntent(mtab);

	// On cherche le path dans etc/mtab
	search_path = (char *)g_strdup_printf((char *)path);
	do {
		while ((match = strcmp(search_path, me->mnt_dir))
				&& getmntent(mtab));
		if (!match)
			break;
		fseek(mtab, 0, SEEK_SET);
		slash = (char *)strrchr(search_path, (char)'/');
		if (slash == NULL)
			g_print("invalid path '%s'\n", path);
		if (strlen(slash) == 1)
			*(slash) = '\0';
		else if (strlen(slash) > 1)
			*(slash + 1) = '\0';
		else
			g_print("found a crack in the matrix!\n");
	} while (strlen(search_path) > 0);
	free(search_path);

	endmntent(mtab);

	if (me && !match)
		return strcmp(me->mnt_type,"fuseblk")==0 ? "ntfs" : g_strdup (me->mnt_type);
	
	return "unknown";
}

void cd_mount_unmount_device (CairoDockModuleInstance *myApplet) {
	
	g_print ("%s %s... \n",myData.bAcquisitionOK ? "Unmount" : "Mount", myConfig.cDevice);
	//g_free (cDevice);
}

void cd_hdd_read_data(CairoDockModuleInstance *myApplet) {
	
	g_timer_stop (myData.pClock);
	double fTimeElapsed = g_timer_elapsed (myData.pClock, NULL);
	g_timer_start (myData.pClock);
	
	struct statfs   sts;
	
	if (statfs (myConfig.cDevice, & sts) != 0) {
		cd_warning ("Unable to detect file system");
		myData.bAcquisitionOK = FALSE;
		return;
	}
	
	myData.llAvail = (long long)sts.f_bavail * sts.f_bsize;
	myData.llFree  = (long long)sts.f_bfree  * sts.f_bsize;
	myData.llTotal = (long long)sts.f_blocks * sts.f_bsize;
	myData.llUsed  = (long long)myData.llTotal - myData.llAvail;

	if (myConfig.iPercentDisplay == CD_AVAIL_SPACE)
		myData.fPourcent = (myData.llAvail * 1.0) / (myData.llTotal * 1.0);
	else if (myConfig.iPercentDisplay == CD_USED_SPACE)
		myData.fPourcent = (myData.llUsed * 1.0) / (myData.llTotal * 1.0);
	
	myData.cType = cd_get_fs_type(myConfig.cDevice);
	
	myData.bAcquisitionOK = TRUE;
}

gboolean cd_hdd_update_from_data (CairoDockModuleInstance *myApplet) {
	
	if ( ! myData.bAcquisitionOK)
	{
		if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL)
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cDefaultName);
		else if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
			if (!myDesklet)
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("N/A");
			else {
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%s:N/A",myConfig.cDefaultName);
				gtk_widget_queue_draw (myDesklet->pWidget);
			}
		
		CD_APPLET_RENDER_GAUGE (myData.pGauge, 0.);
	}
	else
	{
		CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cDefaultName);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		if (myConfig.iInfoDisplay != CAIRO_DOCK_INFO_NONE)
		{
			if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
			{
				if (myDesklet) {
					CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ((myData.fPourcent < 0.1 ? "%s:%.1f%%" : "%s:%.0f%%"),myConfig.cDefaultName,(myData.fPourcent * 100.0));
					gtk_widget_queue_draw (myDesklet->pWidget);
				} else
					CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ((myData.fPourcent < 0.1 ? "%.1f%%" : "%.0f%%"),(myData.fPourcent * 100.0));
			}
			else
			{
				if (myDock)
					CD_APPLET_SET_NAME_FOR_MY_ICON_PRINTF ("HDD : %.1f%%", (myData.fPourcent * 100.0));
			}
		}
			

		CD_APPLET_RENDER_GAUGE (myData.pGauge, myData.fPourcent);
	}
	
	return myData.bAcquisitionOK;
}

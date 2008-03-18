/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#define WNCK_I_KNOW_THIS_IS_UNSTABLE 1
#include <libwnck/libwnck.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <glib/gi18n.h>
#include "applet-struct.h"
#include "applet-switcher.h"
 

#define PATH "/apps/compiz/general/screen0/options/"
#define KEY "/apps/compiz/general/screen0/options/vsize"


extern AppletConfig myConfig;
extern AppletData myData;
extern Switcher_Applet mySwitcher;

CD_APPLET_INCLUDE_MY_VARS


double vp_vscale(Switcher_Applet *switcher)
{
	return  (double)wnck_screen_get_height(mySwitcher.wnck_screen )/(double)wnck_workspace_get_height(wnck_screen_get_active_workspace(mySwitcher.wnck_screen) );

}

double vp_hscale(Switcher_Applet *switcher)
{
	return  (double)wnck_screen_get_width(mySwitcher.wnck_screen )/(double)wnck_workspace_get_width(wnck_screen_get_active_workspace(mySwitcher.wnck_screen) );

}

void calc_dimensions(Switcher_Applet *switcher)
{
mySwitcher.wnck_screen = wnck_screen_get_default(); 
	//wnck_screen_force_update(switcher->wnck_screen);  		
//FIXME this is no longer screen width/height  it's workspace
	int wnck_ws_width=wnck_workspace_get_width(wnck_screen_get_active_workspace(mySwitcher.wnck_screen) );	
	int wnck_ws_height=wnck_workspace_get_height(wnck_screen_get_active_workspace(mySwitcher.wnck_screen) );
	int wnck_scr_width=wnck_screen_get_width(mySwitcher.wnck_screen );	
	int wnck_scr_height=wnck_screen_get_height(mySwitcher.wnck_screen );

	double ws_ratio,scr_ratio;	
	
	ws_ratio=wnck_ws_width/ (double)wnck_ws_height;
	scr_ratio=wnck_scr_width/ (double)wnck_scr_height;

	printf("cols = %d,  rows=%d \n",mySwitcher.cols,mySwitcher.rows);		
	mySwitcher.mini_work_height=mySwitcher.height*mySwitcher.applet_scale/mySwitcher.rows;
	mySwitcher.mini_work_width=mySwitcher.mini_work_height*mySwitcher.applet_scale*scr_ratio*(double)wnck_ws_width/(double)wnck_scr_width*vp_vscale(switcher);
	mySwitcher.width=switcher->mini_work_width*mySwitcher.cols;
}

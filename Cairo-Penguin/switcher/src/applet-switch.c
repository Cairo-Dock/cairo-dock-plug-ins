/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#define WNCK_I_KNOW_THIS_IS_UNSTABLE 1
#include <libwnck/libwnck.h>
#include <stdlib.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <glib/gi18n.h>
#include <gconf/gconf-client.h>
#include <string.h>
#include <cairo-dock.h>
#include "applet-struct.h"
#include "applet-read-data.h"
 

#define PATH "/apps/compiz/general/screen0/options/"
#define KEY "/apps/compiz/general/screen0/options/vsize"


extern AppletConfig myConfig;
extern AppletData myData;
extern Switcher_Applet mySwitcher;
CD_APPLET_INCLUDE_MY_VARS


void num_obtain(int **numdesks)
{


	
//myData.switchers.client = gconf_client_get_default ();
*numdesks = gconf_client_get_int (myData.switchers.client, "/apps/compiz/general/screen0/options/number_of_desktops", NULL);
//myData.switchers.numrows = gconf_client_get_int (myData.switchers.client, "/apps/compiz/general/screen0/options/vsize", NULL);
//myData.switchers.numcols = gconf_client_get_int (myData.switchers.client, "/apps/compiz/general/screen0/options/hsize", NULL);
//*numdesks="8";

printf("rows : %d \n", myData.switchers.numdesks);
//printf("rows : %d \n", myData.switchers.numrows);
//printf("rows : %d \n", myData.switchers.numcols);

}

/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <libxklavier/xklavier.h>

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-xklavier.h"


void cd_xkbd_set_prev_next_group (int iDelta)
{
	const XklEngine *pEngine = xkl_engine_get_instance (cairo_dock_get_Xdisplay ());
	Window Xid = cairo_dock_get_current_active_window ();
	if (Xid == 0)
		Xid = DefaultRootWindow (cairo_dock_get_Xdisplay ());
	XklState state;
	xkl_engine_get_state (pEngine, Xid, &state);
	cd_debug ("keyboard current state : %d;%d", state.group, state.indicators);
	
	int i=0, n = xkl_engine_get_num_groups (pEngine);
	const gchar **pGroupNames = xkl_engine_get_groups_names (pEngine);
	do  // on passe au groupe suivant/precedent en sautant les faux (-).
	{
		i ++;
		state.group += iDelta;  // xkl_engine_get_next_group ne marche pas.
		if (state.group == n)
			state.group = 0;
		else if (state.group < 0)
			state.group = n - 1;
	} while (i < n && (pGroupNames[state.group] == NULL || *pGroupNames[state.group] == '-'));
	
	///xkl_engine_allow_one_switch_to_secondary_group (pEngine);  // sert a quoi ??
	xkl_engine_save_state (pEngine, Xid, &state);
	xkl_engine_lock_group (pEngine, state.group);  // sert a quoi ??
}

void cd_xkbd_set_group (int iNumGroup)
{
	const XklEngine *pEngine = xkl_engine_get_instance (cairo_dock_get_Xdisplay ());
	Window Xid = cairo_dock_get_current_active_window ();
	if (Xid == 0)
		Xid = DefaultRootWindow (cairo_dock_get_Xdisplay ());
	XklState state;
	xkl_engine_get_state (pEngine, Xid, &state);
	cd_debug ("keyboard current state : %d;%d", state.group, state.indicators);
	
	state.group = iNumGroup;
	
	///xkl_engine_allow_one_switch_to_secondary_group (pEngine);  // sert a quoi ??
	xkl_engine_save_state (pEngine, Xid, &state);
	xkl_engine_lock_group (pEngine, state.group);  // sert a quoi ??
}

gboolean cd_xkbd_keyboard_state_changed (CairoDockModuleInstance *myApplet, Window *pWindow)
{
	g_print ("%s (%d)\n", __func__, (pWindow ? *pWindow : 0));
	///if (pWindow == NULL)
	///	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	Window Xid = (pWindow ? *pWindow : 0);
	if (Xid == 0)
		Xid = DefaultRootWindow (cairo_dock_get_Xdisplay ());
	gchar *cShortGroupName = NULL;
	const gchar *cCurrentGroup = NULL;
	GString *sCurrentIndicator = NULL;
	gboolean bRedrawSurface = TRUE;
	
	if (Xid != 0)
	{
		const XklEngine *pEngine = xkl_engine_get_instance (cairo_dock_get_Xdisplay ());
		XklState state;
		xkl_engine_get_state (pEngine, Xid, &state);
		
		if (myData.iCurrentGroup == state.group && myData.iCurrentIndic == state.indicators)
			return CAIRO_DOCK_LET_PASS_NOTIFICATION;
		if (myData.iCurrentGroup == state.group)
			bRedrawSurface = FALSE;
		
		int n = xkl_engine_get_num_groups (pEngine);
		g_return_val_if_fail (n > 0, CAIRO_DOCK_LET_PASS_NOTIFICATION);
		const gchar **pGroupNames = xkl_engine_get_groups_names (pEngine);
		const gchar **pIndicatorNames = xkl_engine_get_indicators_names (pEngine);
		
		cCurrentGroup = pGroupNames[state.group];
		g_print ("group : %d (%s)\n", state.group, cCurrentGroup);
		
		int i;
		if (myConfig.bShowKbdIndicator)
		{
			if (myData.iCurrentGroup == -1 && state.indicators == 0)  // c'est probablement un bug dans libxklavier	qui fait que l'indicateur n'est pas defini au debut.
			{
				g_print ("on force le num lock\n");
				state.indicators = 2;  // num lock, enfin j'espere que c'est toujours le cas ...
				xkl_engine_save_state (pEngine, Xid, &state);
				xkl_engine_lock_group (pEngine, state.group);
			}
			sCurrentIndicator = g_string_new ("");
			for (i = 0; i < n; i ++)
			{
				if ((state.indicators >> i) & 1)
				{
					g_string_append_printf (sCurrentIndicator, "%s%s", (sCurrentIndicator->len == 0 ? "" : " / "), pIndicatorNames[i]);
				}
			}
		}
		
		myData.iCurrentGroup = state.group;
		myData.iCurrentIndic = state.indicators;
		
		cShortGroupName = cairo_dock_cut_string (cCurrentGroup, 3);
		if (strlen(cShortGroupName) > 3)
			cShortGroupName[strlen(cShortGroupName)-3] = '\0';
		/*for (i = 0; i < n; i ++)
		{
			g_print ("kbd group name %d : %s - %s\n", i, pGroupNames[i], pIndicatorNames[i]);
		}
		XklConfigRec *pConfigRec = xkl_config_rec_new ();
		xkl_config_rec_get_from_server (pConfigRec, pEngine);
		if (pConfigRec->layouts != NULL)
		{
			for (i = 0; pConfigRec->layouts[i] != NULL; i ++)
				g_print(" layout : %s\n", pConfigRec->layouts[i]);
		}
		if (pConfigRec->variants != NULL)
		{
			for (i = 0; pConfigRec->variants[i] != NULL; i ++)
				g_print(" variants : %s\n", pConfigRec->variants[i]);
		}
		if (pConfigRec->options != NULL)
		{
			for (i = 0; pConfigRec->options[i] != NULL; i ++)
				g_print(" options : %s\n", pConfigRec->options[i]);
		}*/
	}
	
	cd_xkbd_update_icon (cCurrentGroup, cShortGroupName, sCurrentIndicator ? sCurrentIndicator->str : NULL, bRedrawSurface);
	g_free (cShortGroupName);
	if (sCurrentIndicator != NULL)
		g_string_free (sCurrentIndicator, TRUE);
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

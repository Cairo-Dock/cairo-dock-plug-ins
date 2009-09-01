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
	Display *dsp = (Display*)cairo_dock_get_Xdisplay (); // const
	XklEngine *pEngine = xkl_engine_get_instance (dsp);  // const
	Window Xid = cairo_dock_get_current_active_window ();
	if (Xid == 0)
		Xid = DefaultRootWindow (dsp);
	XklState state;
	xkl_engine_get_state (pEngine, Xid, &state);
	cd_debug ("keyboard current state : %d;%d +%d", state.group, state.indicators, iDelta);
	
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
	
	cd_debug ("keyboard new state : %d", state.group);
	xkl_engine_allow_one_switch_to_secondary_group (pEngine);  // sert a quoi ??
	xkl_engine_save_state (pEngine, Xid, &state);
	xkl_engine_lock_group (pEngine, state.group);  // sert a quoi ??
}

void cd_xkbd_set_group (int iNumGroup)
{
	Display *dsp = (Display*)cairo_dock_get_Xdisplay (); // const
	XklEngine *pEngine = xkl_engine_get_instance (dsp); // const
	Window Xid = cairo_dock_get_current_active_window ();
	if (Xid == 0)
		Xid = DefaultRootWindow (dsp);
	XklState state;
	xkl_engine_get_state (pEngine, Xid, &state);
	cd_debug ("keyboard current state : %d;%d", state.group, state.indicators);
	
	state.group = iNumGroup;
	
	xkl_engine_allow_one_switch_to_secondary_group (pEngine);  // sert a quoi ??
	xkl_engine_save_state (pEngine, Xid, &state);
	xkl_engine_lock_group (pEngine, state.group);  // sert a quoi ??
}

gboolean cd_xkbd_keyboard_state_changed (CairoDockModuleInstance *myApplet, Window *pWindow)
{
	cd_debug ("%s (window:%ld)", __func__, (pWindow ? *pWindow : 0));
	///if (pWindow == NULL)
	///	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	Display *dsp = (Display*)cairo_dock_get_Xdisplay ();
	Window Xid = (pWindow ? *pWindow : 0);
	if (Xid == 0)
		Xid = DefaultRootWindow (dsp);
	gchar *cShortGroupName = NULL;
	const gchar *cCurrentGroup = NULL;
	GString *sCurrentIndicator = NULL;
	gboolean bRedrawSurface = TRUE;
	
	if (Xid != 0)
	{
		XklEngine *pEngine = xkl_engine_get_instance (dsp); // const
		XklState state;
		xkl_engine_get_state (pEngine, Xid, &state);
		
		if (myData.iCurrentGroup == state.group && myData.iCurrentIndic == state.indicators)
			return CAIRO_DOCK_LET_PASS_NOTIFICATION;
		if (myData.iCurrentGroup == state.group)
			bRedrawSurface = FALSE;
		
		// on recupere le groupe courant.
		int n = xkl_engine_get_num_groups (pEngine);
		g_return_val_if_fail (n > 0, CAIRO_DOCK_LET_PASS_NOTIFICATION);
		const gchar **pGroupNames = xkl_engine_get_groups_names (pEngine);
		const gchar **pIndicatorNames = xkl_engine_get_indicators_names (pEngine);
		
		cCurrentGroup = pGroupNames[state.group];
		cd_debug ("group : %d (%s)", state.group, cCurrentGroup);
		
		// on recupere l'indicateur courant.
		int i;
		if (myConfig.bShowKbdIndicator)
		{
			if (myData.iCurrentGroup == -1 && state.indicators == 0)  // c'est probablement un bug dans libxklavier	qui fait que l'indicateur n'est pas defini au debut.
			{
				cd_debug ("on force le num lock");
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
		
		// on se souvient de l'etat courant.
		myData.iCurrentGroup = state.group;
		myData.iCurrentIndic = state.indicators;
		
		// on construit le nom court du groupe courant.
		int index = 0;
		for (i = 0; i < state.group; i ++)  // on cherche les noms identiques.
		{
			if (strncmp (cCurrentGroup, pGroupNames[i], 3) == 0)
				index ++;
		}
		cShortGroupName = g_strndup (cCurrentGroup, 3);  //cairo_dock_cut_string (cCurrentGroup, 3);
		//if (cShortGroupName && strlen(cShortGroupName) > 3)
		//	cShortGroupName[strlen(cShortGroupName)-3] = '\0';
		if (index != 0)
		{
			gchar *tmp = cShortGroupName;
			cShortGroupName = g_strdup_printf ("%s%d", cShortGroupName, index+1);
			g_free (tmp);
		}
		
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

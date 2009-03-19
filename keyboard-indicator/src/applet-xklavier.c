/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <libxklavier/xklavier.h>

#include "applet-struct.h"
#include "applet-xklavier.h"


void cd_xkbd_set_prev_next_group (int iDelta)
{
	const XklEngine *pEngine = xkl_engine_get_instance (cairo_dock_get_Xdisplay ());
	Window Xid = cairo_dock_get_current_active_window ();
	XklState state;
	xkl_engine_get_state (pEngine, Xid, &state);
	cd_debug ("keyboard current state : %d;%d", state.group, state.indicators);
	
	int n = xkl_engine_get_num_groups (pEngine);
	
	state.group += iDelta;  // xkl_engine_get_next_group ne marche pas.
	if (state.group == n)
		state.group = 0;
	else if (state.group < 0)
		state.group = n - 1;
	
	///xkl_engine_allow_one_switch_to_secondary_group (pEngine);  // sert a quoi ??
	xkl_engine_save_state (pEngine, Xid, &state);
	xkl_engine_lock_group (pEngine, state.group);  // sert a quoi ??
}

void cd_xkbd_set_group (int iNumGroup)
{
	const XklEngine *pEngine = xkl_engine_get_instance (cairo_dock_get_Xdisplay ());
	Window Xid = cairo_dock_get_current_active_window ();
	XklState state;
	xkl_engine_get_state (pEngine, Xid, &state);
	cd_debug ("keyboard current state : %d;%d", state.group, state.indicators);
	
	state.group = iNumGroup;
	
	///xkl_engine_allow_one_switch_to_secondary_group (pEngine);  // sert a quoi ??
	xkl_engine_save_state (pEngine, Xid, &state);
	xkl_engine_lock_group (pEngine, state.group);  // sert a quoi ??
}


void cd_xkbd_update_icon (const gchar *cGroupName, const gchar *cShortGroupName, const gchar *cIndicatorName)
{
	if (myData.pBackgroundSurface != NULL)
		cairo_dock_set_icon_surface_full (myDrawContext, myData.pBackgroundSurface, 1., 1., myIcon, myContainer);
	else
		cairo_dock_erase_cairo_context (myDrawContext);
	
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	int iTextWidth, iTextHeight;
	double fTextXOffset, fTextYOffset;
	cairo_surface_t *pTextSurface = cairo_dock_create_surface_from_text_full (cShortGroupName,
		myDrawContext,
		&myConfig.textDescription,
		fMaxScale,
		iWidth,
		&iTextWidth, &iTextHeight, &fTextXOffset, &fTextYOffset);
	if (pTextSurface != NULL)
	{
		cairo_set_source_surface (
			myDrawContext,
			pTextSurface,
			0.,
			0.);
		cairo_paint (myDrawContext);
		cairo_surface_destroy (pTextSurface);
	}
	
	CD_APPLET_UPDATE_REFLECT_ON_MY_ICON;
	
	if (g_bUseOpenGL)
		cairo_dock_update_icon_texture (myIcon);
	
	CD_APPLET_SET_NAME_FOR_MY_ICON (cGroupName);
	
	CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (cIndicatorName);
	
	CD_APPLET_REDRAW_MY_ICON;
}


gboolean cd_xkbd_keyboard_state_changed (CairoDockModuleInstance *myApplet, Window *pWindow)
{
	if (pWindow == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	Window Xid = *pWindow;
	if (Xid == 0)
		Xid = DefaultRootWindow (cairo_dock_get_Xdisplay ());
	gchar *cShortGroupName = NULL;
	const gchar *cCurrentGroup = NULL;
	GString *sCurrentIndicator = NULL;
	
	if (Xid != 0)
	{
		const XklEngine *pEngine = xkl_engine_get_instance (cairo_dock_get_Xdisplay ());
		XklState state;
		xkl_engine_get_state (pEngine, Xid, &state);
		
		int n = xkl_engine_get_num_groups (pEngine);
		g_return_val_if_fail (n > 0, CAIRO_DOCK_LET_PASS_NOTIFICATION);
		const gchar **pGroupNames = xkl_engine_get_groups_names (pEngine);
		const gchar **pIndicatorNames = xkl_engine_get_indicators_names (pEngine);
		
		cCurrentGroup = pGroupNames[state.group];
		
		int i;
		if (myConfig.bShowKbdIndicator)
		{
			sCurrentIndicator = g_string_new ("");
			for (i = 0; i < n; i ++)
			{
				if ((state.indicators >> i) & 1)
				{
					g_string_append_printf (sCurrentIndicator, "%s%s", (sCurrentIndicator->len == 0 ? "" : " / "), pIndicatorNames[i]);
				}
			}
		}
		
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
	
	cd_xkbd_update_icon (cCurrentGroup, cShortGroupName, sCurrentIndicator ? sCurrentIndicator->str : NULL);
	g_free (cShortGroupName);
	if (sCurrentIndicator != NULL)
		g_string_free (sCurrentIndicator, TRUE);
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


void cd_xkbd_render_opengl (CairoDockModuleInstance *myApplet)
{
	
}
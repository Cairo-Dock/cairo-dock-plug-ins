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
	g_print ("%s (%s;%s)\n", __func__, cGroupName, cShortGroupName);
	//\__________________ On sauvegarde l'ancienne surface/texture.
	if (myData.pOldSurface != NULL)
		cairo_surface_destroy (myData.pOldSurface);
	if (myData.iOldTexture != 0)
		_cairo_dock_delete_texture (myData.iOldTexture);
	myData.pOldSurface = myData.pCurrentSurface;
	myData.iOldTexture = myData.iCurrentTexture;
	myData.iOldTextWidth = myData.iCurrentTextWidth;
	myData.iOldTextHeight = myData.iCurrentTextHeight;
	
	//\__________________ On cree la nouvelle surface (la taille du texte peut avoir change).
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	double fTextXOffset, fTextYOffset;
	myData.pCurrentSurface = cairo_dock_create_surface_from_text_full (cShortGroupName,
		myDrawContext,
		&myConfig.textDescription,
		fMaxScale,
		iWidth,
		&myData.iCurrentTextWidth, &myData.iCurrentTextHeight, &fTextXOffset, &fTextYOffset);
	if (g_bUseOpenGL)
	{
		myData.iCurrentTexture = cairo_dock_create_texture_from_surface (myData.pCurrentSurface);
	}
	
	//\__________________ On lance une transition entre ancienne et nouvelle surface/texture.
	cairo_dock_set_transition_on_icon (myIcon, myContainer, myDrawContext,
		(CairoDockTransitionRenderFunc) cd_xkbd_render_step_cairo,
		(CairoDockTransitionGLRenderFunc) cd_xkbd_render_step_opengl,
		TRUE,  // bFastPace
		1000,  // 1s
		TRUE,  // bRemoveWhenFinished
		myApplet);
	
	//\__________________ On met a jour le reste.
	CD_APPLET_SET_NAME_FOR_MY_ICON (cGroupName);
	
	CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (cIndicatorName);
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
		
		if (myData.iCurrentGroup == state.group)
			return CAIRO_DOCK_LET_PASS_NOTIFICATION;
		myData.iCurrentGroup = state.group;
		
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


gboolean cd_xkbd_render_step_opengl (CairoDockModuleInstance *myApplet)
{
	double f;
	if (cairo_dock_has_transition (myIcon))
		f = cairo_dock_get_transition_fraction (myIcon);
	else
		f = 1.;
	
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	
	cairo_dock_set_perspective_view (iWidth, iHeight);
	glScalef (1., -1., 1.);

	double fTheta = - 45. + f * 90.;  // -45 -> 45
	glTranslatef (0., 0., - iWidth * sqrt(2)/2 * cos (fTheta/180.*G_PI));  // pour faire tenir le cube dans la fenetre.
	glEnable (GL_DEPTH_TEST);
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_alpha ();
	_cairo_dock_set_alpha (1.);
	
	// fond
	if (myData.iBackgroundTexture != 0)
		cairo_dock_apply_texture_at_size (myData.iBackgroundTexture, iWidth, iHeight);
	
	// image precedente.
	if (fTheta < 25 && myData.iOldTexture != 0)  // inutile de dessiner si elle est derriere l'image courante, par l'effet de perspective (en fait 22.5, mais bizarrement ca a l'air un peu trop tot).
	{
		glPushMatrix ();
		glRotatef (45. + fTheta, 0., 1., 0.);  // 0 -> 90
		glTranslatef (0., 0., (myData.iCurrentTextWidth ? myData.iCurrentTextWidth : iWidth)/2);
		cairo_dock_apply_texture_at_size (myData.iOldTexture, myData.iOldTextWidth, myData.iOldTextHeight);
		glPopMatrix ();
	}
	
	// image courante a 90deg.
	glRotatef (45. + fTheta, 0., 1., 0.);  // 0 -> 90
	glTranslatef (- (myData.iOldTextWidth ? myData.iOldTextWidth : iWidth)/2, 0., 0.);
	
	glRotatef (-90., 0., 1., 0.);
	cairo_dock_apply_texture_at_size (myData.iCurrentTexture, myData.iCurrentTextWidth, myData.iCurrentTextHeight);
	
	glDisable (GL_DEPTH_TEST);
	_cairo_dock_disable_texture ();
	return TRUE;
}

gboolean cd_xkbd_render_step_cairo (CairoDockModuleInstance *myApplet)
{
	double f;
	if (cairo_dock_has_transition (myIcon))
		f = cairo_dock_get_transition_fraction (myIcon);
	else
		f = 1.;
	
	g_print ("%s (%.2f)\n", __func__, f);
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	
	if (myData.pBackgroundSurface != NULL)
	{
		cairo_set_source_surface (
			myDrawContext,
			myData.pBackgroundSurface,
			0.,
			0.);
		cairo_paint (myDrawContext);
		cairo_dock_set_icon_surface_full (myDrawContext, myData.pBackgroundSurface, 1., 1., myIcon, myContainer);
	}
	
	if (myData.pOldSurface != NULL && 1-f > .01)
	{
		cairo_set_source_surface (
			myDrawContext,
			myData.pOldSurface,
			(iWidth - myData.iOldTextWidth)/2,
			(iHeight - myData.iOldTextHeight)/2);
		cairo_paint_with_alpha (myDrawContext, 1-f);
	}
	if (myData.pCurrentSurface != NULL)
	{
		cairo_set_source_surface (
			myDrawContext,
			myData.pCurrentSurface,
			(iWidth - myData.iCurrentTextWidth)/2,
			(iHeight - myData.iCurrentTextHeight)/2);
		cairo_paint_with_alpha (myDrawContext, f);
	}
	
	return TRUE;
}

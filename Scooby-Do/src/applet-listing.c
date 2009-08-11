/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>

#define __USE_BSD 1
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#include "applet-struct.h"
#include "applet-listing.h"

#define NB_STEPS_FOR_1_ENTRY 6
#define NB_STEPS_LATE 2
#define _listing_compute_nb_steps(pListing) (NB_STEPS_FOR_1_ENTRY + NB_STEPS_LATE * (MIN (myConfig.iNbLinesInListing, pListing->iNbEntries) - 1))
#define _listing_compute_width(pListing) (.4 * g_iScreenWidth[CAIRO_DOCK_HORIZONTAL])
#define _listing_compute_height(pListing) ((myDialogs.dialogTextDescription.iSize + 2) * (myConfig.iNbLinesInListing + 5) + 2*GAP)
#define NB_STEPS_FOR_CURRENT_ENTRY 12
#define NB_STEPS_FOR_SCROLL 8
#define GAP 3

  /////////////////////////////////////////
 /// Definition du container CDListing ///
/////////////////////////////////////////

void cd_do_free_entry (CDEntry *pEntry)
{
	if (pEntry == NULL)
		return ;
	
	g_free (pEntry->cPath);
	g_free (pEntry->cName);
	g_free (pEntry->cCaseDownName);
	g_free (pEntry->cIconName);
	cairo_surface_destroy (pEntry->pIconSurface);
}

void cd_do_backup_entry (CDEntry *pInEntry, CDEntry *pFromEntry)
{
	pInEntry->cPath = g_strdup (pFromEntry->cPath);
	pInEntry->cName = g_strdup (pFromEntry->cName);
	pInEntry->cIconName = NULL;  // on en n'a pas besoin pour le backup.
	pInEntry->pIconSurface = NULL;  // idem.
}

void cd_do_free_listing_backup (CDListingBackup *pBackup)
{
	if (pBackup == NULL)
		return ;
	
	gint i;
	CDEntry *pEntry;
	for (i = 0; i < pBackup->iNbEntries; i ++)
	{
		pEntry = &pBackup->pEntries[i];
		cd_do_free_entry (pEntry);
	}
	g_free (pBackup->pEntries);
	g_free (pBackup);
}

static gboolean on_expose_listing (GtkWidget *pWidget, GdkEventExpose *pExpose, CDListing *pListing)
{
	if (g_bUseOpenGL && pListing->container.glContext)
	{
		GdkGLContext *pGlContext = gtk_widget_get_gl_context (pWidget);
		GdkGLDrawable *pGlDrawable = gtk_widget_get_gl_drawable (pWidget);
		if (!gdk_gl_drawable_gl_begin (pGlDrawable, pGlContext))
			return FALSE;
		
		if (pExpose->area.x + pExpose->area.y != 0)
		{
			glEnable (GL_SCISSOR_TEST);  // ou comment diviser par 4 l'occupation CPU !
			glScissor ((int) pExpose->area.x,
				(int) (pListing->container.bIsHorizontal ? pListing->container.iHeight : pListing->container.iWidth) -
					pExpose->area.y - pExpose->area.height,  // lower left corner of the scissor box.
				(int) pExpose->area.width,
				(int) pExpose->area.height);
		}
		
		cairo_dock_notify_on_container (CAIRO_CONTAINER (pListing), CAIRO_DOCK_RENDER_DEFAULT_CONTAINER, pListing, NULL);
		
		glDisable (GL_SCISSOR_TEST);
		
		if (gdk_gl_drawable_is_double_buffered (pGlDrawable))
			gdk_gl_drawable_swap_buffers (pGlDrawable);
		else
			glFlush ();
		gdk_gl_drawable_gl_end (pGlDrawable);
	}
	else
	{
		cairo_t *pCairoContext = cairo_dock_create_drawing_context (CAIRO_CONTAINER (pListing));
		
		cairo_dock_notify_on_container (CAIRO_CONTAINER (pListing), CAIRO_DOCK_RENDER_DEFAULT_CONTAINER, pListing, pCairoContext);
		
		cairo_destroy (pCairoContext);
	}
	return FALSE;
}
static gboolean on_configure_listing (GtkWidget* pWidget, GdkEventConfigure* pEvent, CDListing *pListing)
{
	gint iNewWidth, iNewHeight;
	if (pListing->container.bIsHorizontal)
	{
		pListing->container.iWindowPositionX = pEvent->x;
		pListing->container.iWindowPositionY = pEvent->y;
		iNewWidth = pEvent->width;
		iNewHeight = pEvent->height;
	}
	else
	{
		pListing->container.iWindowPositionX = pEvent->y;
		pListing->container.iWindowPositionY = pEvent->x;
		iNewWidth = pEvent->height;
		iNewHeight = pEvent->width;
	}
	
	if (pListing->container.iWidth != iNewWidth || pListing->container.iHeight != iNewHeight)
	{
		pListing->container.iWidth = iNewWidth;
		pListing->container.iHeight = iNewHeight;
		
		if (g_bUseOpenGL && pListing->container.glContext)
		{
			GdkGLContext* pGlContext = gtk_widget_get_gl_context (pWidget);
			GdkGLDrawable* pGlDrawable = gtk_widget_get_gl_drawable (pWidget);
			GLsizei w = pEvent->width;
			GLsizei h = pEvent->height;
			if (!gdk_gl_drawable_gl_begin (pGlDrawable, pGlContext))
				return FALSE;
			
			glViewport(0, 0, w, h);
			
			cairo_dock_set_ortho_view (w, h);
			
			gdk_gl_drawable_gl_end (pGlDrawable);
		}
	}
	return FALSE;
}

static gboolean on_key_press_listing (GtkWidget *pWidget, GdkEventKey *pKey, CDListing *pListing)
{
	if (pKey->type == GDK_KEY_PRESS)
	{
		cairo_dock_notify_on_container (CAIRO_CONTAINER (pListing), CAIRO_DOCK_KEY_PRESSED, pListing, pKey->keyval, pKey->state, pKey->string);
	}
	return FALSE;
}
/*static gboolean on_motion_notify_listing (GtkWidget* pWidget, GdkEventMotion* pMotion, CDListing *pListing)
{
	pListing->container.iMouseX = pMotion->x;
	pListing->container.iMouseY = pMotion->y;
	
	gboolean bStartAnimation = FALSE;
	cairo_dock_notify_on_container (pListing, CAIRO_DOCK_MOUSE_MOVED, pListing, &bStartAnimation);
	if (bStartAnimation)
		cairo_dock_launch_animation (CAIRO_CONTAINER (pListing));
	
	gdk_device_get_state (pMotion->device, pMotion->window, NULL, NULL);  // pour recevoir d'autres MotionNotify.
	return FALSE;
}*/
static inline void _place_listing (CDListing *pListing)
{
	int iX, iY;
	if (g_pMainDock->bHorizontalDock)
	{
		iX = g_pMainDock->iWindowPositionX + g_pMainDock->iCurrentWidth/2 - pListing->container.iWidth/2;
		iY = g_pMainDock->iWindowPositionY + (g_pMainDock->bDirectionUp ? - pListing->container.iHeight : g_pMainDock->iCurrentHeight);
	}
	else
	{
		iX = g_pMainDock->iWindowPositionY + (g_pMainDock->bDirectionUp ? - pListing->container.iWidth : g_pMainDock->iCurrentHeight);
		iY = g_pMainDock->iWindowPositionX + g_pMainDock->iCurrentWidth/2 - pListing->container.iHeight/2;
	}
	g_print ("(%d;%d) %dx%d\n", iX, iY, pListing->container.iWidth, pListing->container.iHeight);
	gtk_window_move (GTK_WINDOW (pListing->container.pWidget), iX, iY);
}
CDListing *cd_do_create_listing (void)
{
	CDListing *pListing = g_new0 (CDListing, 1);
	
	pListing->container.iType = CAIRO_DOCK_NB_CONTAINER_TYPES+1;
	pListing->container.bIsHorizontal = TRUE;
	pListing->container.bDirectionUp = TRUE;
	pListing->container.fRatio = 1.;
	
	GtkWidget *pWindow = cairo_dock_create_container_window_no_opengl ();
	gtk_window_set_title (GTK_WINDOW (pWindow), "cairo-dock-listing");
	//gtk_widget_add_events (pWindow, GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);
	g_signal_connect (G_OBJECT (pWindow),
		"expose-event",
		G_CALLBACK (on_expose_listing),
		pListing);
	g_signal_connect (G_OBJECT (pWindow),
		"configure-event",
		G_CALLBACK (on_configure_listing),
		pListing);
	g_signal_connect (G_OBJECT (pWindow),
		"key-press-event",
		G_CALLBACK (on_key_press_listing),
		pListing);
	/*g_signal_connect (G_OBJECT (pWindow),
		"motion-notify-event",
		G_CALLBACK (on_motion_notify_listing),
		pListing);
	g_signal_connect (G_OBJECT (pWindow),
		"button-press-event",
		G_CALLBACK (on_button_press_listing),
		pListing);
	g_signal_connect (G_OBJECT (pWindow),
		"scroll-event",
		G_CALLBACK (on_scroll_listing),
		pListing);*/
	pListing->container.pWidget = pWindow;
	
	gtk_widget_show_all (pWindow);
	gtk_window_stick (GTK_WINDOW (pWindow));
	gtk_window_set_keep_above (GTK_WINDOW (pWindow), TRUE);
	
	pListing->container.iWidth = _listing_compute_width (pListing);
	pListing->container.iHeight = _listing_compute_height (pListing);
	gtk_window_resize (GTK_WINDOW (pWindow),
		pListing->container.iWidth,
		pListing->container.iHeight);
	
	_place_listing (pListing);
	
	return pListing;
}

void cd_do_destroy_listing (CDListing *pListing)
{
	if (pListing == NULL)
		return;
	
	if (pListing->iSidFillEntries != 0)
		g_source_remove (pListing->iSidFillEntries);
	
	if (pListing->container.iSidGLAnimation != 0)
		g_source_remove (pListing->container.iSidGLAnimation);
	gtk_widget_destroy (pListing->container.pWidget);
	
	gint i;
	CDEntry *pEntry;
	for (i = 0; i < pListing->iNbEntries; i ++)
	{
		pEntry = &pListing->pEntries[i];
		cd_do_free_entry (pEntry);
	}
	g_free (pListing->pEntries);
	
	g_free (pListing);
}

  ////////////////////////
 /// Fonctions utiles ///
////////////////////////

gboolean cd_do_update_listing_notification (gpointer pUserData, CDListing *pListing, gboolean *bContinueAnimation)
{
	//g_print ("%s ()\n", __func__);
	if (pListing->iAppearanceAnimationCount > 0)
	{
		pListing->iAppearanceAnimationCount --;
		if (pListing->iAppearanceAnimationCount != 0)
			*bContinueAnimation = TRUE;
	}
	if (pListing->iCurrentEntryAnimationCount > 0)
	{
		pListing->iCurrentEntryAnimationCount --;
		if (pListing->iCurrentEntryAnimationCount != 0)
			*bContinueAnimation = TRUE;
		/// optimisation : ne retracer que la zone concernee...
		
	}
	if (pListing->iScrollAnimationCount > 0)
	{
		pListing->iScrollAnimationCount --;
		if (pListing->iScrollAnimationCount != 0)
			*bContinueAnimation = TRUE;
		double f = (double) pListing->iScrollAnimationCount / NB_STEPS_FOR_SCROLL;
		double fAimedOffset = pListing->iCurrentEntry * (myDialogs.dialogTextDescription.iSize + 2);
		fAimedOffset = pListing->fAimedOffset;
		pListing->fCurrentOffset = pListing->fPreviousOffset * f + fAimedOffset * (1 - f);
	}
	double fRadius = MIN (6, myDialogs.dialogTextDescription.iSize/2+1);
	if (myData.pListing->iTitleWidth > myData.pListing->container.iWidth - 2*fRadius + 10)  // 10 pixels de rab
	{
		myData.pListing->iTitleOffset += 2 * myData.pListing->sens;
		if (myData.pListing->container.iWidth - 2*fRadius + myData.pListing->iTitleOffset > myData.pListing->iTitleWidth)
		{
			myData.pListing->iTitleOffset = myData.pListing->iTitleWidth - (myData.pListing->container.iWidth - 2*fRadius);
			myData.pListing->sens = -1;
		}
		else if (myData.pListing->iTitleOffset < 0)
		{
			myData.pListing->iTitleOffset = 0;
			myData.pListing->sens = 1;
		}
		*bContinueAnimation = TRUE;
		/// optimisation : ne retracer que la zone concernee...
		
	}
	cairo_dock_redraw_container (CAIRO_CONTAINER (pListing));
}

gboolean cd_do_render_listing_notification (gpointer pUserData, CDListing *pListing, cairo_t *pCairoContext)
{
	//g_print ("%s ()\n", __func__);
	int iWidth = pListing->container.iWidth, iHeight = pListing->container.iHeight;
	int iLeftMargin = myDialogs.dialogTextDescription.iSize + 2, iRightMargin = (myDialogs.dialogTextDescription.iSize + 2) / 2;
	int iTopMargin = (myDialogs.dialogTextDescription.iSize + 2) + GAP, iBottomMargin = (myDialogs.dialogTextDescription.iSize + 2) * 4 + GAP;
	CDEntry *pEntry;
	
	// on dessine un cadre et un fond
	double fRadius = MIN (6, myDialogs.dialogTextDescription.iSize/2+1);
	double fLineWidth = 1.;
	cairo_set_line_width (pCairoContext, fLineWidth);
	
	cairo_save (pCairoContext);
	cairo_translate (pCairoContext, 0, fLineWidth);
	cairo_dock_draw_rounded_rectangle (pCairoContext, fRadius, fLineWidth, iWidth - 2 * fRadius - fLineWidth, iTopMargin - GAP);
	cairo_set_source_rgba (pCairoContext, .8, .8, 1., 1.);
	cairo_stroke_preserve (pCairoContext);
	cairo_set_source_rgba (pCairoContext, 1., 1., 1., .7);
	cairo_fill (pCairoContext);

	cairo_translate (pCairoContext, 0, iTopMargin + fLineWidth);
	cairo_dock_draw_rounded_rectangle (pCairoContext, fRadius, fLineWidth, iWidth - 2 * fRadius - fLineWidth, iHeight - iTopMargin - iBottomMargin - GAP);
	cairo_set_source_rgba (pCairoContext, .8, .8, 1., 1.);
	cairo_stroke_preserve (pCairoContext);
	cairo_set_source_rgba (pCairoContext, 1., 1., 1., .7);
	cairo_fill (pCairoContext);
	
	cairo_translate (pCairoContext, 0, iHeight - iTopMargin - 2*fLineWidth - iBottomMargin + GAP);
	cairo_dock_draw_rounded_rectangle (pCairoContext, fRadius, fLineWidth, iWidth - 2 * fRadius - fLineWidth, iBottomMargin - GAP - fLineWidth);
	cairo_set_source_rgba (pCairoContext, .8, .8, 1., 1.);
	cairo_stroke_preserve (pCairoContext);
	cairo_set_source_rgba (pCairoContext, 1., 1., 1., .7);
	cairo_fill (pCairoContext);
	cairo_restore (pCairoContext);
	
	PangoLayout *pLayout = pango_cairo_create_layout (pCairoContext);
	PangoFontDescription *pDesc = pango_font_description_new ();
	
	pango_font_description_set_absolute_size (pDesc, myDialogs.dialogTextDescription.iSize * PANGO_SCALE);
	pango_font_description_set_family_static (pDesc, myDialogs.dialogTextDescription.cFont);
	pango_font_description_set_weight (pDesc, myDialogs.dialogTextDescription.iWeight);
	pango_font_description_set_style (pDesc, myLabels.iconTextDescription.iStyle);
	pango_layout_set_font_description (pLayout, pDesc);
	pango_font_description_free (pDesc);
	
	// on dessine les entrees.
	if (pListing->pEntries != NULL)
	{
		// on dessine chaque entree.
		int iNbSteps = _listing_compute_nb_steps (pListing);  // nb d'etapes pour l'apparition du texte.
		int iOffsetX = NB_STEPS_FOR_1_ENTRY - (iNbSteps - pListing->iAppearanceAnimationCount) - 1;
		if (pListing->iNbEntries >= myConfig.iNbLinesInListing)
			iOffsetX += myConfig.iNbLinesInListing/4*NB_STEPS_LATE;  // permet de donner une transparence aux 25% dernieres lignes.
		double dx, dy, dm = myConfig.iNbLinesInListing * (myDialogs.dialogTextDescription.iSize + 2) / 2;
		dm = 0;
		dy = iTopMargin - pListing->fCurrentOffset + 1 + dm;
		int i;
		double ymax = MIN (iTopMargin + pListing->iNbEntries * (myDialogs.dialogTextDescription.iSize + 2), iHeight - iBottomMargin);
		for (i = 0; i < pListing->iNbEntries; i ++)
		{
			if (iOffsetX >= NB_STEPS_FOR_1_ENTRY)  // en dehors a droite a partir de celui-ci.
				break ;
			pEntry = &pListing->pEntries[i];
			if (pEntry->bHidden)
				continue ;
			
			dx = myDialogs.dialogTextDescription.iSize + 2;  // marge a gauche.
			//if (iOffsetX > 0 && pListing->iAppearanceAnimationCount > 0)
			//	dx += (double) iOffsetX * (iWidth - (myDialogs.dialogTextDescription.iSize + 2)) / NB_STEPS_FOR_1_ENTRY;
			dy += (myDialogs.dialogTextDescription.iSize + 2);
			while (dy + myDialogs.dialogTextDescription.iSize + 2 <= iTopMargin + 1)
				dy += pListing->iNbEntries * (myDialogs.dialogTextDescription.iSize + 2);
			while (dy > ymax)
				dy -= pListing->iNbEntries * (myDialogs.dialogTextDescription.iSize + 2);
			if (dy > ymax || dy + myDialogs.dialogTextDescription.iSize + 2 <= iTopMargin + 1)
				continue;
			cairo_save (pCairoContext);
			cairo_translate (pCairoContext, dx, dy);
			
			// on fait un clip si necessaire.
			if (dy + myDialogs.dialogTextDescription.iSize + 2 > iHeight - iBottomMargin || dy < iTopMargin)  // cette entree n'est que partiellement visible.
			{
				if (dy < iTopMargin)  // elle depasse en haut.
					cairo_rectangle (pCairoContext, -iLeftMargin, iTopMargin - dy, iWidth, myDialogs.dialogTextDescription.iSize + 2 -(iTopMargin - dy));
				else  // elle depasse en bas.
					cairo_rectangle (pCairoContext, -iLeftMargin, 0, iWidth, iHeight - iBottomMargin - dy);
				cairo_clip (pCairoContext);
			}
			
			// on dessine l'icone.
			if (pEntry->pIconSurface != NULL)
			{
				cairo_set_source_surface (pCairoContext, pEntry->pIconSurface, - iLeftMargin + 1, 0.);
				cairo_paint (pCairoContext);
			}
			
			// on souligne l'entree courante.
			if (i == pListing->iCurrentEntry)
			{
				double e = 1. - (double) pListing->iCurrentEntryAnimationCount / NB_STEPS_FOR_CURRENT_ENTRY;
				if (e != 0)
				{
					cairo_save (pCairoContext);
					double rx = .5*(iWidth - iLeftMargin - iRightMargin);
					double ry = .5*(myDialogs.dialogTextDescription.iSize + 2);
					cairo_pattern_t *pPattern = cairo_pattern_create_radial (ry,
						ry,
						0.,
						ry,
						ry,
						e * ry);
					cairo_pattern_set_extend (pPattern, CAIRO_EXTEND_NONE);
					
					cairo_pattern_add_color_stop_rgba (pPattern,
						0.,
						0., 0., 1., .3);
					cairo_pattern_add_color_stop_rgba (pPattern,
						1.,
						0., 0., 0., 0.);
					cairo_scale (pCairoContext, rx/ry, 1.);
					cairo_set_source (pCairoContext, pPattern);
					cairo_paint (pCairoContext);
					cairo_pattern_destroy (pPattern);
					cairo_restore (pCairoContext);
				}
				
				// on dessine l'indicateur de sous-listing.
				if (pEntry->list != NULL)
				{
					cairo_set_source_rgba (pCairoContext, 0., 0., 0., e);
					cairo_move_to (pCairoContext, iWidth - iLeftMargin - iRightMargin, myDialogs.dialogTextDescription.iSize/4);
					cairo_rel_line_to (pCairoContext, iRightMargin, myDialogs.dialogTextDescription.iSize/4);
					cairo_rel_line_to (pCairoContext, -iRightMargin, myDialogs.dialogTextDescription.iSize/4);
					cairo_close_path (pCairoContext);
					cairo_stroke (pCairoContext);
				}
			}
			
			// on dessine le texte.
			cairo_set_source_rgba (pCairoContext, 0., 0., 0., 1. - (double) iOffsetX / NB_STEPS_FOR_1_ENTRY);
			pango_layout_set_text (pLayout, pEntry->cName, -1);
			pango_cairo_show_layout (pCairoContext, pLayout);
			
			// on separe la 1ere entree de la derniere.
			if (i == 0)
			{
				cairo_set_source_rgba (pCairoContext, 0., 0., 0., .5);
				cairo_move_to (pCairoContext, 0., 1.);
				cairo_rel_line_to (pCairoContext, iWidth - iLeftMargin - iRightMargin, 0.);
				double dashes = 2.;
				cairo_set_dash (pCairoContext, &dashes, 1, 0.);
				cairo_stroke (pCairoContext);
				cairo_set_dash (pCairoContext, &dashes, 0, 0.);
			}
			
			cairo_restore (pCairoContext);
			iOffsetX += NB_STEPS_LATE;
		}
		
		// on dessine le chemin de l'entree courante.
		pEntry = &pListing->pEntries[pListing->iCurrentEntry];
		cairo_save (pCairoContext);
		cairo_set_source_rgb (pCairoContext, 0., 0., 0.);
		cairo_translate (pCairoContext, fRadius - pListing->iTitleOffset, 0.);
		pango_layout_set_text (pLayout, pEntry->cPath ? pEntry->cPath : pEntry->cName, -1);
		PangoRectangle ink, log;
		pango_layout_get_pixel_extents (pLayout, &ink, &log);
		pListing->iTitleWidth = ink.width;
		pango_cairo_show_layout (pCairoContext, pLayout);
		cairo_restore (pCairoContext);
	}
	
	// on dessine l'etat de la recherche.
	cairo_translate (pCairoContext, 0, iHeight - iBottomMargin);
	cairo_set_source_surface (pCairoContext, myData.pScoobySurface, 0., 0.);
	cairo_paint (pCairoContext);
	
	cairo_set_source_rgb (pCairoContext, 0., 0., 0.);
	cairo_translate (pCairoContext, 2 * (myDialogs.dialogTextDescription.iSize + 2), GAP);
	if (myData.cStatus != NULL)
	{
		pango_layout_set_text (pLayout, myData.cStatus, -1);
	}
	pango_cairo_show_layout (pCairoContext, pLayout);
	
	// on dessine le filtre.
	/// essayer de dessiner un bouton enfonce ou une encoche (on pourrait mettre le texte en cache dans une surface).
	cairo_translate (pCairoContext, 0., myDialogs.dialogTextDescription.iSize + 2);
	cairo_set_source_surface (pCairoContext, (myData.iCurrentFilter & DO_MATCH_CASE) ? myData.pActiveButtonSurface : myData.pInactiveButtonSurface, 0., 0.);
	cairo_paint (pCairoContext);
	cairo_set_source_rgb (pCairoContext, 0., 0., 0.);
	pango_layout_set_text (pLayout, D_("(F1) Match case"), -1);
	pango_cairo_show_layout (pCairoContext, pLayout);
	
	cairo_translate (pCairoContext, iWidth/3, 0.);
	cairo_set_source_surface (pCairoContext, (myData.iCurrentFilter & DO_TYPE_MUSIC) ? myData.pActiveButtonSurface : myData.pInactiveButtonSurface, 0., 0.);
	cairo_paint (pCairoContext);
	cairo_set_source_rgb (pCairoContext, 0., 0., 0.);
	pango_layout_set_text (pLayout, D_("(F2) Music"), -1);
	pango_cairo_show_layout (pCairoContext, pLayout);
	
	cairo_translate (pCairoContext, iWidth/3, 0.);
	cairo_set_source_surface (pCairoContext, (myData.iCurrentFilter & DO_TYPE_IMAGE) ? myData.pActiveButtonSurface : myData.pInactiveButtonSurface, 0., 0.);
	cairo_paint (pCairoContext);
	cairo_set_source_rgb (pCairoContext, 0., 0., 0.);
	pango_layout_set_text (pLayout, D_("(F3) Image"), -1);
	pango_cairo_show_layout (pCairoContext, pLayout);
	
	cairo_translate (pCairoContext, -2*iWidth/3, myDialogs.dialogTextDescription.iSize + 2);
	cairo_set_source_surface (pCairoContext, (myData.iCurrentFilter & DO_TYPE_VIDEO) ? myData.pActiveButtonSurface : myData.pInactiveButtonSurface, 0., 0.);
	cairo_paint (pCairoContext);
	cairo_set_source_rgb (pCairoContext, 0., 0., 0.);
	pango_layout_set_text (pLayout, D_("(F4) Video"), -1);
	pango_cairo_show_layout (pCairoContext, pLayout);
	
	cairo_translate (pCairoContext, iWidth/3, 0.);
	cairo_set_source_surface (pCairoContext, (myData.iCurrentFilter & DO_TYPE_TEXT) ? myData.pActiveButtonSurface : myData.pInactiveButtonSurface, 0., 0.);
	cairo_paint (pCairoContext);
	cairo_set_source_rgb (pCairoContext, 0., 0., 0.);
	pango_layout_set_text (pLayout, D_("(F5) Text"), -1);
	pango_cairo_show_layout (pCairoContext, pLayout);
	
	cairo_translate (pCairoContext, iWidth/3, 0.);
	cairo_set_source_surface (pCairoContext, (myData.iCurrentFilter & DO_TYPE_HTML) ? myData.pActiveButtonSurface : myData.pInactiveButtonSurface, 0., 0.);
	cairo_paint (pCairoContext);
	cairo_set_source_rgb (pCairoContext, 0., 0., 0.);
	pango_layout_set_text (pLayout, D_("(F6) Html"), -1);
	pango_cairo_show_layout (pCairoContext, pLayout);
	
	cairo_translate (pCairoContext, -2*iWidth/3, myDialogs.dialogTextDescription.iSize + 2);
	cairo_set_source_surface (pCairoContext, (myData.iCurrentFilter & DO_TYPE_SOURCE) ? myData.pActiveButtonSurface : myData.pInactiveButtonSurface, 0., 0.);
	cairo_paint (pCairoContext);
	cairo_set_source_rgb (pCairoContext, 0., 0., 0.);
	pango_layout_set_text (pLayout, D_("(F7) Sources"), -1);
	pango_cairo_show_layout (pCairoContext, pLayout);
	
	g_object_unref (pLayout);
}


static gboolean _fill_entry_icon_idle (CDListing *pListing)
{
	g_print ("%s (%d)", __func__, pListing->iEntryToFill);
	
	CDEntry *pEntry;
	gboolean bHasBeenFilled = FALSE;
	while (pListing->iEntryToFill < pListing->iNbEntries && ! bHasBeenFilled)
	{
		pEntry = &pListing->pEntries[pListing->iEntryToFill];
		if (! pEntry->bHidden && pEntry->fill)
			bHasBeenFilled = pEntry->fill (pEntry);
		pListing->iEntryToFill ++;
	}
	
	if (pListing->iEntryToFill >= pListing->iNbEntries)
	{
		cairo_dock_redraw_container (CAIRO_CONTAINER (myData.pListing));
		pListing->iSidFillEntries = 0;
		return FALSE;
	}
	return TRUE;
}
void cd_do_show_listing (CDEntry *pEntries, int iNbEntries)
{
	if (myData.pListing == NULL)
	{
		myData.pListing = cd_do_create_listing ();
		
		cairo_dock_register_notification_on_container (CAIRO_CONTAINER (myData.pListing),
			CAIRO_DOCK_UPDATE_DEFAULT_CONTAINER,
			(CairoDockNotificationFunc) cd_do_update_listing_notification,
			CAIRO_DOCK_RUN_AFTER,
			NULL);
		cairo_dock_register_notification_on_container (CAIRO_CONTAINER (myData.pListing),
			CAIRO_DOCK_RENDER_DEFAULT_CONTAINER,
			(CairoDockNotificationFunc) cd_do_render_listing_notification,
			CAIRO_DOCK_RUN_AFTER,
			NULL);
		if (myData.pScoobySurface == NULL)
		{
			cairo_t* pSourceContext = cairo_dock_create_context_from_container (CAIRO_CONTAINER (g_pMainDock));
			myData.pScoobySurface = cairo_dock_create_surface_from_image_simple (MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
				pSourceContext,
				2 * (myDialogs.dialogTextDescription.iSize + 2),
				2 * (myDialogs.dialogTextDescription.iSize + 2));
			cairo_destroy (pSourceContext);
		}
		if (myData.pActiveButtonSurface == NULL)
		{
			g_print ("load button : %dx%d\n", myDialogs.dialogTextDescription.iSize + 2, myData.pListing->container.iWidth);
			cairo_t* pSourceContext = cairo_dock_create_context_from_container (CAIRO_CONTAINER (g_pMainDock));
			myData.pActiveButtonSurface = cairo_dock_create_surface_from_image_simple (MY_APPLET_SHARE_DATA_DIR"/active-button.svg",
				pSourceContext,
				(myData.pListing->container.iWidth - (myDialogs.dialogTextDescription.iSize + 2) * 3) / 3,
				myDialogs.dialogTextDescription.iSize + 2);
			myData.pInactiveButtonSurface = cairo_dock_create_surface_from_image_simple (MY_APPLET_SHARE_DATA_DIR"/inactive-button.svg",
				pSourceContext,
				(myData.pListing->container.iWidth - (myDialogs.dialogTextDescription.iSize + 2) * 3) / 3,
				myDialogs.dialogTextDescription.iSize + 2);
			cairo_destroy (pSourceContext);
		}
	}
	else
	{
		gtk_widget_show (myData.pListing->container.pWidget);
		
		int iWidth = _listing_compute_width (pListing);
		int iHeight = _listing_compute_height (pListing);
		if (myData.pListing->container.iWidth != iWidth || myData.pListing->container.iHeight != iHeight)
		{
			gtk_window_resize (GTK_WINDOW (myData.pListing->container.pWidget),
				iWidth,
				iHeight);
		}
		_place_listing (myData.pListing);
		cairo_dock_redraw_container (CAIRO_CONTAINER (myData.pListing));
	}
	
	if (myData.pListing->pEntries != NULL)
	{
		gint i;
		CDEntry *pEntry;
		g_print ("%d entrees precedemment\n", myData.pListing->iNbEntries);
		for (i = 0; i < myData.pListing->iNbEntries; i ++)
		{
			pEntry = &myData.pListing->pEntries[i];
			cd_do_free_entry (pEntry);
		}
		g_free (myData.pListing->pEntries);
	}
	myData.pListing->pEntries = pEntries;
	
	myData.pListing->iNbEntries = iNbEntries;
	myData.pListing->iNbVisibleEntries = iNbEntries;
	
	if (iNbEntries == 0)
		cd_do_set_status (D_("No result"));
	else if (iNbEntries >= myConfig.iNbResultMax)
		cd_do_set_status_printf ("> %d results", myConfig.iNbResultMax);
	else
		cd_do_set_status_printf ("%d %s", iNbEntries, iNbEntries > 1 ? D_("results") : D_("result"));
	
	myData.pListing->iCurrentEntry = MIN (myConfig.iNbLinesInListing, iNbEntries) / 2;
	myData.pListing->iEntryToFill = 0;
	myData.pListing->iCurrentEntryAnimationCount = NB_STEPS_FOR_CURRENT_ENTRY;  // pas de surlignage pendant l'apparition.
	
	myData.pListing->iScrollAnimationCount = 0;
	myData.pListing->fAimedOffset = 0;
	myData.pListing->fPreviousOffset = myData.pListing->fCurrentOffset = 0;
	myData.pListing->sens = 1;
	myData.pListing->iTitleOffset = 0;
	myData.pListing->iTitleWidth = 0;
	
	myData.pListing->iAppearanceAnimationCount = _listing_compute_nb_steps (myData.pListing);
	if (iNbEntries != 0)
		cairo_dock_launch_animation (CAIRO_CONTAINER (myData.pListing));
	
	cd_do_fill_listing_entries (myData.pListing);
}

void cd_do_hide_listing (void)
{
	if (myData.pListing == NULL)
		return;
	if (myData.pListing->iSidFillEntries != 0)
	{
		g_source_remove (myData.pListing->iSidFillEntries);
		myData.pListing->iSidFillEntries = 0;
	}
	myData.pListing->iEntryToFill = 0;
	
	gint i;
	CDEntry *pEntry;
	for (i = 0; i < myData.pListing->iNbEntries; i ++)
	{
		pEntry = &myData.pListing->pEntries[i];
		cd_do_free_entry (pEntry);
	}
	g_free (myData.pListing->pEntries);
	myData.pListing->pEntries = NULL;
	myData.pListing->iNbEntries = 0;
	myData.pListing->iCurrentEntry = 0;
	
	if (myData.pListingHistory != NULL)
	{
		g_list_foreach (myData.pListingHistory, (GFunc) cd_do_free_listing_backup, NULL);
		g_list_free (myData.pListingHistory);
		myData.pListingHistory = NULL;
	}
	
	myData.pListing->iAppearanceAnimationCount = 0;
	myData.pListing->iCurrentEntryAnimationCount = 0;
	myData.pListing->iScrollAnimationCount = 0;
	myData.pListing->fAimedOffset = 0;
	myData.pListing->fPreviousOffset = myData.pListing->fCurrentOffset = 0;
	myData.pListing->iTitleWidth = 0;
	myData.pListing->iTitleOffset = 0;
	myData.pListing->sens = 1;
	
	g_free (myData.cStatus);
	myData.cStatus = NULL;
	myData.bFoundNothing = FALSE;
	
	gtk_widget_hide (myData.pListing->container.pWidget);
}

void cd_do_fill_listing_entries (CDListing *pListing)
{
	pListing->iEntryToFill = 0;
	if (pListing->iSidFillEntries == 0 && pListing->iNbVisibleEntries != 0)
		pListing->iSidFillEntries = g_idle_add ((GSourceFunc)_fill_entry_icon_idle, pListing);
}


void cd_do_select_prev_next_entry_in_listing (gboolean bNext)
{
	myData.pListing->fPreviousOffset = myData.pListing->fCurrentOffset;
	if (bNext)
	{
		int i = myData.pListing->iCurrentEntry;
		do
		{
			i ++;
			if (i >= myData.pListing->iNbEntries)
				i = 0;
		} while (i != myData.pListing->iCurrentEntry && myData.pListing->pEntries[i].bHidden);
		myData.pListing->iCurrentEntry = i;
	}
	else
	{
		int i = myData.pListing->iCurrentEntry;
		do
		{
			i --;
			if (i < 0)
				i = myData.pListing->iNbEntries - 1;
		} while (i != myData.pListing->iCurrentEntry && myData.pListing->pEntries[i].bHidden);
		myData.pListing->iCurrentEntry = i;
	}
	myData.pListing->fAimedOffset += (bNext ? 1:-1) * (myDialogs.dialogTextDescription.iSize + 2);
	
	myData.pListing->iCurrentEntryAnimationCount = NB_STEPS_FOR_CURRENT_ENTRY;
	myData.pListing->iScrollAnimationCount = NB_STEPS_FOR_SCROLL;
	myData.pListing->iTitleOffset = 0;
	myData.pListing->sens = 1;
	cairo_dock_launch_animation (CAIRO_CONTAINER (myData.pListing));
	cairo_dock_redraw_container (CAIRO_CONTAINER (myData.pListing));
}

void cd_do_select_prev_next_page_in_listing (gboolean bNext)
{
	myData.pListing->fPreviousOffset = myData.pListing->fCurrentOffset;
	if (bNext)
	{
		int i = myData.pListing->iCurrentEntry, j = i, k = 0;
		do
		{
			i ++;
			if (i >= myData.pListing->iNbEntries)
				break;
			if (! myData.pListing->pEntries[i].bHidden)
			{
				j = i;
				k ++;
			}
		} while (k < myConfig.iNbLinesInListing);
		myData.pListing->iCurrentEntry = j;
	}
	else
	{
		int i = myData.pListing->iCurrentEntry, j = i, k = 0;
		do
		{
			i --;
			if (i < 0)
				break;
			if (! myData.pListing->pEntries[i].bHidden)
			{
				j = i;
				k ++;
			}
		} while (k < myConfig.iNbLinesInListing);
		myData.pListing->iCurrentEntry = j;
	}
	myData.pListing->fAimedOffset = myData.pListing->iCurrentEntry * (myDialogs.dialogTextDescription.iSize + 2);
	
	myData.pListing->iCurrentEntryAnimationCount = NB_STEPS_FOR_CURRENT_ENTRY;
	myData.pListing->iScrollAnimationCount = NB_STEPS_FOR_SCROLL;
	myData.pListing->iTitleOffset = 0;
	myData.pListing->sens = 1;
	cairo_dock_launch_animation (CAIRO_CONTAINER (myData.pListing));
	cairo_dock_redraw_container (CAIRO_CONTAINER (myData.pListing));
}

void cd_do_select_last_first_entry_in_listing (gboolean bLast)
{
	myData.pListing->fPreviousOffset = myData.pListing->fCurrentOffset;
	if (bLast)
	{
		int i = myData.pListing->iNbEntries - 1;
		while (i > 0 && myData.pListing->pEntries[i].bHidden)
			i --;
		myData.pListing->iCurrentEntry = i;
	}
	else
	{
		int i = 0;
		while (i < myData.pListing->iNbEntries - 1 && myData.pListing->pEntries[i].bHidden)
			i ++;
		myData.pListing->iCurrentEntry = i;
	}
	myData.pListing->fAimedOffset = myData.pListing->iCurrentEntry * (myDialogs.dialogTextDescription.iSize + 2);
	
	myData.pListing->iCurrentEntryAnimationCount = NB_STEPS_FOR_CURRENT_ENTRY;
	myData.pListing->iScrollAnimationCount = NB_STEPS_FOR_SCROLL;
	myData.pListing->iTitleOffset = 0;
	myData.pListing->sens = 1;
	cairo_dock_launch_animation (CAIRO_CONTAINER (myData.pListing));
	cairo_dock_redraw_container (CAIRO_CONTAINER (myData.pListing));
}

void cd_do_select_nth_entry_in_listing (int iNumEntry)
{
	myData.pListing->fPreviousOffset = myData.pListing->fCurrentOffset;
	
	myData.pListing->iCurrentEntry = MIN (iNumEntry, myData.pListing->iNbEntries - 1);
	
	myData.pListing->fAimedOffset = myData.pListing->iCurrentEntry * (myDialogs.dialogTextDescription.iSize + 2);
	
	myData.pListing->iCurrentEntryAnimationCount = NB_STEPS_FOR_CURRENT_ENTRY;
	myData.pListing->iScrollAnimationCount = NB_STEPS_FOR_SCROLL;
	myData.pListing->iTitleOffset = 0;
	myData.pListing->sens = 1;
	cairo_dock_launch_animation (CAIRO_CONTAINER (myData.pListing));
	cairo_dock_redraw_container (CAIRO_CONTAINER (myData.pListing));
}


void cd_do_set_status (const gchar *cStatus)
{
	g_free (myData.cStatus);
	myData.cStatus = g_strdup (cStatus);
	if (myData.pListing)
		cairo_dock_redraw_container (CAIRO_CONTAINER (myData.pListing));
}

void cd_do_set_status_printf (const gchar *cStatusFormat, ...)
{
	g_return_if_fail (cStatusFormat != NULL);
	va_list args;
	va_start (args, cStatusFormat);
	gchar *cStatus = g_strdup_vprintf (cStatusFormat, args);
	cd_do_set_status (cStatus);
	g_free (cStatus);
	va_end (args);
}

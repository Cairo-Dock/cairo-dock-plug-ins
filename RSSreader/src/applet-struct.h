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


#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	// comportement
	gchar *cUrl;
	gchar *cUrlLogin;
	gchar *cUrlPassword;
	gchar *cUserTitle;
	gint iRefreshTime;
	gint iNbLinesInDialog;
	gint iMaxLines;  // bof.
	gchar *cSpecificWebBrowser;
	gboolean bDialogIfFeedChanged;
	gchar *cAnimationIfFeedChanged;
	gint iDialogsDuration;
	// apparence du desklet
	gboolean bDisplayLogo;
	gchar *cLogoPath;
	gdouble fLogoSize;
	gboolean bDisplayBackground;
	double fBackgroundColor1[4];
	double fBackgroundColor2[4];
	gint iBackgroundRadius;
	double fBorderColor[4];
	gint iBorderThickness;
	// apparence du texte
	gint iSpaceBetweenFeedLines;
	double fTitleTextColor[4];
	gchar *cTitleFont;
	gdouble fTitleAlignment;
	double fTextColor[4];
	gchar *cFont;
	gint iTextMargin;
	} ;

typedef struct _CDRssItem {
	gchar *cTitle;
	gchar *cDescription;
	gchar *cLink;
	gchar *cImage;  // pas utilise pour l'instant.
	gchar *cAuthor;  // Atom seulement.
	} CDRssItem ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	CairoDockTask *pTask;  // tache pour recuperer le flux.
	// shared memory.
	gchar *cTaskBridge;
	// end of shared memory.
	gboolean bUpdateIsManual;  // TRUE si l'utilisateur a force le refresh.
	
	GList *pItemList;  // une liste de CDRssItem.
	gchar *PrevFirstTitle;  // 1er item du flux precedent (titre du flux = item 0).
	cairo_surface_t *pLogoSurface;  // surface du logo.
	gdouble fLogoSize;  // taille a laquelle le logo a ete charge.
	
	int iFirstDisplayedItem;  // pour le scroll.
	guint iSidRedrawIdle;
	} ;


#endif

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
	gchar *cSpecificWebBrowser;
	gboolean bInfoBubble;
	gboolean bLeftClicForDesklet;
	gboolean bDisplayLogo;
	gboolean bDisplayBackground;
	double fBackgroundColor1[4];
	double fBackgroundColor2[4];
	gint iBackgroundRadius;
	double fBorderColor[4];
	gint iBorderThickness;
	gint iDialogsDuration;
	gint iRefreshTime;	
	gchar *cUrl;
	
	gchar *cName;
	gint iLines;
	gint iTitleNum;
	
	gchar *cMessageNoUrl;
	gchar *cMessageNoTitle;
	gchar *cMessageFailedToConnect;
	
	gint iTitlePositionX;
	gint iTitlePositionY;
	gint iTextPositionX;
	gint iTextPositionY;
	gint iSpaceBetweenLines;
	
	double fTextColor[4];
	gchar *cFont;
	double fTitleTextColor[4];
	gchar *cTitleFont;
	gchar *cLogoPath;
	gdouble fLogoSize;
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	CairoDockTask *pTask;
	CairoDockTask *pTitleTask;
	CairoDockTask *pAutomaticRefreshTask;
	guint iSidAutomaticRefresh;
	gchar *cFeedLine[31]; // Nombre de flux MAX + L'ensemble en 0
	gchar *cLastFirstFeedLine;
	gchar *cLastSecondFeedLine;
	gchar *cTempText;
	gchar *cTaskBridge;
	gchar *cTitleTaskBridge;
	gchar *cDialogMessage;
		
	gchar *cFontSize;
	gboolean bFontIsBold;
	gboolean bFontIsItalic;
	gchar *cTitleFontSize;
	gboolean bTitleFontIsBold;
	gboolean bTitleFontIsItalic;
	gboolean bUpdateIsManual;
	
	cairo_surface_t *pLogoSurface;
	
	gdouble fLogoSize;
	gint iMyLogoIsOn;
	
	
	
	} ;


#endif

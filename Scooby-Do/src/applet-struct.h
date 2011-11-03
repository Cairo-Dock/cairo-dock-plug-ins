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
	gdouble fFontSizeRatio;  // 1 <=> taille du dock
	gboolean bTextOnTop;
	CairoDockLabelDescription labelDescription;
	gchar *cShortkeySearch;
	gchar *cIconAnimation;
	gdouble pFrameColor[4];
	gint iAnimationDuration;
	gint iAppearanceDuration;
	gint iCloseDuration;
	gint iNbResultMax;
	CairoDockLabelDescription infoDescription;
	gint iNbLinesInListing;
	gchar **cPreferredApplis;
	gboolean bUseFiles;
	gboolean bUseFirefox;
	gboolean bUseRecent;
	gboolean bUseWeb;
	gboolean bUseCommand;
	};

typedef struct _CDEntry CDEntry;
typedef struct _CDBackend CDBackend;
typedef struct _CDListing CDListing;
typedef struct _CDListingBackup CDListingBackup;
typedef struct _CDChar CDChar;

typedef gboolean (*CDFillEntryFunc) (CDEntry *pEntry);
typedef void (*CDExecuteEntryFunc) (CDEntry *pEntry);
typedef GList* (*CDListSubEntryFunc) (CDEntry *pEntry, int *iNbSubEntries);

struct _CDEntry {
	gchar *cPath;
	gchar *cName;
	gchar *cLowerCaseName;
	gchar *cIconName;
	cairo_surface_t *pIconSurface;
	gpointer data;
	gboolean bHidden;
	gboolean bMainEntry;
	CDBackend *pBackend;
	CDFillEntryFunc fill;
	CDExecuteEntryFunc execute;
	CDListSubEntryFunc list;
	} ;

struct _CDListing {
	CairoContainer container;
	GList *pEntries;
	gint iNbEntries;
	GList *pCurrentEntry;
	gint iAppearanceAnimationCount;
	gint iCurrentEntryAnimationCount;
	gint iScrollAnimationCount;
	gdouble fPreviousOffset;
	gdouble fCurrentOffset;
	gdouble fAimedOffset;
	gint iTitleOffset;
	gint iTitleWidth;
	gint sens;
	guint iSidFillEntries;
	GList *pEntryToFill;
	gint iNbVisibleEntries;
	} ;

struct _CDListingBackup {
	GList *pEntries;
	gint iNbEntries;
	GList *pCurrentEntry;
	} ;

struct _CDChar {
	gchar c;
	cairo_surface_t *pSurface;
	GLuint iTexture;
	gint iWidth, iHeight;
	gint iAnimationTime;
	gint iInitialX, iInitialY;
	gint iFinalX, iFinalY;
	gint iCurrentX, iCurrentY;
	gdouble fRotationAngle;
	} ;

typedef enum _CDFilter {
	DO_FILTER_NONE	=0,
	DO_MATCH_CASE	=1<<0,
	DO_TYPE_MUSIC	=1<<1,
	DO_TYPE_IMAGE	=1<<2,
	DO_TYPE_VIDEO	=1<<3,
	DO_TYPE_TEXT	=1<<4,
	DO_TYPE_HTML	=1<<5,
	DO_TYPE_SOURCE	=1<<6
	} CDFilter;

typedef gboolean (*CDBackendInitFunc) (void);
typedef GList * (*CDBackendSearchFunc) (const gchar *cText, gint iFilter, gboolean bSearchAll, int *iNbEntries);
typedef void (*CDBackendStopFunc) (void);

struct _CDBackend {
	// interface
	const gchar *cName;
	gboolean bIsThreaded;
	gboolean bStaticResults;
	CDBackendInitFunc init;
	CDBackendSearchFunc search;
	CDBackendStopFunc stop;
	// private data
	gboolean bIsActive;
	gint iState;  // 0:uninitialized; 1:ok; -1:broken
	CairoDockTask *pTask;
	gboolean bTooManyResults;
	gboolean bFoundNothing;
	GList *pLastShownResults;
	gint iNbLastShownResults;
	// shared memory
	gchar *cCurrentLocateText;  // lecture seule dans le thread
	gint iLocateFilter;  // lecture seule dans le thread
	GList *pSearchResults;  // ecriture seule dans le thread
	gint iNbSearchResults;  // ecriture seule dans le thread
	// end of shared memory
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculation, etc.
struct _AppletData {
	gint iSessionState;  // 0:no session, 1: session closing, 2: session running
	GString *sCurrentText;
	guint iNbValidCaracters;
	Window iPreviouslyActiveWindow;
	gint iTextWidth, iTextHeight;
	gint iCloseTime;
	GList *pCharList;
	gint iAppearanceTime;
	
	gint iPromptAnimationCount;
	cairo_surface_t *pPromptSurface;
	gint iPromptWidth, iPromptHeight;
	GLuint iPromptTexture;
	
	GList *pMatchingIcons;
	GList *pCurrentMatchingElement;
	gint iCurrentMatchingOffset;
	gint iPreviousMatchingOffset;
	gint iMatchingGlideCount;
	gint iMatchingAimPoint;
	
	GList *pApplications;
	GList *pMonitorList;
	GList *pCurrentApplicationToLoad;
	guint iSidLoadExternAppliIdle;
	
	gint iCurrentFilter;
	
	CDListing *pListing;
	gchar *cStatus;
	cairo_surface_t *pScoobySurface;
	cairo_surface_t *pActiveButtonSurface, *pInactiveButtonSurface;
	GList *pListingHistory;
	gchar *cSearchText;
	
	GList *pBackends;
	CairoKeyBinding *cKeyBinding;
	} ;


#endif

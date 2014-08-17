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

#ifndef __TOMBOY_STRUCT__
#define  __TOMBOY_STRUCT__

#include <cairo-dock.h>
#include <cairo-dock-applet-single-instance.h>

typedef enum {
	CD_NOTES_GNOTES=0,
	CD_NOTES_TOMBOY,
	CD_NOTES_DEFAULT,
	CD_NOTES_NB_BACKENDS
	} CDTomboyBackendEnum;

struct _AppletConfig {
	gchar *defaultTitle;
	gchar *cIconDefault;
	gchar *cIconClose;
	gchar *cIconBroken;
	gchar *cNoteIcon;
	gboolean bNoDeletedSignal;
	CDTomboyBackendEnum iAppControlled;
	gchar *cRenderer;
	gboolean bDrawContent;
	gboolean bPopupContent;
	gchar *cDateFormat;
	gboolean bAutoNaming;
	gboolean bAskBeforeDelete;
	gdouble fTextColor[3];
	gint iDialogDuration;
	} ;

typedef struct {
	gchar *cID;
	gchar *cTitle;
	gchar *cTags;
	gchar *cContent;
	guint iCreationDate;
	guint iLastChangeDate;
} CDNote;
	
typedef struct {
	void (*start) (void);
	void (*stop) (void);
	void (*show_note) (const gchar *cNoteID);
	void (*delete_note) (const gchar *cNoteID);
	gchar * (*get_note_content) (const gchar *cNoteID);
	gchar * (*create_note) (const gchar *cTitle);
	void (*run_manager) (void);
} CDNotesBackend;
	
struct _AppletData {
	cairo_surface_t *pSurfaceNote;
	gint iNoteWidth, iNoteHeight;
	gboolean bIsRunning;
	gint iIconState;  // 0:running 1:closed/broken.
	GHashTable *hNoteTable;
	guint iSidResetQuickInfo;
	guint iSidPopupDialog;
	DBusGProxyCall *pDetectTomboyCall;
	DBusGProxyCall *pGetNotesCall;
	GldiTask *pTask;
	CDNotesBackend backend;
	} ;

#endif

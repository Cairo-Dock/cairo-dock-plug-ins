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

#include <alsa/asoundlib.h>
#define _STRUCT_TIMEVAL
#include <cairo-dock.h>

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gboolean bPlayOnClick;
	gchar *cOnClickSound;
	gboolean bPlayOnMiddleClick;
	gchar *cOnMiddleClickSound;
	gboolean bPlayOnHover;
	gchar *cOnHoverSound;
	gdouble fVolume;
	} ;


typedef struct {
	gchar *buffer;  // file content
	gsize length;  // file length
	gchar *data;  // pointer to data (inside the buffer).
	gsize size;  // size of data.
	// snd params
	snd_pcm_format_t format;
	guint channels;
	guint rate;
	guint iNbFrames;
	guint iBitsPerSample;
	gdouble fTimelength;
	} CDSoundFile;

typedef struct {
	CDSoundFile *pSoundFile;
	snd_pcm_t *handle;
	CairoDockTask *pTask;
	} CDSharedMemory;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	CDSoundFile *pOnClickSound;
	CDSoundFile *pOnMiddleClickSound;
	CDSoundFile *pOnHoverSound;
	GList *pTasks;
	} ;


#endif

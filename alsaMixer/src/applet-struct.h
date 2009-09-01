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
#include <alsa/asoundlib.h>


typedef enum {
	VOLUME_NO_DISPLAY = 0,
	VOLUME_ON_LABEL,
	VOLUME_ON_ICON,
	VOLUME_NB_DISPLAYS
	} VolumeTypeDisplay;

typedef enum {
	VOLUME_NO_EFFECT = 0,
	VOLUME_EFFECT_ZOOM,
	VOLUME_EFFECT_TRANSPARENCY,
	VOLUME_EFFECT_BAR,
	VOLUME_NB_EFFECTS
	} VolumeTypeEffect;

struct _AppletConfig {
	gchar *card_id;
	gchar *cMixerElementName;
	gchar *cMixerElementName2;
	gchar *cShowAdvancedMixerCommand;
	VolumeTypeDisplay iVolumeDisplay;
	VolumeTypeEffect iVolumeEffect;
	gchar *cDefaultIcon;
	gchar *cBrokenIcon;
	gchar *cMuteIcon;
	gchar *cShortcut;
	gint iScrollVariation;
	gboolean bHideScaleOnLeave;
	} ;

struct _AppletData {
	snd_mixer_t *mixer_handle;
	gchar *mixer_card_name;
	gchar *mixer_device_name;
	gchar *cErrorMessage;
	snd_mixer_elem_t *pControledElement;
	snd_mixer_elem_t *pControledElement2;  // des fois un element ne controle qu'une sortie (droite ou gauche).
	snd_mixer_selem_id_t *pControledID;
	gboolean bHasMuteSwitch;
	long iVolumeMin, iVolumeMax;  // volumes min et max en unites de la carte son.
	guint iSidCheckVolume;
	CairoDialog *pDialog;
	cairo_surface_t *pSurface;
	//cairo_surface_t *pBrokenSurface;
	cairo_surface_t *pMuteSurface;
	int iCurrentVolume;  // volume courant en %.
	gboolean bIsMute;
	GtkWidget *pScale;
	} ;


#endif

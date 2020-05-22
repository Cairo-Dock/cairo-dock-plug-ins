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

#include <sys/types.h>
#include <alsa/asoundlib.h>
#include <cairo-dock.h>

#ifdef INDICATOR_SOUNDMENU_WITH_IND3
#include "indicator-applet3.h"
#elif defined SOUND_SERVICE_SUPPORT // OLD
#include "indicator-applet.h"
#include "mute-widget.h"
#endif

// Info display types
typedef enum {
	VOLUME_NO_DISPLAY = 0,
	VOLUME_ON_LABEL,
	VOLUME_ON_ICON,
	VOLUME_NB_DISPLAYS
	} VolumeTypeDisplay;

// Icon display types
typedef enum {
	VOLUME_EFFECT_NONE = 0,
	VOLUME_EFFECT_BAR,
	VOLUME_EFFECT_GAUGE,
	VOLUME_NB_EFFECTS
	} VolumeTypeEffect;

struct _AppletConfig {
	// alsa options
	gchar *card_id;
	gchar *cMixerElementName;
	gchar *cMixerElementName2;
	gchar *cShowAdvancedMixerCommand;
	// display
	VolumeTypeDisplay iVolumeDisplay;
	VolumeTypeEffect iVolumeEffect;
	gchar *cDefaultIcon;
	gchar *cBrokenIcon;
	gchar *cMuteIcon;
	gchar *cGThemePath;
	RendererRotateTheme iRotateTheme;
	// accesibility
	gchar *cShortcut;
	gint iScrollVariation;
	gboolean bHideScaleOnLeave;
	#ifdef INDICATOR_SOUNDMENU_WITH_IND3
	gchar *cIndicatorName;
	#endif
	} ;

// Interface of a Sound Controler
typedef struct {
	int (*get_volume) (void);
	void (*set_volume) (int iVolume);
	void (*toggle_mute) (void);
	void (*show_hide) (void);
	void (*stop) (void);
	void (*reload) (void);
	} CDSoundCtl;

struct _AppletData {
	// generic interface
	CDSoundCtl ctl;
	// alsa data
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
	int iCurrentVolume;  // current volume in [0-100]
	// sound service data
	#ifdef INDICATOR_SOUNDMENU_WITH_IND3
	IndicatorObject *pIndicator;
	IndicatorObjectEntry *pEntry;
	#elif defined SOUND_SERVICE_SUPPORT // OLD
	CDAppletIndicator *pIndicator;
	GtkWidget* volume_widget;
	GList *transport_widgets_list;
	GtkWidget* voip_widget;
	MuteWidget* mute_widget;
	gint iCurrentState;
	#endif
	// other
	gboolean bIsMute;
	gint bMuteImage;  // 1 if the "mute" image is currently displayed, 0 if the normal icon is set, -1 if no image is set
	GtkWidget *pScale;
	GldiShortkey *cKeyBinding;
	} ;


#endif

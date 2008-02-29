
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <glib.h>
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

typedef struct {
	gchar *card_id;
	gchar *cMixerElementName;
	gchar *cShowAdvancedMixerCommand;
	VolumeTypeDisplay iVolumeDisplay;
	VolumeTypeEffect iVolumeEffect;
	gchar *cDefaultLabel;
	gchar *cDefaultIcon;
	gchar *cBrokenIcon;
	gchar *cMuteIcon;
	} AppletConfig;

typedef struct {
	snd_mixer_t *mixer_handle;
	gchar *mixer_card_name;
	gchar *mixer_device_name;
	snd_mixer_elem_t *pControledElement;
	snd_mixer_selem_id_t *pControledID;
	gboolean bHasMuteSwitch;
	long iVolumeMin, iVolumeMax;
	guint iSidCheckVolume;
	CairoDockDialog *pDialog;
	cairo_surface_t *pSurface;
	cairo_surface_t *pBrokenSurface;
	cairo_surface_t *pMuteSurface;
	int iCurrentVolume;
	} AppletData;


#endif

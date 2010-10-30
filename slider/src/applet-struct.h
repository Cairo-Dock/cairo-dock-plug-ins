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

#include <glib/gstdio.h>

typedef enum {
	SLIDER_DEFAULT = 0,
	SLIDER_FADE,
	SLIDER_BLANK_FADE,
	SLIDER_FADE_IN_OUT,
	SLIDER_SIDE_KICK,
	SLIDER_DIAPORAMA,
	SLIDER_GROW_UP,
	SLIDER_SHRINK_DOWN,
	SLIDER_CUBE,
	SLIDER_RANDOM,
	SLIDER_NB_ANIMATION
} SliderAnimation;

typedef enum {
	SLIDER_PAUSE = 0,
	SLIDER_OPEN_IMAGE,
	SLIDER_OPEN_FOLDER,
	SLIDER_NB_CLICK_OPTION
} SliderClickOption;

typedef struct {
	double fImgX;
	double fImgY;
	double fImgW;
	double fImgH;
} SliderImageArea;

typedef enum {
	SLIDER_UNKNOWN_FORMAT = 0,
	SLIDER_PNG,
	SLIDER_JPG,
	SLIDER_SVG,
	SLIDER_GIF,
	SLIDER_XPM,
	SLIDER_NB_IMAGE_FORMAT
} SliderImageFormat;

typedef struct {
	gchar *cPath;
	gint iSize;
	SliderImageFormat iFormat;
	gint iOrientation;
	gboolean bGotExifData;
} SliderImage;

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gint iSlideTime;
	gchar *cDirectory;
	gboolean bSubDirs;
	gboolean bRandom;
	gboolean bNoStretch;
	gboolean bFillIcon;
	gboolean bImageName;
	gboolean bGetExifDataAtOnce;
	guint iBackgroundType;
	gdouble pBackgroundColor[4];
	SliderAnimation iAnimation;
	gint iNbAnimationStep;
	SliderClickOption iClickOption;
	SliderClickOption iMiddleClickOption;
	gboolean bUseThread;  // plante sur certaines images (svg) dans X :-(
} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	GList *pList;  // list d'images.
	GList *pElement;  // pointeur sur l'element courant de la liste.
	GList *pExifElement;  // pointeur sur l'element courant de la liste pour la recuperation des donnees exif.
	guint iSidExifIdle;  // SID de la tache exif.
	guint iTimerID;  // timer d'attente de la prochaine image.
	gboolean bPause;
	gdouble fAnimAlpha;  // ces 3 variables sont pour les animations.
	gint iAnimCNT;
	gint sens;
	SliderImageArea slideArea;  // aire de l'image courante.
	SliderImageArea prevSlideArea;  // aire de l'image precedente.
	cairo_surface_t* pCairoSurface;  // surface courante.
	cairo_surface_t* pPrevCairoSurface;  // surface precedente.
	GLuint iTexture;  // texture courante.
	GLuint iPrevTexture;  // texture precedente.
	gint iSurfaceWidth, iSurfaceHeight;  // dimension de la zone de dessin.
	SliderAnimation iAnimation;  // animation de transition courante.
	CairoDockTask *pMeasureDirectory;  // mesure pour parcourir le repertoire courant.
	CairoDockTask *pMeasureImage;  // mesure pour charger l'image courante.
	guint iScrollID;
	gint iNbScroll;
	gchar *cSelectedImagePath;  // donnee du menu
	GList *pAppList;  // donnee du menu
	gchar *cDirectory;
	gboolean bSubDirs;
	gboolean bRandom;
} ;


#endif

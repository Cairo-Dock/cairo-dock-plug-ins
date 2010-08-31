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


#ifndef __CD_CLOCK_STRUCT__
#define  __CD_CLOCK_STRUCT__

#include <cairo-dock.h>
#include <time.h>

//Frame de fond pour chaque partie de l'heure
typedef struct {
	int iWidth;
	int iHeight;
	int iXOffset;
	int iYOffset;
	cairo_surface_t *pFrameSurface;
} ClockDigitalFrame;

//Texte de l'heure.
typedef struct {
	int iXOffset;
	int iYOffset;
	cairo_surface_t *pTextSurface;
} ClockDigitalText;

//Mode digital de l'horloge, contient tout ce dont on a besoin
//Surfaces et paramètres X/Y/W/H
typedef struct {
 ClockDigitalFrame pFrame[4];
 ClockDigitalText pText[4];
 gboolean bSecondCapable;
 int iFrameSpacing;
 int i12modeWidth;
 int i12modeHeight;
 int i12modeXOffset;
 int i12modeYOffset;
 int i12modeFrame;
} ClockDigital;

typedef enum _LayerElement
{
	CLOCK_DROP_SHADOW = 0,
	CLOCK_FACE,
	CLOCK_MARKS,
	CLOCK_HOUR_HAND_SHADOW,
	CLOCK_MINUTE_HAND_SHADOW,
	CLOCK_SECOND_HAND_SHADOW,
	CLOCK_HOUR_HAND,
	CLOCK_MINUTE_HAND,
	CLOCK_SECOND_HAND,
	CLOCK_FACE_SHADOW,
	CLOCK_GLASS,
	CLOCK_FRAME,
	CLOCK_ELEMENTS
} LayerElement;

typedef enum _SurfaceKind
{
	KIND_BACKGROUND = 0,
	KIND_FOREGROUND,
	KIND_HOUR,
	KIND_MINUTE,
	KIND_SECOND,
} SurfaceKind;


typedef struct {
	gint iHour;
	gint iMinute;
	gint iDayOfWeek;
	gint iDayOfMonth;
	gchar *cMessage;
	gchar *cCommand;
	} CDClockAlarm;

typedef enum _CDClockTaskFrequency
{
	CD_TASK_DONT_REPEAT = 0,
	CD_TASK_EACH_MONTH,
	CD_TASK_EACH_YEAR,
	CD_TASK_NB_FREQUENCIES
} CDClockTaskFrequency;

typedef struct {
	// definition
	gchar *cID;
	guint iDay;  // 1-31
	guint iMonth;  // 0-11
	guint iYear;  // par exemple 2010
	gchar *cTitle;
	gchar *cText;
	gboolean bActive;  // = not done.
	gchar *cTags;  // ',' separated
	guint iHour;
	guint iMinute;
	CDClockTaskFrequency iFrequency;
	GList *pSubTaskList;
	// private data
	CairoDockModuleInstance *pApplet;
	gboolean b1DayWarning;
	gboolean b15mnWarning;
	gboolean bFirstWarning;
	gint iWarningDelay;  // en minutes.
	guint iSidWarning;
	CairoDialog *pWarningDialog;
	} CDClockTask;

typedef struct {
	void (*init) (CairoDockModuleInstance *myApplet);
	void (*stop) (CairoDockModuleInstance *myApplet);
	GList * (*get_tasks) (CairoDockModuleInstance *myApplet);
	gboolean (*create_task) (CDClockTask *pTask, CairoDockModuleInstance *myApplet);
	gboolean (*delete_task) (CDClockTask *pTask, CairoDockModuleInstance *myApplet);
	gboolean (*update_task) (CDClockTask *pTask, CairoDockModuleInstance *myApplet);
	gpointer pData;
	} CDClockTaskBackend;

typedef enum _CDClockTextLayout
{
	CD_TEXT_LAYOUT_AUTO = 0,
	CD_TEXT_LAYOUT_1_LINE,
	CD_TEXT_LAYOUT_2_LINES
} CDClockTextLayout;

struct _AppletConfig {
	CairoDockInfoDisplay iShowDate;
	gboolean bShowSeconds;
	gboolean bOldStyle;
	gboolean b24Mode;
	CDClockTextLayout iPreferedTextLayout;
	double fTextColor[4];
	double fDateColor[4];
	gchar *cThemePath;
	gchar *cNumericBackgroundImage;
	GPtrArray *pAlarms;
	gchar *cSetupTimeCommand;
	gchar *cFont;
	gint iWeight;
	gint iStyle;
	gboolean bOutlined;
	gdouble fTextRatio;
	gchar *cLocation;
	gchar *cDigital;
	gint iSmoothAnimationDuration;
	gboolean bSetName;
	gboolean bNormalDate;
	gchar *cTaskMgrName;
	} ;

struct _AppletData {
	cairo_surface_t *pBackgroundSurface;
	cairo_surface_t *pForegroundSurface;
	RsvgDimensionData DimensionData;
	RsvgDimensionData needleDimension;
	gint iNeedleRealWidth, iNeedleRealHeight;
	gdouble iNeedleOffsetX, iNeedleOffsetY;
	gdouble fNeedleScale;
	RsvgHandle *pSvgHandles[CLOCK_ELEMENTS];
	guint iSidUpdateClock;
	GPid iAlarmPID;
	gchar *cSystemLocation;
	gint iLastCheckedMinute, iLastCheckedDay, iLastCheckedMonth, iLastCheckedYear;
	struct tm currentTime;
	
	cairo_surface_t *pNumericBgSurface;
	ClockDigital pDigitalClock;
	guint iTextLayout;
	
	GLuint iBgTexture, iFgTexture, iHourNeedleTexture, iMinuteNeedleTexture, iSecondNeedleTexture, iDateTexture;
	gint iNeedleWidth, iNeedleHeight;
	gint iDateWidth, iDateHeight;
	gint iSmoothAnimationStep;
	
	GList *pTasks;
	CairoDialog *pCalendarDialog;
	GtkWidget *pTaskWindow;
	GHashTable *pBackends;
	CDClockTaskBackend *pBackend;
	GtkListStore *pModel;
	guint iButtonPressTime;
	CDClockTask *pNextTask;
	CDClockTask *pNextAnniversary;
	} ;

#endif



#ifndef __CD_CLOCK_STRUCT__
#define  __CD_CLOCK_STRUCT__

#include <glib.h>

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
	KIND_FOREGROUND
} SurfaceKind;


typedef struct {
	int iHour;
	int iMinute;
	int iDayOfWeek;
	int iDayOfMonth;
	gchar *cMessage;
	} CDClockAlarm;

typedef enum
{
	CLOCK_NO_DATE = 0,
	CLOCK_DATE_ON_ICON,
	CLOCK_DATE_ON_LABEL
} CDClockDatePosition;


#endif


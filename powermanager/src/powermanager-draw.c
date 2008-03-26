#include "string.h"
#include <glib/gi18n.h>

#include "powermanager-struct.h"
#include "powermanager-draw.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;

void iconWitness(int animationLenght)
{
	CD_APPLET_ANIMATE_MY_ICON (1, animationLenght)
}

void update_icon(void)
{
	if(myData.battery_present)
	{
		if (myData.previous_battery_time != myData.battery_time)
		{
			if(myConfig.quickInfoType == MY_APPLET_TIME)
			{
				CD_APPLET_SET_HOURS_MINUTES_AS_QUICK_INFO (myData.battery_time)
			}
			else if(myConfig.quickInfoType == MY_APPLET_CHARGE)
			{
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%d%%", myData.battery_charge)
			}
		}
		
		if (myData.previously_on_battery != myData.on_battery || myData.previous_battery_charge != myData.battery_charge)
		{
			myData.previously_on_battery = myData.on_battery;
			myData.previous_battery_charge = myData.battery_charge;
			
			if(myData.on_battery)
			{
				if(myData.battery_charge >= 95)
					{ CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON ("battery_44.svg") }
				else if(myData.battery_charge >= 65)
					{ CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON ("battery_34.svg") }
				else if(myData.battery_charge >= 35)
					{ CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON ("battery_24.svg") }
				else if(myData.battery_charge >= 5)
					{ CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON ("battery_14.svg") }
				else
					{ CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON ("battery_04.svg") }
			}
			else
			{
				if(myData.battery_charge >= 95)
					{ CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON ("charge_44.svg") }
				else if(myData.battery_charge >= 65)
					{ CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON ("charge_34.svg") }
				else if(myData.battery_charge >= 35)
					{ CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON ("charge_24.svg") }
				else if(myData.battery_charge >= 5)
					{ CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON ("charge_14.svg") }
				else
					{ CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON ("charge_04.svg") }
			}
		}
	}
	else
	{
		CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON ("sector.svg")
	}
}

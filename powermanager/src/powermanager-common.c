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

#include <math.h>
#include <string.h>

#include "powermanager-struct.h"
#include "powermanager-proc-acpi.h"
#include "powermanager-sys-class.h"
#include "powermanager-draw.h"
#include "powermanager-common.h"


static double cd_compute_current_rate (void)
{
	double fPresentRate = 0.;
	if (myData.iPrevPercentage > 0)
	{
		if (myData.iStatPercentageBegin != 0)
			myData.iStatTimeCount += myConfig.iCheckInterval;
		if (myData.iPrevPercentage != myData.iPercentage)  // we compute the rate from 2 changing points.
		{
			if (myData.iStatPercentageBegin == 0)
			{
				myData.iStatPercentageBegin = myData.iPercentage;
				myData.iStatTimeCount= 0;
				myData.iStatTime = 0;
			}
			else
			{
				myData.iStatPercentage = myData.iPercentage;
				myData.iStatTime = myData.iStatTimeCount;
			}
		}
		if (myData.iStatPercentage != 0)
			fPresentRate = (double)fabs (myData.iStatPercentage - myData.iStatPercentageBegin) / myData.iStatTime * 36.;
		cd_message ("instant rate : %.1f -> %.1f (%ds) => %.3f", (double)myData.iStatPercentageBegin, (double)myData.iStatPercentage, myData.iStatTime, fPresentRate);
		/**
		fPresentRate = (myData.iPrevPercentage - myData.iPercentage) * 36. * myData.iCapacity / myConfig.iCheckInterval;
		cd_message ("instant rate : %.2f -> %.2f => %.2f", (double)myData.iPrevPercentage, (double)myData.iPercentage, fPresentRate);
		
		if (fPresentRate != 0)
		{
			myData.fRateHistory[myData.iCurrentIndex] = fPresentRate;
			
			double fMeanRate = 0.;
			int nb_values=0;
			double fNextValue = 0.;
			int iNbStackingValues = 0;
			int i, k;
			for (k = 0; k < myData.iIndexMax; k ++)
			{
				if (myData.iIndexMax == PM_NB_VALUES)
					i = (myData.iCurrentIndex + 1 + k) % PM_NB_VALUES;
				else
					i = k;
				if (myData.fRateHistory[i] != 0)
				{
					if (fNextValue != 0)
					{
						nb_values += iNbStackingValues;
						fMeanRate += fNextValue;
					}
					fNextValue = myData.fRateHistory[i];
					iNbStackingValues = 1;
				}
				else
				{
					iNbStackingValues ++;
				}
			}
			if (nb_values != 0)
				fPresentRate = fabs (fMeanRate) / nb_values;
			cd_message ("mean calculated on %d value(s) : %.2f (current index:%d/%d)", nb_values, fPresentRate, myData.iCurrentIndex, myData.iIndexMax);
			
			myData.iCurrentIndex ++;
			if (myData.iIndexMax < PM_NB_VALUES)
				myData.iIndexMax = myData.iCurrentIndex;
			if (myData.iCurrentIndex == PM_NB_VALUES)
				myData.iCurrentIndex = 0;
		}*/
	}
	return fPresentRate;
}

static void cd_store_current_rate (double fPresentRate)
{
	if (myData.bOnBattery)
	{
		myData.fDischargeMeanRate = (myData.fDischargeMeanRate * myData.iNbDischargeMeasures + fPresentRate) / (myData.iNbDischargeMeasures + 1);
		myData.iNbDischargeMeasures ++;
		cd_debug ("fDischargeMeanRate : %.3f (%d)\n", myData.fDischargeMeanRate, myData.iNbDischargeMeasures);

		if (myConfig.fLastDischargeMeanRate == 0 || (double) fabs (myConfig.fLastDischargeMeanRate - myData.fDischargeMeanRate) / myConfig.fLastDischargeMeanRate > .01)  // l'ecart avec la valeur stockee en conf est devenue grande, on met a jour cette derniere.
		{
			g_print ("write discharge rate : %.2f -> %.2f\n", myConfig.fLastDischargeMeanRate, myData.fDischargeMeanRate);
			cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE,
				G_TYPE_DOUBLE, "Configuration", "discharge rate", myConfig.fLastDischargeMeanRate,
				G_TYPE_INVALID);
			myConfig.fLastDischargeMeanRate = myData.fDischargeMeanRate;
		}
	}
	else
	{
		myData.fChargeMeanRate = (myData.fChargeMeanRate * myData.iNbChargeMeasures + fPresentRate) / (myData.iNbChargeMeasures + 1);
		myData.iNbChargeMeasures ++;
		cd_debug ("fChargeMeanRate : %.3f (%d)\n", myData.fChargeMeanRate, myData.iNbChargeMeasures);
		if (myConfig.fLastChargeMeanRate == 0 || (double) fabs (myConfig.fLastChargeMeanRate - myData.fChargeMeanRate) / myConfig.fLastChargeMeanRate > .01)  // l'ecart avec la valeur stockee en conf est devenue grande, on met a jour cette derniere.
		{
			g_print ("charge rate : %.2f -> %.2f\n", myConfig.fLastChargeMeanRate, myData.fChargeMeanRate);
			cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE,
				G_TYPE_DOUBLE, "Configuration", "charge rate", myConfig.fLastChargeMeanRate,
				G_TYPE_INVALID);
			myConfig.fLastChargeMeanRate = myData.fChargeMeanRate;
		}
	}
}

static double cd_compute_time (double fPresentRate)
{
	double time = 0.;
	if (myData.iPercentage < 99.9)  // not charged or almost charged.
	{
		if (myData.bOnBattery)  // time before discharged.
		{
			if (fPresentRate > 0)
			{
				time = 36. * myData.iPercentage / fPresentRate;
			}
		}
		else  // time before charged
		{
			if (fPresentRate > 0)
			{
				time = 36. * (100. - myData.iPercentage) / fPresentRate;
			}
		}
	}
	return time;
}


int cd_estimate_time (void)
{
	//\_______________ compute the variation rate.
	double fPresentRate = cd_compute_current_rate ();
	
	//\_______________ store this value in conf if it has changed too much.
	if (fPresentRate > 0)
	{
		cd_store_current_rate (fPresentRate);
	}
	else if (myData.bOnBattery || myData.iPercentage < 99.9)  // if we are on sector and fully charged, the rate is of course 0.
	{
		cd_debug ("no rate, using last know values : %.2f ; %.2f\n", myConfig.fLastDischargeMeanRate, myConfig.fLastChargeMeanRate);
		fPresentRate = (myData.bOnBattery ? myConfig.fLastDischargeMeanRate : myConfig.fLastChargeMeanRate);
	}
	
	//\_______________ now compute the time.
	return cd_compute_time (fPresentRate);
}


static gboolean _update_stats (void)
{
	if (myData.bProcAcpiFound)
		cd_get_stats_from_proc_acpi ();
	else
		cd_get_stats_from_sys_class ();
	
	update_icon ();
	
	return (myData.cBatteryStateFilePath != NULL);
}

static gboolean _update_stats_loop (void)
{
	CD_APPLET_ENTER;
	
	gboolean bContinue = _update_stats ();
	
	if (! bContinue)
		myData.checkLoop = 0;
	CD_APPLET_LEAVE (bContinue);
}

 void cd_check_power_files (void)
{
	// try to find the battery
	myData.bProcAcpiFound = cd_find_battery_proc_acpi ();
	if (! myData.bProcAcpiFound)
		myData.bSysClassFound = cd_find_battery_sys_class ();
	
	// if found, get the current stats.
	if (myData.cBatteryStateFilePath != NULL)
		_update_stats ();
	
	// and keep them up-to-date.
	if (myData.cBatteryStateFilePath != NULL)
	{
		myData.fDischargeMeanRate = myConfig.fLastDischargeMeanRate;
		myData.fChargeMeanRate = myConfig.fLastChargeMeanRate;
		myData.checkLoop = g_timeout_add_seconds (myConfig.iCheckInterval, (GSourceFunc) _update_stats_loop, (gpointer) NULL);
	}
}


void cd_powermanager_change_loop_frequency (gint iSeconds)
{
	if (myData.checkLoop != 0)  // la frequence peut avoir change.
	{
		g_source_remove (myData.checkLoop);
		myData.checkLoop = g_timeout_add_seconds (iSeconds, (GSourceFunc) _update_stats_loop, (gpointer) NULL);
	}
}

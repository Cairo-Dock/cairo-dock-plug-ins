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
#include "powermanager-common.h"


double cd_compute_current_rate (void)
{
	double fPresentRate = 0.;
	if (myData.iPrevPercentage > 0)
	{
		fPresentRate = (myData.iPrevPercentage - myData.iPercentage) * 36. * myData.iCapacity / myConfig.iCheckInterval;
		cd_message ("instant rate : %.2f -> %.2f => %.2f", (double)myData.iPrevPercentage, (double)myData.iPercentage, fPresentRate);
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
	}
	return fPresentRate;
}

void cd_store_current_rate (double fPresentRate)
{
	if (myData.bOnBattery)
	{
		myData.fDischargeMeanRate = (myData.fDischargeMeanRate * myData.iNbDischargeMeasures + fPresentRate) / (myData.iNbDischargeMeasures + 1);
		myData.iNbDischargeMeasures ++;
		cd_debug ("fDischargeMeanRate : %.2f (%d)\n", myData.fDischargeMeanRate, myData.iNbDischargeMeasures);

		if (fabs (myConfig.fLastDischargeMeanRate - myData.fDischargeMeanRate) > 30)  // l'ecart avec la valeur stockee en conf est devenue grande, on met a jour cette derniere.
		{
			myConfig.fLastDischargeMeanRate = myData.fDischargeMeanRate;
			cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE,
				G_TYPE_DOUBLE, "Configuration", "discharge rate", myConfig.fLastDischargeMeanRate,
				G_TYPE_INVALID);
		}
	}
	else
	{
		myData.fChargeMeanRate = (myData.fChargeMeanRate * myData.iNbChargeMeasures + fPresentRate) / (myData.iNbChargeMeasures + 1);
		myData.iNbChargeMeasures ++;
		cd_debug ("fChargeMeanRate : %.2f (%d)\n", myData.fChargeMeanRate, myData.iNbChargeMeasures);
		if (fabs (myConfig.fLastChargeMeanRate - myData.fChargeMeanRate) > 30)  // l'ecart avec la valeur stockee en conf est devenue grande, on met a jour cette derniere.
		{
			myConfig.fLastChargeMeanRate = myData.fChargeMeanRate;
			cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE,
				G_TYPE_DOUBLE, "Configuration", "charge rate", myConfig.fLastChargeMeanRate,
				G_TYPE_INVALID);
		}
	}
}

double cd_compute_time (double fPresentRate, int iRemainingCapacity)
{
	double time = 0.;
	if (myData.iPercentage < 99.9)  // not charged or almost charged.
	{
		if (myData.bOnBattery)  // time before discharged.
		{
			if (fPresentRate > 0)
			{
				time = 3600. * iRemainingCapacity / fPresentRate;
			}
		}
		else  // time before charged
		{
			if (fPresentRate > 0)
			{
				time = 3600. * (myData.iCapacity - iRemainingCapacity) / fPresentRate;
			}
		}
	}
	return time;
}

/*********************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <string.h>
#include <math.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-draw.h"

CD_APPLET_INCLUDE_MY_VARS


#define TREE_WIDTH 150
#define TREE_HEIGHT 161
static int s_iLeafPosition[2][3*3] = {{-30,40,1 , 60,105,0 , -45,115,1},{-60,65,0 , 55,115,1 , -30,115,0}};

void cd_shortcuts_load_tree (GList *pIconsList, cairo_t *pCairoContext)
{
	g_print ("%s ()\n", __func__);
	myData.pDeskletIconList = pIconsList;
	myDesklet->icons = pIconsList;
	
	int iNbIcons = 0;
	Icon *pIcon;
	GList *ic;
	for (ic = pIconsList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (! CAIRO_DOCK_IS_SEPARATOR (pIcon))
			iNbIcons ++;
	}
	myData.iNbIconsInTree = iNbIcons;
	
	myData.iNbBranches = (int) ceil (1.*iNbIcons/3.);
	if (myData.iNbBranches == 0)
		return;
	
	double h = myDesklet->iHeight, w = myDesklet->iWidth;
	myData.fTreeWidthFactor = (w > TREE_WIDTH ? 1 : w / TREE_WIDTH);
	myData.fTreeHeightFactor = h / (myData.iNbBranches * TREE_HEIGHT);
	
	g_print (" -> %d icones, %d branches, proportions : %.2fx%.2f\n", myData.iNbIconsInTree, myData.iNbBranches, myData.fTreeWidthFactor, myData.fTreeHeightFactor);
	double fImageWidth = TREE_WIDTH * myData.fTreeWidthFactor, fImageHeight = TREE_HEIGHT * myData.fTreeHeightFactor;
	gchar *cImageFilePath = g_strconcat (MY_APPLET_SHARE_DATA_DIR, "/branche1.svg", NULL);
	myData.pBrancheSurface[0] = cairo_dock_load_image (pCairoContext,
		cImageFilePath,
		&fImageWidth,
		&fImageHeight,
		0.,
		1.,
		FALSE);
	g_free (cImageFilePath);
	
	cImageFilePath = g_strconcat (MY_APPLET_SHARE_DATA_DIR, "/branche2.svg", NULL);
	myData.pBrancheSurface[1] = cairo_dock_load_image (pCairoContext,
		cImageFilePath,
		&fImageWidth,
		&fImageHeight,
		0.,
		1.,
		FALSE);
	g_free (cImageFilePath);
}

void cd_shortcuts_draw_in_desklet (cairo_t *pCairoContext, gpointer data)
{
	double h = myDesklet->iHeight, w = myDesklet->iWidth;
	int i;
	for (i = 0; i < myData.iNbBranches; i ++)
	{
		cairo_save (pCairoContext);
		//g_print (" branche %d en (%.2f;%.2f)\n", i, (w - myData.fTreeWidthFactor * TREE_WIDTH) / 2, h - i * TREE_HEIGHT * myData.fTreeHeightFactor);
		cairo_translate (pCairoContext, (w - myData.fTreeWidthFactor * TREE_WIDTH) / 2, h - (i + 1) * TREE_HEIGHT * myData.fTreeHeightFactor);
		cairo_set_source_surface (pCairoContext, myData.pBrancheSurface[i%2], 0, 0);
		cairo_paint (pCairoContext);
		cairo_restore (pCairoContext);
	}
	
	
	int iBrancheNumber=0, iBrancheType=0, iLeafNumber=0;
	double x, y;
	int sens;
	Icon *pIcon;
	GList *ic;
	for (ic = myData.pDeskletIconList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (! CAIRO_DOCK_IS_SEPARATOR (pIcon))
		{
			x = s_iLeafPosition[iBrancheType][3*iLeafNumber];
			y = s_iLeafPosition[iBrancheType][3*iLeafNumber+1];
			sens = s_iLeafPosition[iBrancheType][3*iLeafNumber+2];
			
			pIcon->fDrawX = w / 2 + x * myData.fTreeWidthFactor - pIcon->fWidth / 2;
			pIcon->fDrawY = h - (iBrancheNumber * TREE_HEIGHT + y) * myData.fTreeHeightFactor - sens * pIcon->fHeight;
			pIcon->fScale = 1;
			pIcon->fAlpha = 1;
			pIcon->fWidthFactor = 1;
			pIcon->fHeightFactor = 1;
			
			cairo_save (pCairoContext);
			cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, FALSE, TRUE, myDesklet->iWidth);
			cairo_restore (pCairoContext);
			
			iLeafNumber ++;
			if (iLeafNumber == 3)
			{
				iLeafNumber = 0;
				iBrancheNumber ++;
				iBrancheType = iBrancheNumber % 2;
			}
		}
	}
}

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

//\________________ Add your name in the copyright file (and / or modify your name here)

#include <cairo-dock.h>
#include <math.h>
#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-xml.h"
#include "applet-nvidia.h"
#include "applet-cpusage.h"


char* ltrim( char* str, const char* t )  // Couper tout depuis la gauche
{
	char* curStr = NULL;

	char look[ 256 ] = { 1, 0 };
	while( *t )
		look[ (unsigned char)*t++ ] = 1;

	curStr = str;
	while( *curStr && look[ *curStr ] )
		++curStr;
	strcpy( str, curStr );
	
	return str; 
} 

char* rtrim( char* str, const char* t )  // Couper tout depuis la droite
{
	char* curEnd = str, *end = str;

	char look[ 256 ] = { 1, 0 };
	while( *t )
		look[ (unsigned char)*t++ ] = 1;

	while( *end )
	{
		if ( !look[ *end ] )
			curEnd = end + 1;
		++end;
	}
	*curEnd = '\0';

	return str;
}



double _Ko_to_Mo (CairoDockModuleInstance *myApplet , double fValueInKo)
{
	fValueInKo = fValueInKo / 1024;
	return fValueInKo;
}

double _Ko_to_Go (CairoDockModuleInstance *myApplet , double fValueInKo)
{
	fValueInKo = _Ko_to_Mo (myApplet ,fValueInKo);
	fValueInKo = _Ko_to_Mo (myApplet ,fValueInKo);
	return fValueInKo;
}


gchar* _AdjustPrecision0 (CairoDockModuleInstance *myApplet , double fValue)
{
	gchar *cReturnValue;
	gint iInteger;
	gint iDecimal;
	
	iInteger = (int)fValue;
	iDecimal = (int)((fValue-iInteger)*10);
	
	if (iDecimal >= 5)
		iInteger++;
	
	cReturnValue =  g_strdup_printf ("%i", iInteger);
	
	return cReturnValue;
	g_free (cReturnValue);
}

gchar* _AdjustPrecision1 (CairoDockModuleInstance *myApplet , double fValue)
{
	gchar *cReturnValue;
	gint iInteger;
	gint iDecimal;
	gint iAfterDecimal;
	
	iAfterDecimal = (int)(((fValue*10)-(int)(fValue*10))*10);
	iInteger = (int)fValue;
	iDecimal = (int)((fValue-iInteger)*10);
	
	if (iAfterDecimal >= 5)
		iDecimal++;
	if (iDecimal == 10)
		cReturnValue =  g_strdup_printf ("%i.0", iInteger+1);
	else
		cReturnValue =  g_strdup_printf ("%i.%i", iInteger, iDecimal);
	
	return cReturnValue;
	g_free (cReturnValue);
}

gchar* _AdjustPrecision2 (CairoDockModuleInstance *myApplet , double fValue)
{
	gchar *cReturnValue;
	gint iInteger;
	gint iDecimal;
	gint iAfterDecimal;
	
	iAfterDecimal = (int)(((fValue*100)-(int)(fValue*100))*10);
	iInteger = (int)fValue;
	iDecimal = (int)((fValue-iInteger)*100);
	
	if (iAfterDecimal >= 5)
		iDecimal++;
	if (iDecimal == 100)
		cReturnValue =  g_strdup_printf ("%i.00", iInteger+1);
	else
	{
		if (iDecimal < 10)
			cReturnValue =  g_strdup_printf ("%i.0%i", iInteger, iDecimal);
		else
			cReturnValue =  g_strdup_printf ("%i.%i", iInteger, iDecimal);
	}
	
	return cReturnValue;
	g_free (cReturnValue);
}


void cd_doncky_periodic_refresh (CairoDockModuleInstance *myApplet)
{
	myData.pPeriodicRefreshTask = cairo_dock_new_task (0,
		(CairoDockGetDataAsyncFunc) cd_launch_command,
		(CairoDockUpdateSyncFunc) cd_retrieve_command_result,
		myApplet);
	cairo_dock_launch_task (myData.pPeriodicRefreshTask);
}


void cd_launch_command (CairoDockModuleInstance *myApplet)
{
	
	GList *it;
	TextZone *pTextZone;
	
	for (it = myData.pTextZoneList; it != NULL; it = it->next)
	{
		pTextZone = it->data;
		
		if (!pTextZone->bImgDraw && pTextZone->cImgPath != NULL) // L'image n'a pas encore été chargée
		{
			
			if (pTextZone->iWidth == 0 && pTextZone->iHeight ==0) // On n'a pas forcé le ratio
			{
				if (pTextZone->iImgSize == 0) // Oups !! -> Pas de taille de spécifié -> On en impose une !
					pTextZone->iImgSize = 50;
					
				double fImgW=0, fImgH=0;
				CairoDockLoadImageModifier iLoadingModifier = 0;  /// CAIRO_DOCK_FILL_SPACE
				iLoadingModifier |= CAIRO_DOCK_KEEP_RATIO;
				
				cairo_t *pCairoContext = cairo_dock_create_context_from_container (myContainer);
				pTextZone->pImgSurface = cairo_dock_create_surface_from_image (pTextZone->cImgPath,
					pCairoContext, // myDrawContext,
					1.,
					pTextZone->iImgSize, pTextZone->iImgSize,
					iLoadingModifier,					
					&fImgW, &fImgH,
					NULL, NULL);
				cairo_destroy (pCairoContext);
							
				//\_______________ On garde l'aire de la surface/texture.			
				pTextZone->iWidth = (int)fImgW;
				pTextZone->iHeight = (int)fImgH;
			}
						
			pTextZone->pImgSurface = cairo_dock_create_surface_for_icon (pTextZone->cImgPath, myDrawContext,
					pTextZone->iWidth, pTextZone->iHeight);
					
			pTextZone->bImgDraw = TRUE; // L'image est désormais chargée -> On peut la dessiner
		}
		
		
		if (pTextZone->iRefresh != 0)
				pTextZone->iTimer++;
			
		
		if (pTextZone->bRefresh)
		{
			if (pTextZone->cInternal == NULL) // C'est une commande bash !
			{
				// cd_debug ("Doncky-debug : ----------------------> JE RAFFRAICHIS LA COMMANDE `%s`", pTextZone->cCommand);
				pTextZone->cResult = cairo_dock_launch_command_sync (pTextZone->cCommand);
			}
			else // C'est une commande interne !
			{
			
				if (strcmp (pTextZone->cInternal, "testsm") == 0)
				{
					cd_sysmonitor_get_nvidia_info (myApplet);
					
					pTextZone->cResult = g_strdup_printf ("myData.fCpuPercent : %i%% \nmyData.cpu_user : %lli \nmyData.cpu_user_nice : %lli \nmyData.cpu_system : %lli \nmyData.cpu_idle : %lli \nmyData.iNbCPU : %i \n"
						"myData.fRamPercent : %i%% \nmyData.ramUsed : %lli \nmyData.ramTotal : %lli \nmyData.ramCached : %lli \nmyData.ramFree : %lli \n"
						"myData.swapTotal : %lli \nmyData.swapFree : %lli \nmyData.fSwapPercent : %i%% \nmyData.swapUsed : %lli \n"
						"myData.cGPUName : %s \nmyData.iVideoRam : %iMB \nmyData.cDriverVersion : %s \nmyData.iGPUTemp : %i°C",
						(int)myData.fCpuPercent,						
						myData.cpu_user,
						myData.cpu_user_nice,
						myData.cpu_system,
						myData.cpu_idle,
						myData.iNbCPU,
						(int)myData.fRamPercent,
						myData.ramUsed,
						myData.ramTotal,
						myData.ramCached,
						myData.ramFree,
						myData.swapTotal,
						myData.swapFree,
						(int)myData.fSwapPercent,
						myData.swapUsed,
						myData.cGPUName,
						myData.iVideoRam,
						myData.cDriverVersion,
						myData.iGPUTemp);
				}
										
				else if (strcmp (pTextZone->cInternal, "cpuperc") == 0)
				{
					pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision0 (myApplet, myData.fCpuPercent));
					
					if (atof(pTextZone->cResult) < 10)
						pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision1 (myApplet, myData.fCpuPercent));
					else
						pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision0 (myApplet, myData.fCpuPercent));
				}
				
				else if (strcmp (pTextZone->cInternal, "cpuperc2f") == 0) // Restreint à 2 chiffres : de 00 à 99 !
				{
					pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision0 (myApplet, myData.fCpuPercent));
					
					if (atof(pTextZone->cResult) < 10)
						pTextZone->cResult = g_strdup_printf ("0%s", _AdjustPrecision0 (myApplet, myData.fCpuPercent));
					else if (atof(pTextZone->cResult) == 100)
						pTextZone->cResult = g_strdup_printf ("99");
					else
						pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision0 (myApplet, myData.fCpuPercent));
				}
					
				else if (strcmp (pTextZone->cInternal, "memperc") == 0)
				{
					pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision0 (myApplet, myData.fRamPercent));
					
					if (atof(pTextZone->cResult) < 10)
						pTextZone->cResult = g_strdup_printf ("0%s", _AdjustPrecision1 (myApplet, myData.fRamPercent));
					else if (atof(pTextZone->cResult) == 100)
						pTextZone->cResult = g_strdup_printf ("99");						
					else
						pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision0 (myApplet, myData.fRamPercent));
				}
				
				else if (strcmp (pTextZone->cInternal, "memperc2f") == 0) // Restreint à 2 chiffres : de 00 à 99 !
				{
					pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision0 (myApplet, myData.fRamPercent));
					
					if (atof(pTextZone->cResult) < 10)
						pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision1 (myApplet, myData.fRamPercent));
					else
						pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision0 (myApplet, myData.fRamPercent));
				}
	
										
				else if (strcmp (pTextZone->cInternal, "mem") == 0) // en Mo
				{
					pTextZone->cResult = g_strdup_printf ("%f", _Ko_to_Mo(myApplet, myData.ramTotal /100. * myData.fRamPercent));
					pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision0 (myApplet, atof(pTextZone->cResult)));
				}
				
				else if (strcmp (pTextZone->cInternal, "memg") == 0) // Identique à mem mais en Go
				{
					pTextZone->cResult = g_strdup_printf ("%f", _Ko_to_Go(myApplet, myData.ramTotal /100. * myData.fRamPercent));
					pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision2 (myApplet, atof(pTextZone->cResult)));
				}

				else if (strcmp (pTextZone->cInternal, "memmax") == 0) // en Mo
				{
					pTextZone->cResult = g_strdup_printf ("%f", _Ko_to_Mo(myApplet, myData.ramTotal));
					pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision0 (myApplet, atof(pTextZone->cResult)));
				}
				
				else if (strcmp (pTextZone->cInternal, "memmaxg") == 0) // Identique à memmax mais en Go
				{
					pTextZone->cResult = g_strdup_printf ("%f", _Ko_to_Go(myApplet, myData.ramTotal));
					pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision2 (myApplet, atof(pTextZone->cResult)));
				}
				
								
				else if (strcmp (pTextZone->cInternal, "swapperc") == 0)
				{
					pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision0 (myApplet, myData.fSwapPercent));
					
					if (atof(pTextZone->cResult) < 10)
						pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision1 (myApplet, myData.fSwapPercent));
					else
						pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision0 (myApplet, myData.fSwapPercent));
				}
				
				else if (strcmp (pTextZone->cInternal, "swapperc2f") == 0) // Restreint à 2 chiffres : de 00 à 99 !
				{
					pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision0 (myApplet, myData.fSwapPercent));
					
					if (atof(pTextZone->cResult) < 10)
						pTextZone->cResult = g_strdup_printf ("0%s", _AdjustPrecision0 (myApplet, myData.fSwapPercent));
					else if (atof(pTextZone->cResult) == 100)
						pTextZone->cResult = g_strdup_printf ("99");
					else
						pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision0 (myApplet, myData.fSwapPercent));
				}
				
				else if (strcmp (pTextZone->cInternal, "swap") == 0) // en Mo
				{
					pTextZone->cResult = g_strdup_printf ("%f", _Ko_to_Mo(myApplet, myData.swapUsed));
					
					if (atof(pTextZone->cResult) < 10)
						pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision2 (myApplet, atof(pTextZone->cResult)));
					else if (atof(pTextZone->cResult) < 100)
						pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision1 (myApplet, atof(pTextZone->cResult)));
					else
						pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision0 (myApplet, atof(pTextZone->cResult)));
				}
				
				else if (strcmp (pTextZone->cInternal, "swapg") == 0) // Identique à swap mais en Go
				{
					pTextZone->cResult = g_strdup_printf ("%f", _Ko_to_Go(myApplet, myData.swapUsed));
					pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision2 (myApplet, atof(pTextZone->cResult)));
				}
				
				else if (strcmp (pTextZone->cInternal, "swapmax") == 0) // en Mo
				{
					pTextZone->cResult = g_strdup_printf ("%f", _Ko_to_Mo(myApplet, myData.swapTotal));
					pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision0 (myApplet, atof(pTextZone->cResult)));
				}
				
				else if (strcmp (pTextZone->cInternal, "swapmaxg") == 0) // Identique à swapmax mais en Go
				{
					pTextZone->cResult = g_strdup_printf ("%f", _Ko_to_Go(myApplet, myData.swapTotal));
					pTextZone->cResult = g_strdup_printf ("%s", _AdjustPrecision2 (myApplet, atof(pTextZone->cResult)));
				}
				
				else if (strcmp (pTextZone->cInternal, "nvtemp") == 0)
					pTextZone->cResult = g_strdup_printf ("%i", myData.iGPUTemp);
					
				else if (strcmp (pTextZone->cInternal, "nvname") == 0)
				{
					cd_sysmonitor_get_nvidia_info (myApplet);					
					pTextZone->cResult = g_strdup_printf ("%s", myData.cGPUName);					
				}
				
				else if (strcmp (pTextZone->cInternal, "nvdriver") == 0)
				{
					cd_sysmonitor_get_nvidia_info (myApplet);					
					pTextZone->cResult = g_strdup_printf ("%s", myData.cDriverVersion);					
				}
				
				else if (strcmp (pTextZone->cInternal, "nvram") == 0)
				{
					cd_sysmonitor_get_nvidia_info (myApplet);					
					pTextZone->cResult = g_strdup_printf ("%i", myData.iVideoRam);					
				}
				
				else if (strcmp (pTextZone->cInternal, "uptime") == 0)
				{
					gchar *cUpTime = NULL, *cActivityTime = NULL;
					cd_sysmonitor_get_uptime (&cUpTime, &cActivityTime);		
					pTextZone->cResult = g_strdup_printf ("%s", cUpTime);			
				}
			}			
		}
	}	
}

void cd_retrieve_command_result (CairoDockModuleInstance *myApplet)
{
	GList *it;
	TextZone *pTextZone;
	

	for (it = myData.pTextZoneList; it != NULL; it = it->next)
	{
		pTextZone = it->data;
			
		if (pTextZone->iRefresh != 0 || pTextZone->bRefresh)
		{
			if (pTextZone->bRefresh && pTextZone->cResult != NULL)
			{
				pTextZone->cText = g_strdup_printf ("%s",pTextZone->cResult);
			}
						    
			if (pTextZone->iRefresh != 0 && pTextZone->iTimer >= pTextZone->iRefresh)
			{
				pTextZone->bRefresh = TRUE;
				pTextZone->iTimer = 0; // On remet le timer à 0
			}
			else
				pTextZone->bRefresh = FALSE;
		}
	}	
	cd_applet_update_my_icon (myApplet); // Quand tous les textes sont chargés, on peut dessiner
	myData.pPeriodicRefreshTask = NULL;
}


void cd_applet_draw_my_desklet (CairoDockModuleInstance *myApplet, int iWidth, int iHeight)
{
	if (iWidth < 20 || iHeight < 20)  // inutile de dessiner tant que le desklet n'a pas atteint sa taille definitive.
		return;
	cd_debug ("%s (%dx%d)", __func__, iWidth, iHeight);
	PangoLayout *pLayout = pango_cairo_create_layout (myDrawContext);
	PangoRectangle ink, log;
	
	// On efface la surface cairo actuelle
	cairo_dock_erase_cairo_context (myDrawContext);	
	
	// dessin du fond (optionnel).
	if (myConfig.bDisplayBackground)
	{
		cairo_save (myDrawContext);
		cairo_translate (myDrawContext,
				.5*myConfig.iBorderThickness,
				.5*myConfig.iBorderThickness);		
		cairo_pattern_t *pGradationPattern = cairo_pattern_create_linear (0.,
			0.,
			0.,
			iHeight);
		cairo_pattern_add_color_stop_rgba (pGradationPattern,
			0.,
			myConfig.fBackgroundColor1[0],
			myConfig.fBackgroundColor1[1],
			myConfig.fBackgroundColor1[2],
			myConfig.fBackgroundColor1[3]);
		cairo_pattern_add_color_stop_rgba (pGradationPattern,
			1.,
			myConfig.fBackgroundColor2[0],
			myConfig.fBackgroundColor2[1],
			myConfig.fBackgroundColor2[2],
			myConfig.fBackgroundColor2[3]);
		cairo_set_source (myDrawContext, pGradationPattern);
		
		if (myConfig.iBackgroundRadius != 0)  // On a besoin d'un rayon
		{
			cairo_dock_draw_rounded_rectangle (myDrawContext,
				myConfig.iBackgroundRadius,
				0.,
				iWidth - myConfig.iBorderThickness - 2 * myConfig.iBackgroundRadius,
				iHeight - myConfig.iBorderThickness);
		}
		else  // Il ne faut pas de rayon
		{
			cairo_rectangle (myDrawContext,
				0., 0.,
				iWidth - myConfig.iBorderThickness,
				iHeight - myConfig.iBorderThickness);
		}
		cairo_fill (myDrawContext);
		cairo_pattern_destroy (pGradationPattern);
		cairo_restore (myDrawContext);
	}
	
	
	
	
	
	
	// ################################################################################################################################################################
	// ############ DEBUT DU DESSIN DU TEXTE
	// ################################################################################################################################################################
	
	int iMargin = .5 * myConfig.iBorderThickness + (1. - sqrt (2) / 2) * myConfig.iBackgroundRadius;  // marge a gauche et au-dessus, pour ne pas mordre sur le coin arrondi.
	
	myData.fCurrentX = iMargin + myConfig.iTextMargin;  // position du curseur sur la ligne.
	myData.fCurrentY = iMargin + myConfig.iTextMargin;  // position de la ligne courante.
	
	GList *it = myData.pTextZoneList;
	TextZone *pTextZone;
	
	GList *it1;
	gboolean bFirstPass=TRUE;

	double fCurrentLineWidth = 0; // Dimension de la ligne complète (= avec plusieurs zones de textes)
	double fCurrentLineHeight = 0;
	gboolean bFirstTextInLine = TRUE;
		
		
		
	if (myData.pTextZoneList == NULL)
		return;
	
	it = myData.pTextZoneList;
	do
	{
		it1=it;
		do // boucle sur it de it1 jusqu'à retour chariot => line_width
		{
			pTextZone = it->data;
			
			if (pTextZone->cFont == NULL || pTextZone->cFont =="") // Si aucune font -> on prend celle de la config
				pTextZone->cFont = g_strdup_printf("%s", myConfig.cDefaultFont);
			
			
			if (pTextZone->cText != NULL) // On récupère la largeur d'un espace pour caler le texte à droite
			{
				PangoFontDescription *fd = pango_font_description_from_string (pTextZone->cFont);
				pango_layout_set_font_description (pLayout, fd);
				pango_font_description_free (fd);
				
				pango_layout_set_text (pLayout, " ", -1);
				pango_layout_get_pixel_extents (pLayout, &ink, &log);

				pTextZone->iFontSize = (int)log.width;
			}

			
			PangoFontDescription *fd = pango_font_description_from_string (pTextZone->cFont);
			pango_layout_set_font_description (pLayout, fd);
			pango_font_description_free (fd);
			
			pango_layout_set_text (pLayout, pTextZone->cText, -1);
			pango_layout_get_pixel_extents (pLayout, &ink, &log);
			
			if (pTextZone->cText != NULL)
				fCurrentLineWidth += log.width;
			else if (pTextZone->bLimitedBar || pTextZone->bImgDraw)
				fCurrentLineWidth += pTextZone->iWidth;
			
			
			
			// A voir comment définir l'alignement en Z
			if (bFirstTextInLine)
				fCurrentLineHeight = log.height;
			else
			{
				if (log.height > fCurrentLineHeight)
					fCurrentLineHeight = log.height;				
			}
			
			
			
			
			it = it->next;
		} while (it != NULL && ! pTextZone->bEndOfLine);
		
		
		// boucle sur it de it1 jusqu'à retour chariot => dessin
		it = it1;
		do
		{
	
			pTextZone = it->data;
			
			if (pTextZone->cFont == NULL || pTextZone->cFont =="") // Si aucune font -> on prend celle de la config
				pTextZone->cFont = g_strdup_printf("%s", myConfig.cDefaultFont);
			
			PangoFontDescription *fd = pango_font_description_from_string (pTextZone->cFont);
			pango_layout_set_font_description (pLayout, fd);
			pango_font_description_free (fd);
			
			
			if (pTextZone->fTextColor[3]== 0 && pTextZone->cText != NULL) // Si aucune couleur alors qu'un texte est défini -> on prend celle de la config
				cairo_set_source_rgba (myDrawContext, myConfig.fDefaultTextColor[0], myConfig.fDefaultTextColor[1], myConfig.fDefaultTextColor[2], myConfig.fDefaultTextColor[3]);
			else // sinon, on utilise la couleur du .xml
				cairo_set_source_rgba (myDrawContext, pTextZone->fTextColor[0], pTextZone->fTextColor[1], pTextZone->fTextColor[2], pTextZone->fTextColor[3]);
			
			myData.cCurrentText = g_strdup_printf ("%s",pTextZone->cText);
			
			pango_layout_set_text (pLayout, myData.cCurrentText, -1);
			pango_layout_get_pixel_extents (pLayout, &ink, &log);

			
			if (bFirstTextInLine) // On calcule le décalage nécéssaire pour respecter l'alignement souhaité
			{
				int iWidth, iHeight;
				CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
				
				if (strcmp (pTextZone->cAlign, "left") == 0)
					myData.fCurrentX = iMargin + myConfig.iTextMargin;
				else if (strcmp (pTextZone->cAlign, "center") == 0)
				{
					if (pTextZone->cImgPath != NULL)
						myData.fCurrentX = (iWidth/2) - (pTextZone->iWidth/2);
					else
						myData.fCurrentX = (iWidth/2)- (fCurrentLineWidth/2);
				}
				else if (strcmp (pTextZone->cAlign, "right") == 0)
				{
					if (pTextZone->cImgPath != NULL)
						myData.fCurrentX = iWidth - iMargin - myConfig.iTextMargin - pTextZone->iWidth - pTextZone->iFontSize;
					else
						myData.fCurrentX = iWidth - iMargin - myConfig.iTextMargin - fCurrentLineWidth - pTextZone->iFontSize;
				}
			}
			
			if (pTextZone->bBar || pTextZone->bLimitedBar) // On dessine la barre
			{
				int value;
				myData.cCurrentText = g_strdup_printf ("%s",pTextZone->cText);
				
				if (pTextZone->bBar)
				{
					int iWidth, iHeight;
					CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
					pTextZone->iWidth =  iWidth - iMargin*2 - myConfig.iTextMargin - (myData.fCurrentX + 8);
				}
				
					
				
				int i;
				for (i=0 ; i < 2 ; i++)
				{
					if (i==0)
						value = (int)((atof(myData.cCurrentText) / 100)*pTextZone->iWidth);
					else
						value = pTextZone->iWidth;						
					
					// On dessine la valeur PUIS le cadre							
					cairo_save (myDrawContext);
					
					if (pTextZone->fTextColor[3]== 0 && pTextZone->cText != NULL) // Si aucune couleur
						cairo_set_source_rgba (myDrawContext,
							myConfig.fDefaultTextColor[0],
							myConfig.fDefaultTextColor[1],
							myConfig.fDefaultTextColor[2],
							myConfig.fDefaultTextColor[3]);
					else
						cairo_set_source_rgba (myDrawContext,
							pTextZone->fTextColor[0],
							pTextZone->fTextColor[1],
							pTextZone->fTextColor[2],
							pTextZone->fTextColor[3]);
											
					cairo_set_line_width (myDrawContext, 0.5);
					cairo_translate (myDrawContext,
						myData.fCurrentX + 8, // On décale légèrement la barre du texte
						myData.fCurrentY + log.height*4/5  - pTextZone->iHeight);
					cairo_dock_draw_rounded_rectangle (myDrawContext,
						0,
						0.,
						value,
						pTextZone->iHeight);
					
					if (i==0)
						cairo_fill (myDrawContext);
					else
						cairo_stroke (myDrawContext);
					cairo_restore (myDrawContext);					
				}
			}
			else if (pTextZone->bImgDraw) // Il y a une image
			{
				if (pTextZone->pImgSurface != NULL)
				{
					cairo_set_source_surface (myDrawContext, pTextZone->pImgSurface,
							myData.fCurrentX,
							myData.fCurrentY);
					cairo_paint (myDrawContext);
				}			
			}
			else // C'est un texte !
			{
				cairo_move_to (myDrawContext,
					myData.fCurrentX,
					myData.fCurrentY + (fCurrentLineHeight *4/5) - (log.height * 4/5)); // *4/5 pour se caler sur le bas du texte avec la font la plus grosse
				pango_cairo_show_layout (myDrawContext, pLayout); // On dessine la ligne sur notre desklet
			}
				
			if (pTextZone->bEndOfLine) // On passe à la ligne du dessous
			{
				if (pTextZone->bNextNewLine) // Sinon, on reste sur la même ligne -> Utile pour avoir 1 zone alignée à gauche et 1 zone alignée à droite par exemple ;-)
				{
					if (pTextZone->bImgDraw)
						myData.fCurrentY += pTextZone->iHeight + myConfig.iSpaceBetweenLines;
					else
						myData.fCurrentY += log.height + myConfig.iSpaceBetweenLines;
				}
				//~ else if (pTextZone->bNextNewLine && (pTextZone->bBar || pTextZone->bLimitedBar) )
					//~ myData.fCurrentY += myConfig.iSpaceBetweenLines + pTextZone->iHeight;
								
				myData.fCurrentX = iMargin + myConfig.iTextMargin;
				
				bFirstTextInLine = TRUE;
				fCurrentLineWidth = 0;
			}
			else // On laisse le curseur à la position actuelle
			{
				if (pTextZone->bBar || pTextZone->bLimitedBar)  // On prend en compte un éventuelle barre
					myData.fCurrentX += pTextZone->iWidth + 16 ;
				else  // valable pour les textes et les images
				{
					double w;
					pango_layout_get_pixel_extents (pLayout, &ink, &log);
				
					w = log.width + log.x;
					myData.fCurrentX += w;
				}
				
				bFirstTextInLine = FALSE;
			} 
			
			it = it->next;
		} while (it != NULL && ! pTextZone->bEndOfLine);

	} while (it != NULL);
	
	// ################################################################################################################################################################
	// ############ FIN DU DESSIN DU TEXTE
	// ################################################################################################################################################################
	
	
	// dessin du cadre (optionnel).
	if (myConfig.bDisplayBackground)
	{
		cairo_save (myDrawContext);
		cairo_set_source_rgba (myDrawContext,
			myConfig.fBorderColor[0],
			myConfig.fBorderColor[1],
			myConfig.fBorderColor[2],
			myConfig.fBorderColor[3]);
		cairo_set_line_width (myDrawContext, myConfig.iBorderThickness);
		cairo_translate (myDrawContext,
			.5*myConfig.iBorderThickness,
			.5*myConfig.iBorderThickness);
		cairo_dock_draw_rounded_rectangle (myDrawContext,
			myConfig.iBackgroundRadius,
			0.,
			iWidth - 2*myConfig.iBackgroundRadius - myConfig.iBorderThickness,
			iHeight - myConfig.iBorderThickness);
		cairo_stroke (myDrawContext);
		cairo_restore (myDrawContext);
	}
	
	g_object_unref (pLayout);
	
	
	// on met a jour la texture OpenGL.
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
	{
		cairo_dock_update_icon_texture (myIcon);
	}
}


void cd_applet_update_my_icon (CairoDockModuleInstance *myApplet)
{
	if (myDesklet)
	{
		// taille de la texture.
		int iWidth, iHeight;
		CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
		
		cd_applet_draw_my_desklet (myApplet, iWidth, iHeight);		
		
		CD_APPLET_REDRAW_MY_ICON;
	}
}

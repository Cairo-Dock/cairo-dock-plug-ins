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
#include "applet-disk-usage.h"


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


void cd_doncky_periodic_refresh (CairoDockModuleInstance *myApplet)
{
	cd_debug ("Doncky-debug ---------------> REFRESH");
	
	
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
				cd_debug ("Doncky-debug ------------> pTextZone->iWidth = %i", pTextZone->iWidth);
				pTextZone->iHeight = (int)fImgH;
				cd_debug ("Doncky-debug ------------> pTextZone->iHeight = %i", pTextZone->iHeight);
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
					pTextZone->cResult = g_strdup_printf ("%.0f", myData.fCpuPercent);
										
					if (atof(pTextZone->cResult) < 10)
						pTextZone->cResult = g_strdup_printf ("%.1f", myData.fCpuPercent);
					else
						pTextZone->cResult = g_strdup_printf ("%.0f", myData.fCpuPercent);
				}
				
				else if (strcmp (pTextZone->cInternal, "cpuperc2f") == 0) // Restreint à 2 chiffres : de 00 à 99 !
				{
					pTextZone->cResult = g_strdup_printf ("%.0f", myData.fCpuPercent);
					
					if (atof(pTextZone->cResult) < 10)
						pTextZone->cResult = g_strdup_printf ("0%.0f", myData.fCpuPercent);
					else if (atof(pTextZone->cResult) == 100)
						pTextZone->cResult = g_strdup_printf ("99");
					else
						pTextZone->cResult = g_strdup_printf ("%.0f", myData.fCpuPercent);
				}
					
				else if (strcmp (pTextZone->cInternal, "memperc") == 0)
				{
					pTextZone->cResult = g_strdup_printf ("%.0f", myData.fRamPercent);
					
					if (atof(pTextZone->cResult) < 10)
						pTextZone->cResult = g_strdup_printf ("0%.0f", myData.fRamPercent);
					else if (atof(pTextZone->cResult) == 100)
						pTextZone->cResult = g_strdup_printf ("99");						
					else
						pTextZone->cResult = g_strdup_printf ("%.0f", myData.fRamPercent);
				}
				
				else if (strcmp (pTextZone->cInternal, "memperc2f") == 0) // Restreint à 2 chiffres : de 00 à 99 !
				{
					pTextZone->cResult = g_strdup_printf ("%.0f", myData.fRamPercent);
					
					if (atof(pTextZone->cResult) < 10)
						pTextZone->cResult = g_strdup_printf ("%.1f", myData.fRamPercent);
					else
						pTextZone->cResult = g_strdup_printf ("%.0f", myData.fRamPercent);
				}
	
										
				else if (strcmp (pTextZone->cInternal, "mem") == 0) // en Mo
				{
					pTextZone->cResult = g_strdup_printf ("%f", _Ko_to_Mo(myApplet, myData.ramTotal /100. * myData.fRamPercent));
					pTextZone->cResult = g_strdup_printf ("%.0f", atof(pTextZone->cResult));
				}
				
				else if (strcmp (pTextZone->cInternal, "memg") == 0) // Identique à mem mais en Go
				{
					pTextZone->cResult = g_strdup_printf ("%f", _Ko_to_Go(myApplet, myData.ramTotal /100. * myData.fRamPercent));
					pTextZone->cResult = g_strdup_printf ("%.2f", atof(pTextZone->cResult));
				}

				else if (strcmp (pTextZone->cInternal, "memmax") == 0) // en Mo
				{
					pTextZone->cResult = g_strdup_printf ("%f", _Ko_to_Mo(myApplet, myData.ramTotal));
					pTextZone->cResult = g_strdup_printf ("%.0f", atof(pTextZone->cResult));
				}
				
				else if (strcmp (pTextZone->cInternal, "memmaxg") == 0) // Identique à memmax mais en Go
				{
					pTextZone->cResult = g_strdup_printf ("%f", _Ko_to_Go(myApplet, myData.ramTotal));
					pTextZone->cResult = g_strdup_printf ("%.2f", atof(pTextZone->cResult));
				}
				
								
				else if (strcmp (pTextZone->cInternal, "swapperc") == 0)
				{
					pTextZone->cResult = g_strdup_printf ("%.0f", myData.fSwapPercent);
					
					if (atof(pTextZone->cResult) < 10)
						pTextZone->cResult = g_strdup_printf ("%.1f", myData.fSwapPercent);
					else
						pTextZone->cResult = g_strdup_printf ("%.0f", myData.fSwapPercent);				
				}
				
				else if (strcmp (pTextZone->cInternal, "swapperc2f") == 0) // Restreint à 2 chiffres : de 00 à 99 !
				{
					pTextZone->cResult = g_strdup_printf ("%.0f", myData.fSwapPercent);
					
					if (atof(pTextZone->cResult) < 10)
						pTextZone->cResult = g_strdup_printf ("0%.0f", myData.fSwapPercent);
					else if (atof(pTextZone->cResult) == 100)
						pTextZone->cResult = g_strdup_printf ("99");
					else
						pTextZone->cResult = g_strdup_printf ("%.0f", myData.fSwapPercent);
				}
				
				else if (strcmp (pTextZone->cInternal, "swap") == 0) // en Mo
				{
					pTextZone->cResult = g_strdup_printf ("%f", _Ko_to_Mo(myApplet, myData.swapUsed));
					
					if (atof(pTextZone->cResult) < 10)
						pTextZone->cResult = g_strdup_printf ("%.2f", atof(pTextZone->cResult));
					else if (atof(pTextZone->cResult) < 100)
						pTextZone->cResult = g_strdup_printf ("%.1f", atof(pTextZone->cResult));
					else
						pTextZone->cResult = g_strdup_printf ("%.0f", atof(pTextZone->cResult));
				}
				
				else if (strcmp (pTextZone->cInternal, "swapg") == 0) // Identique à swap mais en Go
				{
					pTextZone->cResult = g_strdup_printf ("%f", _Ko_to_Go(myApplet, myData.swapUsed));
					pTextZone->cResult = g_strdup_printf ("%.2f", atof(pTextZone->cResult));
				}
				
				else if (strcmp (pTextZone->cInternal, "swapmax") == 0) // en Mo
				{
					pTextZone->cResult = g_strdup_printf ("%f", _Ko_to_Mo(myApplet, myData.swapTotal));
					pTextZone->cResult = g_strdup_printf ("%.0f", atof(pTextZone->cResult));
				}
				
				else if (strcmp (pTextZone->cInternal, "swapmaxg") == 0) // Identique à swapmax mais en Go
				{
					pTextZone->cResult = g_strdup_printf ("%f", _Ko_to_Go(myApplet, myData.swapTotal));
					pTextZone->cResult = g_strdup_printf ("%.2f", atof(pTextZone->cResult));
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
				
				else if (strcmp (pTextZone->cInternal, "fs_size") == 0)
				{
					if (strcmp (pTextZone->cMountPoint, "") != 0)
						pTextZone->cResult = g_strdup_printf ("%s", cd_doncky_get_disk_info (pTextZone->cMountPoint, 0));
				}
				
				else if (strcmp (pTextZone->cInternal, "fs_free") == 0)
					pTextZone->cResult = g_strdup_printf ("%s", cd_doncky_get_disk_info (pTextZone->cMountPoint, 1));
					
				else if (strcmp (pTextZone->cInternal, "fs_used") == 0)
					pTextZone->cResult = g_strdup_printf ("%s", cd_doncky_get_disk_info (pTextZone->cMountPoint, 2));
				
				else if (strcmp (pTextZone->cInternal, "fs_freeperc") == 0)
					pTextZone->cResult = g_strdup_printf ("%s", cd_doncky_get_disk_info (pTextZone->cMountPoint, 3));
				
				else if (strcmp (pTextZone->cInternal, "fs_usedperc") == 0)
				{
					if (strcmp (pTextZone->cMountPoint, "") == 0 || pTextZone->cMountPoint == NULL)
						pTextZone->cMountPoint = g_strdup_printf ("/");	
					pTextZone->cResult = g_strdup_printf ("%s", cd_doncky_get_disk_info (pTextZone->cMountPoint, 4));
				}
				
				else if (strcmp (pTextZone->cInternal, "fs_type") == 0)
					pTextZone->cResult = g_strdup_printf ("%s", cd_doncky_get_disk_info (pTextZone->cMountPoint, 5));
				
				else if (strcmp (pTextZone->cInternal, "fs_device") == 0)
					pTextZone->cResult = g_strdup_printf ("%s", cd_doncky_get_disk_info (pTextZone->cMountPoint, 6));				
				
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
			else // Pas de refresh de spécifié
			{
				
				if (pTextZone->cText == NULL || strcmp (pTextZone->cText, "") == 0) // La récupération s'est mal passé -> On relance !
					pTextZone->bRefresh = TRUE;
				else			
					pTextZone->bRefresh = FALSE; // On a récupéré l'info -> On arrête là !
			}
		}	
	}
	cd_applet_update_my_icon (myApplet); // Quand tous les textes sont chargés, on peut dessiner
	myData.pPeriodicRefreshTask = NULL;
}


void cd_applet_draw_my_desklet (CairoDockModuleInstance *myApplet, int iWidth, int iHeight)
{
	if (iWidth < 20 || iHeight < 20)  // inutile de dessiner tant que le desklet n'a pas atteint sa taille definitive.
		return;
	//cd_debug ("Doncky-debug --> %s (%dx%d)", __func__, iWidth, iHeight);
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
	int iSpaceBetweenElements = 8; // Décalage horizontal entre les barres et les autres éléments <- Sinon, tout est collé et c'est pô bô ! :-D
	
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
		
		//\_______ boucle sur it de it1 jusqu'à retour chariot => LINE_WIDTH & LINE_HEIGHT
		do 
		{
			pTextZone = it->data;
			
			if (pTextZone->cFont == NULL || pTextZone->cFont =="") // Si aucune font -> on prend celle de la config
				pTextZone->cFont = g_strdup_printf("%s", myConfig.cDefaultFont);			
						
			if (pTextZone->cText != NULL && ! (pTextZone->bBar || pTextZone->bLimitedBar || pTextZone->bImgDraw))
			{
				PangoFontDescription *fd = pango_font_description_from_string (pTextZone->cFont);
				pango_layout_set_font_description (pLayout, fd);
				pango_font_description_free (fd);
				
				// On récupère la largeur d'un espace pour caler le texte à droite
				pango_layout_set_text (pLayout, " ", -1);
				pango_layout_get_pixel_extents (pLayout, &ink, &log);
				pTextZone->iFontSizeWidth = (int)log.width;
				pTextZone->iFontSizeHeight = (int)log.height;
				
				// On récupère la largeur du texte à afficher
				pango_layout_set_text (pLayout, pTextZone->cText, -1);
				pango_layout_get_pixel_extents (pLayout, &ink, &log);				
				fCurrentLineWidth += log.width;
				
				if (log.height > fCurrentLineHeight && ! myData.bLastWasSameLine)
					fCurrentLineHeight = log.height;
				else if (myData.bLastWasSameLine)
					fCurrentLineHeight = (double)myData.iLastLineHeight;
				
			}			
			else if (pTextZone->bLimitedBar || pTextZone->bImgDraw)
			{
				fCurrentLineWidth += pTextZone->iWidth;
				
				if ( (pTextZone->iHeight > fCurrentLineHeight) && ! myData.bLastWasSameLine)
					fCurrentLineHeight = (double)pTextZone->iHeight;
				else if (myData.bLastWasSameLine)
					fCurrentLineHeight = (double)myData.iLastLineHeight;
			}
			else if (pTextZone->bBar)
			{				
				if ( (pTextZone->iHeight > fCurrentLineHeight) && ! myData.bLastWasSameLine)
					fCurrentLineHeight = (double)pTextZone->iHeight;
				else if (myData.bLastWasSameLine)
					fCurrentLineHeight = (double)myData.iLastLineHeight;
			}
						
			it = it->next;
		} while (it != NULL && ! pTextZone->bEndOfLine);
		
		
		
		
		//\_______ boucle sur it de it1 jusqu'à retour chariot => DESSIN
		it = it1;
		do
		{
	
			pTextZone = it->data;
			
						
			if (pTextZone->cFont == NULL || pTextZone->cFont =="") // Si aucune font -> on prend celle de la config
				pTextZone->cFont = g_strdup_printf("%s", myConfig.cDefaultFont);
			
			
			 // On calcule le décalage WIDTH nécéssaire pour respecter l'alignement sur la largeur souhaité
			if (bFirstTextInLine)
			{
				int iWidth, iHeight;
				CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
				
				if (strcmp (pTextZone->cAlignWidth, "left") == 0)
					myData.fCurrentX = iMargin + myConfig.iTextMargin;
				else if (strcmp (pTextZone->cAlignWidth, "center") == 0)
				{
					if (pTextZone->cImgPath != NULL)
						myData.fCurrentX = (iWidth/2) - (pTextZone->iWidth/2);
					else
						myData.fCurrentX = (iWidth/2)- (fCurrentLineWidth/2);
				}
				else if (strcmp (pTextZone->cAlignWidth, "right") == 0)
				{
					if (pTextZone->cImgPath != NULL)
						myData.fCurrentX = iWidth - iMargin - myConfig.iTextMargin - pTextZone->iWidth - pTextZone->iFontSizeWidth;
					else
						myData.fCurrentX = iWidth - iMargin - myConfig.iTextMargin - fCurrentLineWidth - pTextZone->iFontSizeWidth;
				}
				
				if (pTextZone->bBar)
					 myData.fCurrentX -= iSpaceBetweenElements ; // Il y a un élément avant la barre -> On supprime l'espace
				
			}
			
			
			if (! bFirstTextInLine)
				pTextZone->cAlignHeight = g_strdup_printf("%s", myData.cLastAlignHeight); // On récupère l'alignement de la ligne (=l'alignement du 1er élément)
			else
				myData.cLastAlignHeight = g_strdup_printf("%s", pTextZone->cAlignHeight); // On mémorise l'aligenement du 1er élément de la ligne
				
			// On calcule le décalage HEIGHT nécéssaire pour respecter l'alignement sur la hauteur souhaité		
			if (strcmp (pTextZone->cAlignHeight, "middle") == 0)
			{
				if (pTextZone->cImgPath != NULL || pTextZone->bBar || pTextZone->bLimitedBar )
					myData.fCurrentYalign = myData.fCurrentY + fCurrentLineHeight/2 - pTextZone->iHeight/2; 
				else // C'est un texte -> On décale un peu pour aligner
					myData.fCurrentYalign = myData.fCurrentY  + fCurrentLineHeight/2 - pTextZone->iFontSizeHeight/2;
			}			
			else if (strcmp (pTextZone->cAlignHeight, "low") == 0)
			{
				if (pTextZone->cImgPath != NULL || pTextZone->bBar || pTextZone->bLimitedBar )
					myData.fCurrentYalign = myData.fCurrentY + fCurrentLineHeight - pTextZone->iHeight;
				else // C'est un texte -> On décale un peu pour aligner
					myData.fCurrentYalign = myData.fCurrentY + fCurrentLineHeight - pTextZone->iFontSizeHeight;
			}
			else if (strcmp (pTextZone->cAlignHeight, "top") == 0)
			{
				if (pTextZone->cImgPath != NULL || pTextZone->bBar || pTextZone->bLimitedBar )
					myData.fCurrentYalign = myData.fCurrentY; // On laisse à la position courante
				else // C'est un texte -> On décale un peu pour aligner
					myData.fCurrentYalign = myData.fCurrentY;
			}
			
			
			PangoFontDescription *fd = pango_font_description_from_string (pTextZone->cFont);
			pango_layout_set_font_description (pLayout, fd);
			pango_font_description_free (fd);
			
			
			if (pTextZone->fTextColor[3]== 0 && pTextZone->cText != NULL) // Si aucune couleur alors qu'un texte est défini -> on prend celle de la config
				cairo_set_source_rgba (myDrawContext, myConfig.fDefaultTextColor[0], myConfig.fDefaultTextColor[1], myConfig.fDefaultTextColor[2], myConfig.fDefaultTextColor[3]);
			else // sinon, on utilise la couleur du .xml
				cairo_set_source_rgba (myDrawContext, pTextZone->fTextColor[0], pTextZone->fTextColor[1], pTextZone->fTextColor[2], pTextZone->fTextColor[3]);
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			if (pTextZone->cText != NULL)
				pango_layout_set_text (pLayout, pTextZone->cText, -1);
			else
				pango_layout_set_text (pLayout,"", -1);
			pango_layout_get_pixel_extents (pLayout, &ink, &log);





			
			
			
			if (pTextZone->bBar || pTextZone->bLimitedBar) // On dessine la barre
			{
				int value;				
				
				if (pTextZone->bBar)
				{
					int iWidth, iHeight;
					CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
					pTextZone->iWidth =  iWidth - iMargin*2 - myConfig.iTextMargin - (myData.fCurrentX + iSpaceBetweenElements);
				}
				
				int i;
				for (i=0 ; i < 2 ; i++)
				{
					if (i==0)
					{
						if (pTextZone->bStroke)
							value = pTextZone->iWidth; // Une "stroke" (=trait) est une barre toujours pleine ;-) 
						else
						{
							myData.cCurrentText = g_strdup_printf ("%s",pTextZone->cText); // Sinon, çà plante à la ligne d'en dessous
							value = (int)((atof(myData.cCurrentText) / 100)*pTextZone->iWidth);
						}					
					}
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
					
					
					if (pTextZone->bBar)
						cairo_translate (myDrawContext,
							myData.fCurrentX + iSpaceBetweenElements + pTextZone->iOverrideW,
							myData.fCurrentYalign + pTextZone->iOverrideH);
					else
						cairo_translate (myDrawContext,
							myData.fCurrentX + pTextZone->iOverrideW,
							myData.fCurrentYalign + pTextZone->iOverrideH);
					
					
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
							myData.fCurrentX + pTextZone->iOverrideW,
							myData.fCurrentYalign + pTextZone->iOverrideH);
					cairo_paint (myDrawContext);
				}			
			}
			else // C'est un texte !
			{
				cairo_move_to (myDrawContext,
					myData.fCurrentX + pTextZone->iOverrideW,
					myData.fCurrentYalign + pTextZone->iOverrideH);
				pango_cairo_show_layout (myDrawContext, pLayout); // On dessine la ligne sur notre desklet
			}
				
			if (pTextZone->bEndOfLine) // On passe à la ligne du dessous
			{
				if (pTextZone->bNextNewLine) // C'est un <br/> (ou un <br>0</br> )-> On passe à la ligne d'en dessous
				{
					myData.fCurrentY += fCurrentLineHeight + myConfig.iSpaceBetweenLines + pTextZone->iSpaceBetweenLines;
					myData.bLastWasSameLine = FALSE;
				}
				else  // C'est un <nbr/> -> on reste sur la même ligne -> Utile pour avoir 1 zone alignée à gauche et 1 zone alignée à droite par exemple ;-)
				{
					myData.iLastLineHeight = (int)fCurrentLineHeight + pTextZone->iSpaceBetweenLines;					
					myData.bLastWasSameLine = TRUE;
				}
												
				myData.fCurrentX = iMargin + myConfig.iTextMargin;
				
				bFirstTextInLine = TRUE;
				fCurrentLineWidth = 0; // On remet la largeur de ligne à 0 pour le prochain calcul
				fCurrentLineHeight = 0; // On remet la hauteur de ligne à 0 pour le prochain calcul
			}
			else // On laisse le curseur à la position actuelle
			{
				if (pTextZone->bBar || pTextZone->bLimitedBar || pTextZone->bImgDraw)  // On prend en compte un éventuelle barre ou une image
					myData.fCurrentX += pTextZone->iWidth;				
				else  // valable pour les textes
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

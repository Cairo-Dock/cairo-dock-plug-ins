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
#include "applet-rame.h"


void rtrim( gchar* str, const gchar* t )  // Couper tout depuis la droite
{
	/* gchar* curEnd = str, *end = str;
	gchar look[ 256 ] = { 1, 0 };
	while( *t )
		look[ (unsigned char)*t++ ] = 1;
	while( *end )
	{
		if ( !look[ *end ] )
			curEnd = end + 1;
		++end;
	}
	*curEnd = '\0';*/
	gint iStrLength = strlen (str);
	gint iTLength = strlen (t);
	if (iStrLength < iTLength)
		return;

	gchar *cPosition = str + iStrLength - iTLength;
	if (g_strcmp0 (cPosition, t) == 0)
		cPosition[0] = '\0';
}

void ltrim( gchar* str, const gchar* t )  // Couper tout depuis la gauche
{
	g_strreverse (str);
	gchar* cSearch = g_strdup (t);
	g_strreverse (cSearch);
	rtrim( str, cSearch);
	g_strreverse (str);	
	g_free (cSearch);
} 

/*
gchar *g_str_replace (const gchar *cString, const gchar *cWord, const gchar *cReplace)
{
	gchar *cPart1;
	gchar *cPart2;
	gchar *cWordTemp;
	if (g_strstr_len (cString, -1, cWord) != NULL) // On remplace
	{
		gchar *cFinalString = g_strdup (cString);
		while (g_strstr_len (cFinalString, -1, cWord) != NULL)
		{
			cPart1 = g_strdup (cFinalString);
			cWordTemp = g_strdup (cWord);
			g_strreverse (cPart1);
			g_strreverse (cWordTemp);
			cPart1 = strstr(cPart1, cWordTemp) ;
			ltrim( cPart1, cWordTemp );
			g_strreverse (cPart1);
			cPart2 = g_strdup (cFinalString);
			
			while (g_strstr_len (cPart2, -1, cWord) != NULL)
			{
				cPart2 = strstr(cPart2, cWord);				
				ltrim( cPart2, cWord );
			}
			// On colle le texte au milieu
			cFinalString = g_strdup_printf ("%s%s%s", cPart1,  g_strdup_printf("%s",cReplace), cPart2);
			
		}
		return g_strdup (cFinalString);
		g_free (cFinalString);
	}
	else
		return g_strdup_printf("%s",cString); // Remplacement non trouvé -> On retourne la phrase d'origine
	g_free (cPart1);
	g_free (cPart2);
	g_free (cWordTemp);
}*/

gchar *g_str_replace (gchar *cString, const gchar cWord, const gchar cReplace)
{
	if (! cString)
		return NULL;

	for (int i = 0; cString[i] != '\0'; i++)
	{
		if (cString[i] == cWord)
			cString[i] = cReplace;
	}
	return cString;
}

void cd_doncky_get_color_from_xml (gchar *cNodeContent, double *fColor)
{
	int iText1, iText2, iText3, iText4;
	sscanf (cNodeContent, "%d;%d;%d;%d", &iText1, &iText2, &iText3, &iText4);
	
	// On récupère le 1er champ -> red
	fColor[0] = iText1 / 255;
	// On récupère le 2ème champ -> = green
	fColor[1] = iText2 / 255;
	// On récupère le 3ème champ -> = blue
	fColor[2] = iText3 / 255;
	// On récupère le dernier champ -> alpha
	fColor[3] = iText4 / 255;
}

void cd_doncky_export_color_to_conf (double *fColor, const gchar *cGroupName, const gchar *cParam, CairoDockModuleInstance *myApplet)
{
	gchar *cStringInConfig = NULL;
	cStringInConfig = g_strdup_printf ("%f;%f;%f;%f;", fColor[0], fColor[1], fColor[2], fColor[3]);
	g_str_replace (cStringInConfig, ',', '.');
	cd_debug ("DONCKY-debug : %s=%s", cParam, cStringInConfig); 

	cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, cGroupName, cParam, cStringInConfig, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
	g_free (cStringInConfig);
}

/*
gchar *g_str_position (const gchar *cString, const int iPosition, const char cSeparator)
{
	gchar *strSeparator = g_strdup_printf("%c", cSeparator);
	
	if ((g_strstr_len (cString, -1, strSeparator) != NULL) && (iPosition > 0)) // Separator trouvé -> On coupe
	{
		gchar *cFinalString = g_strdup (cString);
		
		if (iPosition == 1)
		{			
			g_strreverse (cFinalString);
			cFinalString = strrchr(cFinalString, cSeparator);
			ltrim( cFinalString, strSeparator);
			g_strreverse (cFinalString);
		}
		else
		{
			int i = 1;
			do
			{
				if (g_strstr_len (cFinalString, -1, strSeparator) == NULL) // Il n'y a plus de séparateur -> Mauvais champ spécifié
				{
					cd_debug ("DONCKY-debug :  /!\\ Missing values detected. Search for the string '%s' to fix it.", cString);
					// On retourne 0 pour le champ non trouvé :
					cFinalString = g_strdup_printf("0");
					i = iPosition;
				}
				else
				{
					cFinalString = strchr(cFinalString, cSeparator);
					ltrim( cFinalString, strSeparator);
					i++;
				}
			}
			while (i != iPosition);
			
			if ((g_strstr_len (cFinalString, -1, strSeparator) != NULL) && (iPosition > 0)) // Il reste des séparateurs -> On coupe les derniers champs
			{
				g_strreverse (cFinalString);
				cFinalString = strrchr(cFinalString, cSeparator);
				ltrim( cFinalString, strSeparator);
				g_strreverse (cFinalString);
			}
		}
		return g_strdup (cFinalString);
		g_free (cFinalString);
		g_free (strSeparator);
	
	}
	else
		return g_strdup_printf("%s",cString); // Pas de séparateur -> On retourne la phrase d'origine
}*/


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


void cd_launch_command (CairoDockModuleInstance *myApplet)
{
	
	// SYSTEM-MONITOR
	myData.bNeedsUpdate = FALSE;	
	if (myConfig.bShowCpu)
		cd_sysmonitor_get_cpu_data (myApplet);
	if (myConfig.bShowRam || myConfig.bShowSwap)
		cd_sysmonitor_get_ram_data (myApplet);
	if (myConfig.bShowNvidia)
	{
		if ((myData.iTimerCount % 3) == 0)  // la temperature ne varie pas tres vite et le script nvidia-settings est lourd, on decide donc de ne mettre a jour qu'une fois sur 3.
			cd_sysmonitor_get_nvidia_data (myApplet);
	}	
	if (! myData.bInitialized)
	{
		cd_sysmonitor_get_cpu_info (myApplet);
		myData.bInitialized = TRUE;
	}
	myData.iTimerCount ++;
	cd_debug ("=== Refresh");
	
	
	// Autre :
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

				cairo_surface_destroy (pTextZone->pImgSurface);
				pTextZone->pImgSurface = cairo_dock_create_surface_from_image (pTextZone->cImgPath,
					1.,
					pTextZone->iImgSize, pTextZone->iImgSize,
					iLoadingModifier,
					&fImgW, &fImgH,
					NULL, NULL);
				
				//\_______________ On garde l'aire de la surface/texture.			
				pTextZone->iWidth = (int)fImgW;
				pTextZone->iHeight = (int)fImgH;
			}
			cairo_surface_destroy (pTextZone->pImgSurface);
			pTextZone->pImgSurface = cairo_dock_create_surface_for_icon (pTextZone->cImgPath,
					pTextZone->iWidth, pTextZone->iHeight);
					
			pTextZone->bImgDraw = TRUE; // L'image est désormais chargée -> On peut la dessiner
		}
		
		
		if (pTextZone->iRefresh != 0)
				pTextZone->iTimer++;
		
		if (pTextZone->bRefresh)
		{
			g_free (pTextZone->cResult);
			if (pTextZone->bIsBash) // C'est une commande bash !
				pTextZone->cResult = cairo_dock_launch_command_sync (pTextZone->cCommand);
			else if (pTextZone->bIsInternal)// C'est une commande interne !
			{
				if (strcmp (pTextZone->cCommand, "cpuperc") == 0)
				{
					if (myData.fCpuPercent < 10)
						pTextZone->cResult = g_strdup_printf ("%.1f", myData.fCpuPercent);
					else
						pTextZone->cResult = g_strdup_printf ("%.0f", myData.fCpuPercent);
				}
				
				else if (strcmp (pTextZone->cCommand, "cpuperc2f") == 0) // Restreint à 2 chiffres : de 00 à 99 !
				{
					if (myData.fCpuPercent < 10)
						pTextZone->cResult = g_strdup_printf ("0%.0f", myData.fCpuPercent);
					else if (myData.fCpuPercent == 100)
						pTextZone->cResult = g_strdup_printf ("99");
					else
						pTextZone->cResult = g_strdup_printf ("%.0f", myData.fCpuPercent);
				}
					
				else if (strcmp (pTextZone->cCommand, "memperc") == 0)
				{
					if (myData.fRamPercent < 10)
						pTextZone->cResult = g_strdup_printf ("0%.0f", myData.fRamPercent);
					else if (myData.fRamPercent == 100)
						pTextZone->cResult = g_strdup  ("99");
					else
						pTextZone->cResult = g_strdup_printf ("%.0f", myData.fRamPercent);
				}
				
				else if (strcmp (pTextZone->cCommand, "memperc2f") == 0) // Restreint à 2 chiffres : de 00 à 99 !
				{
					if (myData.fRamPercent < 10)
						pTextZone->cResult = g_strdup_printf ("%.1f", myData.fRamPercent);
					else
						pTextZone->cResult = g_strdup_printf ("%.0f", myData.fRamPercent);
				}
										
				else if (strcmp (pTextZone->cCommand, "mem") == 0) // en Mo
				{
					pTextZone->cResult = g_strdup_printf ("%.0f", _Ko_to_Mo(myApplet, myData.ramTotal /100. * myData.fRamPercent));
				}
				
				else if (strcmp (pTextZone->cCommand, "memg") == 0) // Identique à mem mais en Go
				{
					pTextZone->cResult = g_strdup_printf ("%.2f", _Ko_to_Go(myApplet, myData.ramTotal /100. * myData.fRamPercent));
				}

				else if (strcmp (pTextZone->cCommand, "memmax") == 0) // en Mo
				{
					pTextZone->cResult = g_strdup_printf ("%.0f", _Ko_to_Mo(myApplet, myData.ramTotal));
				}
				
				else if (strcmp (pTextZone->cCommand, "memmaxg") == 0) // Identique à memmax mais en Go
				{
					pTextZone->cResult = g_strdup_printf ("%.2f", _Ko_to_Go(myApplet, myData.ramTotal));
				}
								
				else if (strcmp (pTextZone->cCommand, "swapperc") == 0)
				{
					if (myData.fSwapPercent < 10)
						pTextZone->cResult = g_strdup_printf ("%.1f", myData.fSwapPercent);
					else
						pTextZone->cResult = g_strdup_printf ("%.0f", myData.fSwapPercent);				
				}
				
				else if (strcmp (pTextZone->cCommand, "swapperc2f") == 0) // Restreint à 2 chiffres : de 00 à 99 !
				{
					if (myData.fSwapPercent < 10)
						pTextZone->cResult = g_strdup_printf ("0%.0f", myData.fSwapPercent);
					else if (myData.fSwapPercent == 100)
						pTextZone->cResult = g_strdup ("99");
					else
						pTextZone->cResult = g_strdup_printf ("%.0f", myData.fSwapPercent);
				}
				
				else if (strcmp (pTextZone->cCommand, "swap") == 0) // en Mo
				{
					double fTmp = _Ko_to_Mo(myApplet, myData.swapUsed);
					
					if (fTmp < 10)
						pTextZone->cResult = g_strdup_printf ("%.2f", fTmp);
					else if (fTmp < 100)
						pTextZone->cResult = g_strdup_printf ("%.1f", fTmp);
					else
						pTextZone->cResult = g_strdup_printf ("%.0f", fTmp);
				}
				
				else if (strcmp (pTextZone->cCommand, "swapg") == 0) // Identique à swap mais en Go
				{
					pTextZone->cResult = g_strdup_printf ("%.2f", _Ko_to_Go(myApplet, myData.swapUsed));
				}
				
				else if (strcmp (pTextZone->cCommand, "swapmax") == 0) // en Mo
				{
					pTextZone->cResult = g_strdup_printf ("%.0f", _Ko_to_Mo(myApplet, myData.swapTotal));
				}
				
				else if (strcmp (pTextZone->cCommand, "swapmaxg") == 0) // Identique à swapmax mais en Go
				{
					pTextZone->cResult = g_strdup_printf ("%.2f", _Ko_to_Go(myApplet, myData.swapTotal));
				}
				
				else if (strcmp (pTextZone->cCommand, "nvtemp") == 0)
				{
					myConfig.bShowNvidia = TRUE;
					pTextZone->cResult = g_strdup_printf ("%i", myData.iGPUTemp);
				}
					
				else if (strcmp (pTextZone->cCommand, "nvname") == 0)
				{
					myConfig.bShowNvidia = TRUE;
					cd_sysmonitor_get_nvidia_info (myApplet);					
					pTextZone->cResult = g_strdup (myData.cGPUName);					
				}
				
				else if (strcmp (pTextZone->cCommand, "nvdriver") == 0)
				{
					myConfig.bShowNvidia = TRUE;
					cd_sysmonitor_get_nvidia_info (myApplet);					
					pTextZone->cResult = g_strdup (myData.cDriverVersion);					
				}
				
				else if (strcmp (pTextZone->cCommand, "nvram") == 0)
				{
					myConfig.bShowNvidia = TRUE;
					cd_sysmonitor_get_nvidia_info (myApplet);					
					pTextZone->cResult = g_strdup_printf ("%i", myData.iVideoRam);					
				}
				
				else if (strcmp (pTextZone->cCommand, "uptime") == 0)
				{
					gchar *cUpTime = NULL, *cActivityTime = NULL;
					cd_sysmonitor_get_uptime (&cUpTime, &cActivityTime);		
					pTextZone->cResult = cUpTime;
					g_free (cActivityTime);
				}
				
				else if (strcmp (pTextZone->cCommand, "fs_size") == 0)
				{
					if (strcmp (pTextZone->cMountPoint, "") != 0)
						pTextZone->cResult = cd_doncky_get_disk_info (pTextZone->cMountPoint, 0);
				}
				
				else if (strcmp (pTextZone->cCommand, "fs_free") == 0)
					pTextZone->cResult = cd_doncky_get_disk_info (pTextZone->cMountPoint, 1);
					
				else if (strcmp (pTextZone->cCommand, "fs_used") == 0)
					pTextZone->cResult = cd_doncky_get_disk_info (pTextZone->cMountPoint, 2);
				
				else if (strcmp (pTextZone->cCommand, "fs_freeperc") == 0)
					pTextZone->cResult = cd_doncky_get_disk_info (pTextZone->cMountPoint, 3);
				
				else if (strcmp (pTextZone->cCommand, "fs_usedperc") == 0)
				{
					if (pTextZone->cMountPoint == NULL || pTextZone->cMountPoint == '\0')
						pTextZone->cMountPoint = g_strdup ("/");	
					pTextZone->cResult = cd_doncky_get_disk_info (pTextZone->cMountPoint, 4);
				}
				
				else if (strcmp (pTextZone->cCommand, "fs_type") == 0)
					pTextZone->cResult = cd_doncky_get_disk_info (pTextZone->cMountPoint, 5);
				
				else if (strcmp (pTextZone->cCommand, "fs_device") == 0)
					pTextZone->cResult = cd_doncky_get_disk_info (pTextZone->cMountPoint, 6);
			}
			// cd_debug ("===== %s (%d %d) => %s", pTextZone->cCommand, pTextZone->bIsBash, pTextZone->bIsInternal, pTextZone->cResult);
		}
	}	
}

gboolean cd_retrieve_command_result (CairoDockModuleInstance *myApplet)
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
				g_free (pTextZone->cText);
				pTextZone->cText = g_strdup (pTextZone->cResult);
			}
			
			if (pTextZone->iRefresh != 0 && pTextZone->iTimer >= pTextZone->iRefresh)
			{
				pTextZone->bRefresh = TRUE;
				pTextZone->iTimer = 0; // On remet le timer à 0
			}
			else // Pas de refresh de spécifié
			{
				if (pTextZone->cText == NULL || strcmp (pTextZone->cText, "Please wait...") == 0) // La récupération s'est mal passé -> On relance !
				{
					pTextZone->bRefresh = TRUE;
					pTextZone->iTimer = 0; // On remet le timer à 0
					
					cd_debug ("DONCKY-debug : Error with this command =  %s (%d, %d, %s, %s)", pTextZone->cCommand, pTextZone->iRefresh, pTextZone->bRefresh, pTextZone->cText, pTextZone->cResult);
				}	
				else			
					pTextZone->bRefresh = FALSE; // On a récupéré l'info -> On arrête là !
			}
		}
	}
	cd_applet_update_my_icon (myApplet); // Quand tous les textes sont chargés, on peut dessiner
	return TRUE;
}



void cd_applet_draw_my_desklet (CairoDockModuleInstance *myApplet, int iWidth, int iHeight)
{
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
			
			if (pTextZone->cFont == NULL || pTextZone->cFont == '\0') // Si aucune font -> on prend celle de la config
				pTextZone->cFont = g_strdup (myConfig.cDefaultFont);			
						
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
			else if (pTextZone->bLimitedBar || pTextZone->bImgDraw || pTextZone->bBar )
			{
				fCurrentLineWidth += pTextZone->iWidth;
				
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
						
			if (pTextZone->cFont == NULL || pTextZone->cFont == '\0') // Si aucune font -> on prend celle de la config
				pTextZone->cFont = g_strdup (myConfig.cDefaultFont);
			
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
			{
				g_free (pTextZone->cAlignHeight);
				pTextZone->cAlignHeight = g_strdup (myData.cLastAlignHeight); // On récupère l'alignement de la ligne (=l'alignement du 1er élément)
			}
			else
			{
				g_free (myData.cLastAlignHeight);
				myData.cLastAlignHeight = g_strdup (pTextZone->cAlignHeight); // On mémorise l'aligenement du 1er élément de la ligne
			}
			
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
			
			
			if (pTextZone->bBar || pTextZone->bLimitedBar) // On dessine la barre
			{
				int value;
				if (pTextZone->bBar)
				{
					int iWidth, iHeight;
					CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
					if (bFirstTextInLine)
						pTextZone->iWidth =  iWidth - iMargin*2 - myConfig.iTextMargin*2;
					else
						pTextZone->iWidth =  iWidth - myData.fCurrentX - iMargin - myConfig.iTextMargin  - iSpaceBetweenElements;
				}
				
				int i;
				for (i=0 ; i < 2 ; i++)
				{
					if (i==0)
					{
						g_free (myData.cCurrentText);
						myData.cCurrentText = g_strdup (pTextZone->cText); // Sinon, çà plante à la ligne d'en dessous
						value = (int)((atof(myData.cCurrentText) / 100)*pTextZone->iWidth);						
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
							myData.fCurrentX + iSpaceBetweenElements + myData.iPrevOverrideW,
							myData.fCurrentYalign + myData.iPrevOverrideH);
					else
						cairo_translate (myDrawContext,
							myData.fCurrentX + myData.iPrevOverrideW,
							myData.fCurrentYalign + myData.iPrevOverrideH);
					
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
							myData.fCurrentX + myData.iPrevOverrideW,
							myData.fCurrentYalign + myData.iPrevOverrideH);
					cairo_paint (myDrawContext);
				}			
			}
			else // C'est un texte !
			{
				cairo_move_to (myDrawContext,
					myData.fCurrentX + myData.iPrevOverrideW,
					myData.fCurrentYalign + myData.iPrevOverrideH);
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
			
			
			if (pTextZone->iOverrideH != 0)
				myData.iPrevOverrideH = pTextZone->iOverrideH;
			else
				myData.iPrevOverrideH = 0;
			if (pTextZone->iOverrideW != 0)
				myData.iPrevOverrideW = pTextZone->iOverrideW;
			else
				myData.iPrevOverrideW = 0;
			
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
	// taille de la texture.
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);	
	cd_applet_draw_my_desklet (myApplet, iWidth, iHeight);
	
	CD_APPLET_REDRAW_MY_ICON;
}

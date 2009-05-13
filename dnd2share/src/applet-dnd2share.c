#include <stdlib.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-dnd2share.h"



void cd_dnd2share_check_number_of_stored_pictures (void)
{
	/// A FAIRE : Nettoyer les endroits où la fonction est utilisée !  <- Pour l'instant, on le fait un peu partout :-D
	
	gchar *cPicFile;
	gboolean bContinueSearch = TRUE;
	
	
	myData.iNumberOfStoredPic = 0;
	
	//~ while (myData.iNumberOfStoredPic < myConfig.iNbItems && bContinueSearch)
	while (bContinueSearch)
	{
		cPicFile = g_strdup_printf ("%s/%i.preview", myData.cWorkingDirPath, myData.iNumberOfStoredPic+1);
		if (g_file_test (cPicFile, G_FILE_TEST_EXISTS))
		{
			myData.iNumberOfStoredPic++;
			bContinueSearch = TRUE;
		}
		else
			bContinueSearch = FALSE;
	}
	g_free (cPicFile);
	cd_debug("DND2SHARE : Nombre d'images stockées : %i",myData.iNumberOfStoredPic);	
}


void cd_dnd2share_extract_urls_from_log (void)
{
	// On récupère toutes les infos dans le fichier de log :
	//D'abord l'url de DisplayImage
	/// A FAIRE : se passer des g_spawn_command_line_sync !
	gchar *cCommandDisplayImage = g_strdup_printf ("grep -oEm 1 '\\[url\\=([^]]*)' %s", myData.cCurrentLogFile);
	myData.cDisplayImage = NULL;
	g_spawn_command_line_sync (cCommandDisplayImage, &myData.cDisplayImage,  NULL, NULL, NULL);
	myData.cDisplayImage = strchr(myData.cDisplayImage, 'h'); // On retire tout ce qui se trouve avant http://
	myData.cDisplayImage[strlen(myData.cDisplayImage) - 1] = '\0';  // on retire le \n à la fin
	cd_debug ("DND2SHARE : Display Image = %s", myData.cDisplayImage);
	g_free (cCommandDisplayImage);
	// Puis l'url de DirectLink
	gchar *cCommandDirectLink = g_strdup_printf ("grep -oEm 1 '\\[img\\]([^[]*)' %s", myData.cCurrentLogFile);
	myData.cDirectLink = NULL;
	g_spawn_command_line_sync (cCommandDirectLink, &myData.cDirectLink,  NULL, NULL, NULL);
	myData.cDirectLink = strchr(myData.cDirectLink, 'h'); // On retire tout ce qui se trouve avant http://
	myData.cDirectLink[strlen(myData.cDirectLink) - 1] = '\0';  // on retire le \n à la fin
	cd_debug ("DND2SHARE : Direct Link = %s", myData.cDirectLink);
	g_free (cCommandDirectLink);
	// Puis on créé les autres URLs à la mano ;-) :
	myData.cBBCodeFullPic = g_strdup_printf ("[url=%s][img]%s[/img][/url]", myData.cDisplayImage, myData.cDirectLink);
	gchar *cDirectLinkWithoutExt;
	cDirectLinkWithoutExt = g_strdup_printf ("%s", myData.cDisplayImage); // On copie "myData.cDisplayImage" dans "cDirectLinkWithoutExt"
	cDirectLinkWithoutExt[strlen(cDirectLinkWithoutExt) - 5] = '\0';  // on retire le .html\0 à la fin
	cd_debug ("DND2SHARE : BBCODE_Full = '%s'", myData.cBBCodeFullPic);
	myData.cBBCode150px = g_strdup_printf ("[url=%s][img]%st.jpg[/img][/url]", myData.cDisplayImage, cDirectLinkWithoutExt);
	myData.cBBCode600px = g_strdup_printf ("[url=%s][img]%stt.jpg[/img][/url]", myData.cDisplayImage, cDirectLinkWithoutExt);
	g_free (cDirectLinkWithoutExt);
	cd_debug ("DND2SHARE : BBCODE_150px = '%s'", myData.cBBCode150px);
	cd_debug ("DND2SHARE : BBCODE_600px = '%s'", myData.cBBCode600px);
	
	/// A FAIRE : Controler la présence des bbcodes .... car ils peuvent ne pas exister en fonction de la taille de l'image

	// On remplit le nouveau fichier de conf :
	FILE* fichier = NULL;
	fichier = fopen(myData.cCurrentConfigFile, "w+");
	if (fichier != NULL)
	{
		fprintf(fichier, "[URLS]\nDisplayImage = %s\nDirectLink = %s\nBBCodeFull = [url=%s][img]%s[/img][/url]\nBBCode150px = %s\nBBCode600px = %s\n",
				myData.cDisplayImage,
				myData.cDirectLink,
				myData.cDisplayImage, myData.cDirectLink,
				myData.cBBCode150px,
				myData.cBBCode600px);
		fclose(fichier);
		remove(myData.cCurrentLogFile);  // On efface l'ancien fichier log
	}
	else
	{
		cd_debug ("DND2SHARE : Erreur dans la création du fichier %s", myData.cCurrentConfigFile);
	}

	cd_dnd2share_check_number_of_stored_pictures ();

}

void cd_dnd2share_get_urls_from_stored_file (void)
{	
	if (g_file_test (myData.cCurrentConfigFile, G_FILE_TEST_EXISTS))
	{
		GError *erreur = NULL;
		GKeyFile *pKeyFile = cairo_dock_open_key_file (myData.cCurrentConfigFile);
			
		
		if (pKeyFile != NULL)
		{
			myData.cDisplayImage = g_key_file_get_string (pKeyFile, "URLS", "DisplayImage", &erreur);
			if (erreur != NULL)
			{
				cd_warning (erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
			
			myData.cDirectLink = g_key_file_get_string (pKeyFile, "URLS", "DirectLink", &erreur);
			if (erreur != NULL)
			{
				cd_warning (erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
			
			myData.cBBCodeFullPic = g_key_file_get_string (pKeyFile, "URLS", "BBCodeFull", &erreur);
			if (erreur != NULL)
			{
				cd_warning (erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
			
			myData.cBBCode150px = g_key_file_get_string (pKeyFile, "URLS", "BBCode150px", &erreur);
			if (erreur != NULL)
			{
				cd_warning (erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
			
			myData.cBBCode600px = g_key_file_get_string (pKeyFile, "URLS", "BBCode600px", &erreur);
			if (erreur != NULL)
			{
				cd_warning (erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
			
			g_key_file_free (pKeyFile);
						
			myData.cCurrentPicturePath = g_strdup_printf ("%s/%i.preview",myData.cWorkingDirPath, myData.iCurrentPictureNumber);
			cd_debug ("DND2SHARE : Image '%s/%i.preview'",myData.cWorkingDirPath, myData.iCurrentPictureNumber);		
		}
	}
	
}


void cd_dnd2share_new_picture (gchar *cDroppedPicturePath)
{
	gint iTailleMaxImage = 2000000;  // Taille limite en octets autorisée pour l'envoi d'image
	gint iTailleImage;
	gchar *cOutput;
	
	g_free (myData.cCurrentPicturePath);
		
	GString *command_check_size = g_string_new ("");
	g_string_printf (command_check_size, "stat -c%%s \"%s\"", cDroppedPicturePath);
	g_spawn_command_line_sync (command_check_size->str, &cOutput,  NULL, NULL, NULL);
	iTailleImage = atoi(cOutput);
	cd_debug ("DND2SHARE : taille de l'image = %i octets",iTailleImage);
	
	if (iTailleImage < iTailleMaxImage)  // L'image est inférieure au 2Mo Maxi -> On peut continuer :-)
	{
		
		cd_dnd2share_check_number_of_stored_pictures ();
		/// A FAIRE :  On décale tout :
		// On décale toutes les images stockées si on dépasse le nombre d'images demandées :	
		if (myData.iNumberOfStoredPic == myConfig.iNbItems && myConfig.bEnableHistoryLimit)
		{
			gint iSourcePictureNumber = 2; 
			gchar *cSourcePicturePath;
			gchar *cSourceLogFile;
			gchar *cSourceConfigFile;
			
			gint iDestPictureNumber = 1; 
			gchar *cDestPicturePath;
			gchar *cDestLogFile;
			gchar *cDestConfigFile;
					
			gint i;
			
			for (i=0 ; i< myData.iNumberOfStoredPic; i++)
			{
				cSourcePicturePath = g_strdup_printf ("%s/%i.preview", myData.cWorkingDirPath, iSourcePictureNumber);
				cSourceLogFile = g_strdup_printf ("%s/%i.log", myData.cWorkingDirPath, iSourcePictureNumber);
				cSourceConfigFile = g_strdup_printf ("%s/%i.conf",myData.cWorkingDirPath, iSourcePictureNumber);
				
				cDestPicturePath = g_strdup_printf ("%s/%i.preview", myData.cWorkingDirPath, iDestPictureNumber);
				cDestLogFile = g_strdup_printf ("%s/%i.log", myData.cWorkingDirPath, iDestPictureNumber);
				cDestConfigFile = g_strdup_printf ("%s/%i.conf",myData.cWorkingDirPath, iDestPictureNumber);
				
				remove(cDestPicturePath);
				remove(cDestLogFile);
				remove(cDestConfigFile);
							
				rename(cSourcePicturePath, cDestPicturePath);
				rename(cSourceLogFile, cDestLogFile);
				rename(cSourceConfigFile, cDestConfigFile);
				
				iSourcePictureNumber = iSourcePictureNumber + 1;
				iDestPictureNumber = iDestPictureNumber + 1;
			}
			g_free (cSourcePicturePath);
			g_free (cSourceLogFile);
			g_free (cSourceConfigFile);
			g_free (cDestPicturePath);
			g_free (cDestLogFile);
			g_free (cDestConfigFile);	
		}
		
		cd_dnd2share_check_number_of_stored_pictures ();
		myData.iCurrentPictureNumber = myData.iNumberOfStoredPic + 1;
		myData.cCurrentLogFile = g_strdup_printf ("%s/%i.log", myData.cWorkingDirPath, myData.iCurrentPictureNumber);
		myData.cCurrentPicturePath = g_strdup_printf ("%s/%i.preview", myData.cWorkingDirPath, myData.iCurrentPictureNumber);
		myData.cCurrentConfigFile = g_strdup_printf ("%s/%i.conf",myData.cWorkingDirPath, myData.iCurrentPictureNumber);
				
		cd_debug ("DND2SHARE : Fichier de log -> %s", myData.cCurrentLogFile);	
		
		// On envoie notre fichier :
		GString *command_upload = g_string_new ("");
		g_string_printf (command_upload, "curl uppix.net -F myimage=@%s -F submit=Upload -F formup=1 -o %s", cDroppedPicturePath, myData.cCurrentLogFile);
		g_spawn_command_line_async (command_upload->str, NULL);
		/// à voir comment rajouter un équivalent à > /dev/null 2>&1 
		g_string_free (command_upload, TRUE);
		
		
		// On affecte l'image à notre icone et on redraw :
		CD_APPLET_SET_IMAGE_ON_MY_ICON (cDroppedPicturePath);
		CD_APPLET_REDRAW_MY_ICON;
		
			
		// On affiche une info-bulle quand tout est terminé :	
		if (myConfig.bEnableDialogs)
		{
			cairo_dock_remove_dialog_if_any (myIcon);
			cairo_dock_show_temporary_dialog ("%s\n%s",
				myIcon,
				myContainer,
				myConfig.dTimeDialogs,
				D_("Picture has been uploaded."),
				D_("Press 'Left mouse button' to retrieve the urls"));
		}
				
		// On copie l'image dans myData.cWorkingDirPath :
		/// A FAIRE : Copier une miniature au lieu de l'original ;-)
		gchar *cCommandCopyPicture = g_strdup_printf ("cp %s %s", cDroppedPicturePath, myData.cCurrentPicturePath);
		g_spawn_command_line_async (cCommandCopyPicture, NULL);
		g_free (cCommandCopyPicture);
	}
	else  // L'image est trop volumineuse 
	{
		cd_debug ("DND2SHARE : Désolé, l'image dépasse le quota de 2Mo imposé par Uppix.net");
		
		if (myConfig.bEnableDialogs)
		{
			cairo_dock_remove_dialog_if_any (myIcon);
			cairo_dock_show_temporary_dialog ("%s\n%s",
				myIcon,
				myContainer,
				myConfig.dTimeDialogs,
				D_("Sorry, the picture exceeds"),
				D_("the 2Mo Max file size allowed by Uppix.net"));
		}
		
	}
	g_string_free (command_check_size, TRUE);
	g_free (cOutput);
	
	cd_dnd2share_check_number_of_stored_pictures ();	
}

void cd_dnd2share_delete_picture (void)
{
	cd_message ("DND2SHARE : J'efface l'image et les fichiers .log et .conf (si présents)");
		
	remove(myData.cCurrentPicturePath);
	remove(myData.cCurrentLogFile);
	remove(myData.cCurrentConfigFile);
	
	// On décale les fichiers si on a supprimer une image différente de la dernière
	if (myData.iCurrentPictureNumber != myData.iNumberOfStoredPic)
	{
		gint iSourcePictureNumber = myData.iCurrentPictureNumber + 1; 
		gchar *cSourcePicturePath;
		gchar *cSourceLogFile;
		gchar *cSourceConfigFile;
		
		gint iDestPictureNumber = myData.iCurrentPictureNumber; 
		gchar *cDestPicturePath;
		gchar *cDestLogFile;
		gchar *cDestConfigFile;
				
		gint i;
		
		for (i=0 ; i<myData.iNumberOfStoredPic; i++)
		{
			cSourcePicturePath = g_strdup_printf ("%s/%i.preview", myData.cWorkingDirPath, iSourcePictureNumber);
			cSourceLogFile = g_strdup_printf ("%s/%i.log", myData.cWorkingDirPath, iSourcePictureNumber);
			cSourceConfigFile = g_strdup_printf ("%s/%i.conf",myData.cWorkingDirPath, iSourcePictureNumber);
			
			cDestPicturePath = g_strdup_printf ("%s/%i.preview", myData.cWorkingDirPath, iDestPictureNumber);
			cDestLogFile = g_strdup_printf ("%s/%i.log", myData.cWorkingDirPath, iDestPictureNumber);
			cDestConfigFile = g_strdup_printf ("%s/%i.conf",myData.cWorkingDirPath, iDestPictureNumber);
						
			rename(cSourcePicturePath, cDestPicturePath);
			rename(cSourceLogFile, cDestLogFile);
			rename(cSourceConfigFile, cDestConfigFile);
			
			iSourcePictureNumber = iSourcePictureNumber + 1;
			iDestPictureNumber = iDestPictureNumber + 1;
		}
		g_free (cSourcePicturePath);
		g_free (cSourceLogFile);
		g_free (cSourceConfigFile);
		g_free (cDestPicturePath);
		g_free (cDestLogFile);
		g_free (cDestConfigFile);
	}
	else
	{
		// Pas de décalage à faire mais on passe à l'image précédente et on redraw :
		myData.iCurrentPictureNumber = myData.iCurrentPictureNumber - 1;
		myData.cCurrentPicturePath = g_strdup_printf ("%s/%i.preview", myData.cWorkingDirPath, myData.iCurrentPictureNumber);
		myData.cCurrentLogFile = g_strdup_printf ("%s/%i.log", myData.cWorkingDirPath, myData.iCurrentPictureNumber);
		myData.cCurrentConfigFile = g_strdup_printf ("%s/%i.conf",myData.cWorkingDirPath, myData.iCurrentPictureNumber);
	}
	
	
	cd_dnd2share_check_number_of_stored_pictures ();
	
	if (myData.iNumberOfStoredPic == 0)
	{
		CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON (MY_APPLET_ICON_FILE);
		cairo_dock_remove_dialog_if_any (myIcon);
		cairo_dock_show_temporary_dialog ("No stored files\nJust drag'n drop a file on the icon to upload it",
			myIcon,
			myContainer,
			myConfig.dTimeDialogs);
	}
	else 
	{
		if (myData.cCurrentPicturePath != NULL)
		{
			CD_APPLET_SET_IMAGE_ON_MY_ICON (myData.cCurrentPicturePath);
			cairo_dock_remove_dialog_if_any (myIcon);
			cairo_dock_show_temporary_dialog ("Picture %i:\nPress 'Left mouse button' to\ncopy the prefered url\ninto the clipboard",
				myIcon,
				myContainer,
				myConfig.dTimeDialogs,
				myData.iCurrentPictureNumber);
		}
	}
	CD_APPLET_REDRAW_MY_ICON;	
}


void cd_dnd2share_delete_all_pictures (void)
{
	cd_message ("DND2SHARE : J'efface le répertoire complet...");

	// On efface PUIS re-crée le répertoire de travail :
	gchar *cCommandDeleteDir = g_strdup_printf ("rm -rf %s", myData.cWorkingDirPath);
	g_spawn_command_line_async (cCommandDeleteDir, NULL);
	g_free (cCommandDeleteDir);
	gchar *cCommandCreateDir = g_strdup_printf ("mkdir %s", myData.cWorkingDirPath);
	g_spawn_command_line_async (cCommandCreateDir, NULL);
	g_free (cCommandCreateDir);
	
	// et on remet tout à zéro :
	myData.iNumberOfStoredPic = 0;
	myData.iCurrentPictureNumber = 0;
	// puis on redraw :
	CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON (MY_APPLET_ICON_FILE);
	CD_APPLET_REDRAW_MY_ICON;	
}

void cd_dnd2share_copy_url_into_clipboard (gint iUrlNumberInList)
{	
	gchar *cUrlToCopy;
	// On copie notre url dans le clipboard :	
	GtkClipboard *pClipBoard;
	pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
	
	cd_debug ("DND2SHARE : Url à copier dans le clipboard = %i", iUrlNumberInList); 
	
	switch (iUrlNumberInList)
    {
		case (0):
			cUrlToCopy =  g_strdup_printf ("%s", myData.cBBCode150px);
		break;
		case (1):
			cUrlToCopy =  g_strdup_printf ("%s", myData.cBBCode600px);
		break;
		case (2):
			cUrlToCopy =  g_strdup_printf ("%s", myData.cBBCodeFullPic);
		break;
		case (3):
			cUrlToCopy =  g_strdup_printf ("%s", myData.cDisplayImage);
		break;
		case (4):
			cUrlToCopy =  g_strdup_printf ("%s", myData.cDirectLink);
		break;
		default:
			cd_debug ("DND2SHARE : Erreur dans le choix de l'url !");
		break;
    }
    cd_debug ("DND2SHARE : cUrlToCopy = %s",cUrlToCopy);
    gtk_clipboard_set_text (pClipBoard, cUrlToCopy, -1);
    cd_debug ("DND2SHARE : C'est copié !!!");
    g_free (cUrlToCopy);
    cd_debug ("DND2SHARE : cUrlToCopy est libéré !!!");
}

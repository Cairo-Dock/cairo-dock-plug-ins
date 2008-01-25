/**********************************************************************************

This file is a part of the cairo-dock project,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

**********************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cairo-dock.h"

#include "applet-draw.h"
#include "applet-trashes-manager.h"

CD_APPLET_INCLUDE_MY_VARS

extern GList *my_pTrashDirectoryList;
extern gchar *my_cDefaultBrowser;
extern int my_iQuickInfoType;
extern int my_iNbTrashes, my_iNbFiles, my_iSize;
extern int my_iSizeLimit;

static GStaticRWLock s_mTasksMutex = G_STATIC_RW_LOCK_INIT;
static GList *s_pTasksList = NULL;
static int s_iThreadIsRunning = 0;
static int s_iSidTimerRedraw = 0;
static int s_iSidDelayMeasure = 0;

gpointer cd_dustbin_threaded_calculation (gpointer data)
{
	int iNbFiles, iSize;
	do
	{
		//\________________________ On quitte si plus de message.
		g_static_rw_lock_writer_lock (&s_mTasksMutex);
		if (s_pTasksList == NULL)  // aucun message dans la file d'attente, on quitte.
		{
			g_print ("*** plus de message, on quitte le thread.\n");
			g_atomic_int_set (&s_iThreadIsRunning, 0);
			g_static_rw_lock_writer_unlock (&s_mTasksMutex);
			break;
		}
		
		//\________________________ On recupere le message de tete.
		GList *pFirstElement = s_pTasksList;
		CdDustbinMessage *pMessage = pFirstElement->data;
		CdDustbin *pDustbin = pMessage->pDustbin;
		gchar *cURI = pMessage->cURI;
		g_print ("*** recuperation du message : %s\n", cURI);
		
		//\________________________ On l'enleve de la liste.
		s_pTasksList = g_list_remove (s_pTasksList, pMessage);
		/*s_pTasksList = pFirstElement->next;
		if (s_pTasksList != NULL)
			s_pTasksList->prev = NULL;
		g_list_free (pFirstElement);*/
		g_free (pMessage);
		
		g_static_rw_lock_writer_unlock (&s_mTasksMutex);
		
		//\________________________ On traite le message.
		if (pDustbin == NULL)  // recalcul complet.
		{
			cd_dustbin_measure_all_dustbins (&my_iNbFiles, &my_iSize);
		}
		else if (cURI == NULL)
		{
			g_atomic_int_add (&my_iNbFiles, - pDustbin->iNbFiles);
			g_atomic_int_add (&my_iSize, - pDustbin->iSize);
			cd_dustbin_measure_directory (pDustbin->cPath, my_iQuickInfoType, pDustbin, &pDustbin->iNbFiles, &pDustbin->iSize);
			g_atomic_int_add (&my_iNbFiles, pDustbin->iNbFiles);
			g_atomic_int_add (&my_iSize, pDustbin->iSize);
		}
		else  // calcul d'un fichier supplementaire.
		{
			cd_dustbin_measure_one_file (cURI, my_iQuickInfoType, pDustbin, &iNbFiles, &iSize);
			pDustbin->iNbFiles += iNbFiles;
			pDustbin->iSize += iSize;
			g_atomic_int_add (&my_iNbFiles, iNbFiles);
			g_atomic_int_add (&my_iSize, iSize);
		}
		g_free (cURI);
	}
	while (1);
	
	g_print ("*** fin du thread -> %dfichiers , %db\n", my_iNbFiles, my_iSize);
	
	return NULL;
}


void cd_dustbin_free_message (CdDustbinMessage *pMessage)
{
	if (pMessage == NULL)
		return;
	g_free (pMessage->cURI);
	g_free (pMessage);
}

void cd_dustbin_remove_all_messages (void)
{
	g_list_foreach (s_pTasksList, (GFunc) cd_dustbin_free_message, NULL);
	g_list_free (s_pTasksList);
	s_pTasksList = NULL;
}

void cd_dustbin_remove_messages (CdDustbin *pDustbin)
{
	CdDustbinMessage *pMessage;
	GList *pElement, *pNextElement;
	if (s_pTasksList == NULL)
		return ;
	
	pElement = s_pTasksList;
	do
	{
		pNextElement = pElement->next;
		if (pNextElement == NULL)
			break;
		
		pMessage = pNextElement->data;  // on ne peut pas enlever l'element courant, sinon on perd 'pElement'.
		if (pMessage->pDustbin == pDustbin)  // on l'enleve de la liste et on l'efface.
		{
			s_pTasksList = g_list_remove (s_pTasksList, pMessage);
			cd_dustbin_free_message (pMessage);
			/*pElement->next = pNextElement->next;
			if (pNextElement->next != NULL)
				pNextElement->next->prev = pElement;
			g_list_free (pNextElement);*/
		}
		else
		{
			pElement = pNextElement;
		}
	} while (TRUE);
	
	pElement = s_pTasksList;
	pMessage = pElement->data;
	if (pMessage->pDustbin == pDustbin)  // on l'enleve de la liste et on l'efface.
	{
		s_pTasksList = g_list_remove (s_pTasksList, pMessage);
		cd_dustbin_free_message (pMessage);
		/*cd_dustbin_free_message (pMessage);
		s_pTasksList = pElement->next;
		g_list_free (pElement);*/
	}
}


static gboolean _cd_dustbin_check_for_redraw (gpointer data)
{
	int iThreadIsRunning = g_atomic_int_get (&s_iThreadIsRunning);
	g_print ("%s (%d)\n", __func__, iThreadIsRunning);
	if (! iThreadIsRunning)
	{
		s_iSidTimerRedraw = 0;
		g_print ("  redessin (%d,%d)\n", my_iNbFiles, my_iSize);
		if (my_iQuickInfoType == CD_DUSTBIN_INFO_NB_FILES || my_iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT)
			cd_dustbin_draw_quick_info (TRUE);
		cd_dustbin_signal_full_dustbin ();
		return FALSE;
	}
	return TRUE;
}
static void _cd_dustbin_launch_measure (void)
{
	g_print ("%s ()\n", __func__);
	if (g_atomic_int_compare_and_exchange (&s_iThreadIsRunning, 0, 1))  // il etait egal a 0, on lui met 1 et on lance le thread.
	{
		g_print (" ==> lancement du thread de calcul\n");
		if (s_iSidTimerRedraw == 0)
			s_iSidTimerRedraw = g_timeout_add (100, (GSourceFunc) _cd_dustbin_check_for_redraw, (gpointer) NULL);
		
		GError *erreur = NULL;
		GThread* pThread = g_thread_create ((GThreadFunc) cd_dustbin_threaded_calculation,
			NULL,
			FALSE,
			&erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
		}
	}
}
static gboolean _cd_dustbin_launch_measure_delayed (gpointer *data)
{
	_cd_dustbin_launch_measure ();
	s_iSidDelayMeasure = 0;
	return FALSE;
}
void cd_dustbin_add_message (gchar *cURI, CdDustbin *pDustbin)
{
	g_print ("%s (%s)\n", __func__, cURI);
	g_static_rw_lock_writer_lock (&s_mTasksMutex);
	
	CdDustbinMessage *pNewMessage = g_new (CdDustbinMessage, 1);
	pNewMessage->cURI = cURI;
	pNewMessage->pDustbin = pDustbin;
	
	if (pDustbin == NULL)
	{
		cd_dustbin_remove_all_messages ();
		s_pTasksList = g_list_prepend (s_pTasksList, pNewMessage);
		g_atomic_int_set (&my_iNbFiles, -1);  // en cours.
		g_atomic_int_set (&my_iSize, -1);  // en cours.
		cd_dustbin_draw_quick_info (TRUE);
	}
	else if (cURI == NULL)
	{
		cd_dustbin_remove_messages (pDustbin);
		s_pTasksList = g_list_prepend (s_pTasksList, pNewMessage);
	}
	else
	{
		s_pTasksList = g_list_append (s_pTasksList, pNewMessage);
	}
	g_static_rw_lock_writer_unlock (&s_mTasksMutex);
	
	if (! g_atomic_pointer_get (&s_iThreadIsRunning))
	{
		if (s_iSidDelayMeasure != 0)
		{
			g_print ("  lancement calcul retarde\n");
			g_source_remove (s_iSidDelayMeasure);
			s_iSidDelayMeasure = 0;
		}
		s_iSidDelayMeasure = g_timeout_add (400, (GSourceFunc) _cd_dustbin_launch_measure_delayed, NULL);  // on retarde le calcul, car il y'a probablement d'autres fichiers qui vont arriver.
	}
}



int cd_dustbin_count_trashes (gchar *cDirectory)
{
	g_print ("%s (%s)\n", __func__, cDirectory);
	GError *erreur = NULL;
	GDir *dir = g_dir_open (cDirectory, 0, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return 0;
	}
	
	int iNbTrashes = 0;
	struct stat buf;
	const gchar *cFileName;
	GString *sFilePath = g_string_new ("");
	while ((cFileName = g_dir_read_name (dir)) != NULL)
	{
		iNbTrashes ++;
	}
	
	g_string_free (sFilePath, TRUE);
	g_dir_close (dir);
	return (iNbTrashes);
}

void cd_dustbin_measure_directory (gchar *cDirectory, CdDustbinInfotype iInfoType, CdDustbin *pDustbin, int *iNbFiles, int *iSize)
{
	g_print ("%s (%s)\n", __func__, cDirectory);
	g_atomic_int_set (iNbFiles, 0);
	g_atomic_int_set (iSize, 0);

	GError *erreur = NULL;
	GDir *dir = g_dir_open (cDirectory, 0, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	int iNbFilesSubDir, iSizeSubDir;
	struct stat buf;
	const gchar *cFileName;
	CdDustbinMessage *pMessage;
	GString *sFilePath = g_string_new ("");
	while ((cFileName = g_dir_read_name (dir)) != NULL)
	{
		g_static_rw_lock_reader_lock (&s_mTasksMutex);
		if (s_pTasksList != NULL)
		{
			pMessage = s_pTasksList->data;
			if (pMessage->pDustbin == NULL || pMessage->pDustbin == pDustbin)  // une demande de recalcul complet a ete faite sur cette poubelle, on interromp le calcul.
			{
				g_static_rw_lock_reader_unlock (&s_mTasksMutex);
				break ;
			}
		}
		g_static_rw_lock_reader_unlock (&s_mTasksMutex);
		
		g_string_printf (sFilePath, "%s/%s", cDirectory, cFileName);
		if (lstat (sFilePath->str, &buf) != -1)
		{
			if (S_ISDIR (buf.st_mode))
			{
				g_print ("  %s est un repertoire\n", sFilePath->str);
				iNbFilesSubDir = 0;
				iSizeSubDir = 0;
				cd_dustbin_measure_directory (sFilePath->str, iInfoType, pDustbin, &iNbFilesSubDir, &iSizeSubDir);
				g_atomic_int_add (iNbFiles, iNbFilesSubDir);
				g_atomic_int_add (iSize, iSizeSubDir);
				g_print ("  + %d fichiers dans ce sous-repertoire\n", iNbFilesSubDir );
			}
			else
			{
				g_atomic_int_add (iNbFiles, 1);
				g_atomic_int_add (iSize, buf.st_size);
			}
		}
	}
	
	g_string_free (sFilePath, TRUE);
	g_dir_close (dir);
}

void cd_dustbin_measure_one_file (gchar *cURI, CdDustbinInfotype iInfoType, CdDustbin *pDustbin, int *iNbFiles, int *iSize)
{
	g_print ("%s (%s)\n", __func__, cURI);
	
	GError *erreur = NULL;
	gchar *cFilePath = g_filename_from_uri (cURI, NULL, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		g_atomic_int_set (iNbFiles, 0);
		g_atomic_int_set (iSize, 0);
		return ;
	}
	
	struct stat buf;
	if (lstat (cFilePath, &buf) != -1)
	{
		if (S_ISDIR (buf.st_mode))
		{
			cd_dustbin_measure_directory (cFilePath, iInfoType, pDustbin, iNbFiles, iSize);
		}
		else
		{
			g_atomic_int_set (iNbFiles, 1);
			g_atomic_int_set (iSize, buf.st_size);
		}
	}
	else
	{
		g_atomic_int_set (iNbFiles, 0);
		g_atomic_int_set (iSize, 0);
	}
	g_free (cFilePath);
}

void cd_dustbin_measure_all_dustbins (int *iNbFiles, int *iSize)
{
	g_print ("%s ()\n", __func__);
	g_atomic_int_set (iNbFiles, 0);
	g_atomic_int_set (iSize, 0);
	
	int iNbFilesHere, iSizeHere;
	CdDustbin *pDustbin;
	GList *pElement;
	for (pElement = my_pTrashDirectoryList; pElement != NULL; pElement = pElement->next)
	{
		pDustbin = pElement->data;
		
		cd_dustbin_measure_directory (pDustbin->cPath, my_iQuickInfoType, pDustbin, &pDustbin->iNbFiles, &pDustbin->iSize);
		
		g_atomic_int_add (iNbFiles, pDustbin->iNbFiles);
		g_atomic_int_add (iSize, pDustbin->iSize);
	}
}



void cd_dustbin_delete_trash (GtkMenuItem *menu_item, gchar *cDirectory)
{
	gchar *cQuestion;
	if (cDirectory != NULL)
		cQuestion = g_strdup_printf (_D("You're about to delete all files in %s. Sure ?"), cDirectory);
	else if (my_pTrashDirectoryList != NULL)
		cQuestion = g_strdup_printf (_D("You're about to delete all files in all dustbins. Sure ?"));
	else
		return;
	int answer = cairo_dock_ask_question_and_wait (cQuestion, myIcon, myDock);
	g_free (cQuestion);
	if (answer == GTK_RESPONSE_YES)
	{
		GString *sCommand = g_string_new ("rm -rf ");
		if (cDirectory != NULL)
		{
			g_string_append_printf (sCommand, "%s/*", cDirectory);
		}
		else
		{
			CdDustbin *pDustbin;
			GList *pElement;
			for (pElement = my_pTrashDirectoryList; pElement != NULL; pElement = pElement->next)
			{
				pDustbin = pElement->data;
				g_string_append_printf (sCommand, "%s/* ", pDustbin->cPath);
			}
		}
		g_print (">>> %s\n", sCommand->str);
		system (sCommand->str);  // g_spawn_command_line_async() ne marche pas pour celle-la.
		g_string_free (sCommand, TRUE);
	}
}

void cd_dustbin_show_trash (GtkMenuItem *menu_item, gchar *cDirectory)
{
	if (my_cDefaultBrowser != NULL)
	{
		GString *sCommand = g_string_new (my_cDefaultBrowser);
		if (cDirectory != NULL)
		{
			g_string_append_printf (sCommand, " %s", cDirectory);
		}
		else if (my_pTrashDirectoryList != NULL)
		{
			CdDustbin *pDustbin;
			GList *pElement;
			for (pElement = my_pTrashDirectoryList; pElement != NULL; pElement = pElement->next)
			{
				pDustbin = pElement->data;
				g_string_append_printf (sCommand, " %s", pDustbin->cPath);
			}
		}
		else
			return ;
		//g_print (">>> %s\n", sCommand->str);
		GError *erreur = NULL;
		g_spawn_command_line_async (sCommand->str, &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : when trying to execute '%s' : %s\n", sCommand->str, erreur->message);
			g_error_free (erreur);
			//gchar *cTipMessage = g_strdup_printf ("A problem occured\nIf '%s' is not your usual file browser, you can change it in the conf panel of this module", my_cDefaultBrowser);
			cairo_dock_show_temporary_dialog (_D("A problem occured\nIf '%s' is not your usual file browser,\nyou can change it in the conf panel of this module"), myIcon, myDock, 5000, my_cDefaultBrowser);
			//g_free (cTipMessage);
		}
		g_string_free (sCommand, TRUE);
	}
	else
	{
		cairo_dock_fm_launch_uri (cDirectory != NULL ? cDirectory : "trash:/");
	}
}

void cd_dustbin_sum_all_measures (int *iNbFiles, int *iSize)
{
	int iTotalMeasure = 0;
	CdDustbin *pDustbin;
	GList *pElement;
	for (pElement = my_pTrashDirectoryList; pElement != NULL; pElement = pElement->next)
	{
		pDustbin = pElement->data;
		g_atomic_int_add (iNbFiles, pDustbin->iNbFiles);
		g_atomic_int_add (iSize, pDustbin->iSize);
	}
}



gboolean cd_dustbin_is_monitored (gchar *cDustbinPath)
{
	g_return_val_if_fail (cDustbinPath != NULL, FALSE);
	CdDustbin *pDustbin;
	GList *pElement;
	for (pElement = my_pTrashDirectoryList; pElement != NULL; pElement = pElement->next)
	{
		pDustbin = pElement->data;
		if (pDustbin->cPath != NULL && strcmp (pDustbin->cPath, cDustbinPath) == 0)
			return TRUE;
	}
	return FALSE;
}

gboolean cd_dustbin_add_one_dustbin (gchar *cDustbinPath, int iAuthorizedWeight)
{
	g_return_val_if_fail (cDustbinPath != NULL, FALSE);
	g_print ("%s (%s)\n", __func__, cDustbinPath);
	
	CdDustbin *pDustbin = g_new0 (CdDustbin, 1);
	pDustbin->cPath = cDustbinPath;
	pDustbin->iAuthorizedWeight = iAuthorizedWeight;
	my_pTrashDirectoryList = g_list_prepend (my_pTrashDirectoryList, pDustbin);
	
	if (cairo_dock_fm_add_monitor_full (cDustbinPath, TRUE, NULL, (CairoDockFMMonitorCallback) cd_dustbin_on_file_event, pDustbin))
	{
		pDustbin->iNbTrashes = cd_dustbin_count_trashes (cDustbinPath);
		g_atomic_int_add (&my_iNbTrashes, pDustbin->iNbTrashes);
		g_print ("  my_iNbTrashes <- %d\n", my_iNbTrashes);
		return TRUE;
	}
	else
		return FALSE;
}

void cd_dustbin_free_dustbin (CdDustbin *pDustbin)
{
	g_free (pDustbin->cPath);
	g_free (pDustbin);
}

void cd_dustbin_remove_all_dustbins (void)
{
	g_static_rw_lock_writer_lock (&s_mTasksMutex);
	cd_dustbin_remove_all_messages ();
	g_static_rw_lock_writer_unlock (&s_mTasksMutex);  // un g_thread_join() serait peut-etre necessaire.
	
	CdDustbin *pDustbin;
	GList *pElement;
	for (pElement = my_pTrashDirectoryList; pElement != NULL; pElement = pElement->next)
	{
		pDustbin = pElement->data;
		cairo_dock_fm_remove_monitor_full (pDustbin->cPath, FALSE, NULL);
		cd_dustbin_free_dustbin (pDustbin);
	}
	g_list_free (my_pTrashDirectoryList);
	my_pTrashDirectoryList = NULL;
}

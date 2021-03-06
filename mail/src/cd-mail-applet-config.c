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

#include <string.h>
#include <cairo-dock.h>

#include "cd-mail-applet-struct.h"
#include "cd-mail-applet-config.h"
#include "cd-mail-applet-accounts.h"
#include "cd-mail-applet-etpan.h"

typedef void (*cd_mail_fill_account)(CDMailAccount *mailaccount, GKeyFile *pKeyFile, const gchar *mailbox_name);
typedef void (*cd_mail_create_account)( GKeyFile *pKeyFile, const gchar *pMailAccountName );

struct storage_type_def {
	const gchar * name;
	const gchar * description;
	cd_mail_fill_account pfillFunc;
	cd_mail_create_account pcreateFunc;
};

static struct storage_type_def storage_tab[] = {
	{"pop3", "POP3", cd_mail_retrieve_pop3_params, cd_mail_create_pop3_params },
	{"imap", "IMAP", cd_mail_retrieve_imap_params, cd_mail_create_imap_params },
	{"mbox", "MBox", cd_mail_retrieve_mbox_params, cd_mail_create_mbox_params },
	{"mh", "MH", cd_mail_retrieve_mh_params, cd_mail_create_mh_params },
	{"maildir", "MailDir", cd_mail_retrieve_maildir_params , cd_mail_create_maildir_params},
	{"gmail", "GMail", cd_mail_retrieve_gmail_params, cd_mail_create_gmail_params },
	{"yahoo", "Yahoo!", cd_mail_retrieve_yahoo_params, cd_mail_create_yahoo_params },
	{"hotmail", "Hotmail / Live", cd_mail_retrieve_hotmail_params, cd_mail_create_hotmail_params },
	{"free", "Free", cd_mail_retrieve_free_params, cd_mail_create_free_params },
	{"neuf", "Neuf Télécom", cd_mail_retrieve_neuf_params, cd_mail_create_neuf_params },
	{"sfr", "SFR", cd_mail_retrieve_sfr_params, cd_mail_create_sfr_params },
	{"orange", "Orange", cd_mail_retrieve_orange_params, cd_mail_create_orange_params },
	{"skynet", "Skynet (Belgacom)", cd_mail_retrieve_skynet_params, cd_mail_create_skynet_params }
#if ( __WORDSIZE == 64 )
/* in 64bit libetpan crashes with RSS, so... avoid it. */
#warning "Compilation 64bits: avoiding RSS accounts"
#else
	,{"feed", "RSS/Feed", cd_mail_retrieve_feed_params, cd_mail_create_feed_params }
#endif
};

const guint MAIL_NB_STORAGE_TYPES = sizeof(storage_tab) / sizeof(struct storage_type_def);

#define _find_image_path(path) \
	__extension__ ({\
	gchar *_found_image = NULL; \
	if (path != NULL) { \
		_found_image = cairo_dock_search_image_s_path (path); \
		if (_found_image == NULL)\
			_found_image = cairo_dock_search_icon_s_path (path, MAX (myIcon->image.iWidth, myIcon->image.iHeight)); }\
	_found_image; })
static void _get_mail_accounts (GKeyFile *pKeyFile, GldiModuleInstance *myApplet)
{
	//\_______________ On remet a zero les comptes mail.
	if( myData.pMailAccounts )
	{
		guint i;
		for (i = 0; i < myData.pMailAccounts->len; i++)
		{
			CDMailAccount *pMailAccount;
			pMailAccount = g_ptr_array_index (myData.pMailAccounts, i);

			if( pMailAccount != NULL && pMailAccount->pAccountMailTimer != NULL )
				gldi_task_stop (pMailAccount->pAccountMailTimer);
		}
	}
	cd_mail_free_all_accounts (myApplet);
	
	myData.iPrevNbUnreadMails = 0;
	myData.iNbUnreadMails = 0;
	
	//\_______________ On recupere les comptes mail dans le fichier de conf.
	CDMailAccount *pMailAccount;
	gchar *cMailAccountName;
	guint j;
	gsize i, length = 0;
	gboolean bFlushConfFileNeeded = FALSE;
	gchar **pGroupList = g_key_file_get_groups (pKeyFile, &length);
	myData.pMailAccounts = g_ptr_array_sized_new (length - 3);
	
	cd_debug ("recuperons les comptes ...\n");
	for (i = 3; i < length; i ++)  // Icon, Desklet, Configuration + mail groups
	{
		cMailAccountName = pGroupList[i];
		cd_debug ("+ on recupere le compte '%s'", cMailAccountName);
		
		// Get the type of the account.
		if (! g_key_file_has_key (pKeyFile, cMailAccountName, "type", NULL))
			continue ;
		
		gchar *cMailAccountType = g_key_file_get_string (pKeyFile, cMailAccountName, "type", NULL);
		if (cMailAccountType == NULL)
			continue;
		
		for( j = 0; j < MAIL_NB_STORAGE_TYPES; j++ )
		{
			if (g_ascii_strcasecmp (storage_tab[j].name, cMailAccountType) == 0)
			{
				break;
			}
		}
		g_free (cMailAccountType);
		
		// in case the account type is unknown, just ignore.
		if( j >= MAIL_NB_STORAGE_TYPES )
			continue;
		cd_debug ("  mail type : %d", j);
		
		// create a new mail account.
		pMailAccount = g_new0 (CDMailAccount, 1);
		g_ptr_array_add (myData.pMailAccounts, pMailAccount);

		pMailAccount->name = g_strdup (cMailAccountName);
		pMailAccount->timeout = CD_CONFIG_GET_INTEGER_WITH_DEFAULT (cMailAccountName, "timeout mn", 10);
		pMailAccount->pAppletInstance = myApplet;
		pMailAccount->cMailApp = CD_CONFIG_GET_STRING (cMailAccountName, "mail application");  // a specific mail application to launch for this account, if any.
		gchar *cIconName = CD_CONFIG_GET_STRING (cMailAccountName, "icon name");
		pMailAccount->cIconName = _find_image_path (cIconName);
		g_free (cIconName);
		(storage_tab[j].pfillFunc)( pMailAccount, pKeyFile, cMailAccountName );
	}
	g_strfreev (pGroupList);
}

CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	gchar *path;
	path = CD_CONFIG_GET_STRING ("Configuration", "no mail image");
	myConfig.cNoMailUserImage = _find_image_path (path);
	g_free (path);
	path = CD_CONFIG_GET_STRING ("Configuration", "has mail image");
	myConfig.cHasMailUserImage = _find_image_path (path);
	g_free (path);
	myConfig.bPlaySound = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "play sound", TRUE);
	path = CD_CONFIG_GET_STRING ("Configuration", "new mail sound");
	myConfig.cNewMailUserSound = (path?cairo_dock_generate_file_path (path):NULL);
	g_free (path);

	myConfig.cAnimation = CD_CONFIG_GET_STRING ("Configuration", "animation");
	myConfig.iAnimationDuration = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "anim duration", 5);
	myConfig.cMailApplication = CD_CONFIG_GET_STRING ("Configuration", "mail application");
	myConfig.bShowMessageContent = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "show content", TRUE);
	myConfig.iNbMaxShown = MIN (30, CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "max shown mails", 10));
	myConfig.bAlwaysShowMailCount = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "show zero mail", TRUE);
	//myConfig.bShowMessageContent = FALSE;
	
	myConfig.cThemePath = CD_CONFIG_GET_THEME_PATH ("Configuration", "theme", "themes", "Default");
	
	myConfig.cRenderer = CD_CONFIG_GET_STRING ("Configuration", "renderer");
	
	myConfig.bCheckOnStartup = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "check", TRUE);
	
	myConfig.iDialogDuration = 1000 * CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "time_dialog", 5);
	
	//\_________________ On recupere les comptes mail.
	if (myConfig.bCheckOnStartup)
		_get_mail_accounts (CD_APPLET_MY_KEY_FILE, myApplet);
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free( myConfig.cNoMailUserImage );
	g_free( myConfig.cHasMailUserImage );
	g_free( myConfig.cNewMailUserSound );
	g_free( myConfig.cMailApplication );
	g_free( myConfig.cAnimation );
	g_free (myConfig.cThemePath);
	g_free (myConfig.cRenderer);

	if( myData.pMessagesDialog != NULL ) // make sure the dialog is closed
	{
		gldi_object_unref (GLDI_OBJECT(myData.pMessagesDialog));
		myData.pMessagesDialog = NULL;
	}
		
	myConfig.iNbMaxShown = 0;
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cd_mail_free_all_accounts (myApplet);
	CD_APPLET_DELETE_MY_ICONS_LIST;

	if (myData.iCubeCallList != 0)
		glDeleteLists (myData.iCubeCallList, 1);
	if (myData.iNoMailTexture != 0)
		glDeleteTextures (1, &myData.iNoMailTexture);
	if (myData.iHasMailTexture != 0)
		glDeleteTextures (1, &myData.iHasMailTexture);
		
	if (myData.cWorkingDirPath != 0)
		g_free(myData.cWorkingDirPath);
CD_APPLET_RESET_DATA_END



static void _cd_mail_add_new_account (GtkComboBox *pMailTypesCombo, GtkEntry *pMailNameEntry, GldiModuleInstance *myApplet)
{
	cd_debug ("");
	
	//\____________ On recupere le type et le nom du nouveau compte.
	guint iChosenAccountType = gtk_combo_box_get_active(pMailTypesCombo);
	if( iChosenAccountType >= MAIL_NB_STORAGE_TYPES )
	{
		cd_warning ("while trying get chosen account type (%d) : out of range.", iChosenAccountType);
		gldi_dialog_show_temporary_with_icon (D_("Please choose an account type."), myIcon, myContainer, 3000, "same icon");
		return ;
	}
	
	const gchar *pMailAccountName = gtk_entry_get_text(pMailNameEntry);
	if( !pMailNameEntry || *pMailAccountName == '\0' || strcmp (pMailAccountName, D_("New account's name")) == 0)
	{
		cd_warning ("while trying get name of account to create : empty name");
		gldi_dialog_show_temporary_with_icon (D_("Please enter a name for this account."), myIcon, myContainer, 3000, "same icon");
		return ;
	}
	
	//\____________ On ouvre notre fichier de conf.
	GKeyFile* pKeyFile = cairo_dock_open_key_file (CD_APPLET_MY_CONF_FILE);
	g_return_if_fail (pKeyFile != NULL);
	
	if (g_key_file_has_group (pKeyFile, pMailAccountName))
	{
		gldi_dialog_show_temporary_with_icon (D_("This account already exists.\nPlease choose another name for the new account."), myIcon, myContainer, 5000, "same icon");
		g_key_file_free (pKeyFile);
		return ;
	}
	
	//\____________ On rajoute les champs du nouveau compte mail.
	(storage_tab[iChosenAccountType].pcreateFunc)( pKeyFile, pMailAccountName );

	g_key_file_set_string (pKeyFile, pMailAccountName, "remove account", "");
	g_key_file_set_comment(pKeyFile, pMailAccountName, "remove account", "_0 Remove this account", NULL);
	
	cairo_dock_write_keys_to_file (pKeyFile, CD_APPLET_MY_CONF_FILE);
	
	//\____________ On recharge le panneau de config.
	gsize length = 0;
	gchar **pGroupList = g_key_file_get_groups (pKeyFile, &length);
	g_strfreev (pGroupList);
	
	CD_APPLET_RELOAD_CONFIG_PANEL_WITH_PAGE (length-1);  // on se place sur le dernier onglet, qui est celui du nouveau compte.
	
	g_key_file_free (pKeyFile);
}
static void _cd_mail_activate_account (GtkEntry *pEntry, GldiModuleInstance *myApplet)
{
	GtkComboBox *pMailTypesCombo = GTK_COMBO_BOX(g_object_get_data(G_OBJECT (pEntry), "MailTypesCombo"));
	_cd_mail_add_new_account (pMailTypesCombo, pEntry, myApplet);
}
static void _cd_mail_add_account (GtkButton *pButton, GldiModuleInstance *myApplet)
{
	GtkComboBox *pMailTypesCombo = GTK_COMBO_BOX(g_object_get_data(G_OBJECT (pButton), "MailTypesCombo"));
	GtkEntry *pMailNameEntry = GTK_ENTRY(g_object_get_data(G_OBJECT (pButton), "MailNameEntry"));
	_cd_mail_add_new_account (pMailTypesCombo, pMailNameEntry, myApplet);
}

static void _cd_mail_remove_account (GtkButton *pButton, GldiModuleInstance *myApplet)
{
	cd_debug ("");
	//\____________ On supprime le groupe correspondant dans le fichier de conf.
	GKeyFile* pKeyFile = cairo_dock_open_key_file (CD_APPLET_MY_CONF_FILE);
	g_return_if_fail (pKeyFile != NULL);
	
	guint iNumAccount = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (pButton), "AccountIndex"));
	g_return_if_fail (iNumAccount > 2);
	gsize length = 0;
	gchar **pGroupList = g_key_file_get_groups (pKeyFile, &length);
	g_return_if_fail (iNumAccount < length);
	
	gchar *cMailAccount = pGroupList[iNumAccount];
	g_key_file_remove_group (pKeyFile, cMailAccount, NULL);
	
	cairo_dock_write_keys_to_file (pKeyFile, CD_APPLET_MY_CONF_FILE);
	
	g_key_file_free (pKeyFile);
	
	//\____________ On recharge le panneau de config.
	CD_APPLET_RELOAD_CONFIG_PANEL;
	
	//\____________ On supprime le compte et son icone de la liste.
	CDMailAccount *pMailAccount;
	guint i;
	for (i = 0; i < myData.pMailAccounts->len; i ++)
	{
		pMailAccount = g_ptr_array_index (myData.pMailAccounts, i);
		if( !pMailAccount ) continue;
		
		if (strcmp (cMailAccount, pMailAccount->name) == 0)
		{
			cd_debug ("mail : found old account");
			CDMailAccount *pRemovedMailAccount = g_ptr_array_remove_index (myData.pMailAccounts, i); // decale tout de 1 vers la gauche.
			Icon *pIcon = pRemovedMailAccount->icon;
			if (pIcon) // only one account: no icon to remove
			{
				gldi_icon_detach (pIcon);
				cd_debug ("mail : delete old icon");
				gldi_object_unref (GLDI_OBJECT(pIcon));
			}
			cd_debug ("mail : delete old account");
			cd_mail_free_account (pRemovedMailAccount);
			break ;
		}
	}
	
	if (myData.pMailAccounts->len <= 1)
	{
		CD_APPLET_DELETE_MY_ICONS_LIST;
		if (myData.pMailAccounts->len == 1)
		{
			pMailAccount = g_ptr_array_index (myData.pMailAccounts, 0);
			if (pMailAccount)
				pMailAccount->icon = NULL;
		}
	}
	
	
	g_strfreev (pGroupList);
}

void cd_mail_load_custom_widget (GldiModuleInstance *myApplet, GKeyFile* pKeyFile, GSList *pWidgetList)
{
	cd_debug ("");
	//\____________ On recupere notre widget personnalise (un simple container vide qu'on va remplir avec nos trucs).
	CairoDockGroupKeyWidget *pGroupKeyWidget = cairo_dock_gui_find_group_key_widget_in_list (pWidgetList, "Configuration", "add account");
	g_return_if_fail (pGroupKeyWidget != NULL);

	GtkWidget *pCustomWidgetBox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_pack_end (GTK_BOX (pGroupKeyWidget->pKeyBox),
		pCustomWidgetBox,
		FALSE,
		FALSE,
		0);

	//\____________ On cree un combo pour selectionner le type de compte mail qu'on voudrait ajouter
	GtkWidget *pMailTypesCombo = gtk_combo_box_text_new ();

	if( pMailTypesCombo )
	{
		guint j;
		for( j = 0; j < MAIL_NB_STORAGE_TYPES; j++ )
		{
			gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (pMailTypesCombo), storage_tab[j].description );
		}
	}
	gtk_box_pack_start (GTK_BOX (pCustomWidgetBox),
		pMailTypesCombo,
		FALSE,
		FALSE,
		0);
	
	//\____________ On cree une entree de texte pour le nom du compte mail et on l'ajoute dans notre container.
	GtkWidget *pEntry = gtk_entry_new ();
	//gtk_entry_set_text (GTK_ENTRY (pEntry), D_("New account's name"));
	gtk_widget_set_tooltip_text (pEntry, D_("Enter a name for this account. You can give it any name you want."));
	g_object_set_data (G_OBJECT (pEntry), "MailTypesCombo", pMailTypesCombo);
	g_signal_connect (G_OBJECT (pEntry),
		"activate",
		G_CALLBACK (_cd_mail_activate_account),
		myApplet);
	gtk_box_pack_start (GTK_BOX (pCustomWidgetBox),
		pEntry,
		FALSE,
		FALSE,
		0);

	//\____________ On cree un bouton pour ajouter un compte mail et on l'ajoute dans notre container.
	GtkWidget *pButton = gtk_button_new_from_icon_name (GLDI_ICON_NAME_ADD, GTK_ICON_SIZE_BUTTON);
	g_object_set_data (G_OBJECT (pButton), "MailTypesCombo", pMailTypesCombo); // associer le bouton add avec le combo
	g_object_set_data (G_OBJECT (pButton), "MailNameEntry", pEntry); // associer le bouton add avec le texte du nom
	g_signal_connect (G_OBJECT (pButton),
		"clicked",
		G_CALLBACK (_cd_mail_add_account),
		myApplet);
	gtk_box_pack_start (GTK_BOX (pCustomWidgetBox),
		pButton,
		FALSE,
		FALSE,
		0);

	//\____________ Pour chaque compte mail, on va creer un bouton "Remove" dans l'onglet correspondant
	//g_return_if_fail (myData.pMailAccounts != NULL);
	//CDMailAccount *pMailAccount;
	gsize length = 0;
	gchar **pGroupList = g_key_file_get_groups (pKeyFile, &length);
	gchar *cMailAccountName;
	guint i;
	for( i = 3; i < length; i++ )
	//for( i = 0; i < myData.pMailAccounts->len; i++ )  // on utilise myData.pMailAccounts plutot que repartir de la liste des groupes, car on veut passer le nom en entree de la callback, il faut donc qu'il soit persistent, donc c'est plus simple comme ca.
	{
		//pMailAccount = g_ptr_array_index (myData.pMailAccounts, i);  // i-3
		//cMailAccountName = pMailAccount->name;
		cMailAccountName = pGroupList[i];
		cd_debug ("- on ajoute le bouton remove au compte '%s'", cMailAccountName);
		if (! g_key_file_has_group (pKeyFile, cMailAccountName))
		{
			cd_warning ("mail : no group for mail account '%s'", cMailAccountName);
			continue;
		}
		
		//\____________ On recupere notre widget personnalise (un simple container vide qu'on va remplir avec nos trucs).
		CairoDockGroupKeyWidget *pGroupKeyWidget = cairo_dock_gui_find_group_key_widget_in_list (pWidgetList, cMailAccountName, "remove account");
		if( pGroupKeyWidget == NULL )
		{
			cd_warning ("mail : oups, there is a problem in the conf file");
			continue;
		}
		
		//\____________ On cree un bouton pour supprimer le compte et on l'ajoute dans notre container.
		pButton = gtk_button_new_with_label (D_("Remove Account"));
		g_object_set_data (G_OBJECT (pButton), "AccountIndex", GINT_TO_POINTER (i));
		g_signal_connect (G_OBJECT (pButton),
			"clicked",
			G_CALLBACK (_cd_mail_remove_account),
			myApplet);
		gtk_box_pack_start (GTK_BOX (pGroupKeyWidget->pKeyBox),
			pButton,
			FALSE,
			FALSE,
			0);
	}
	g_strfreev (pGroupList);
}


void cd_mail_save_custom_widget (GldiModuleInstance *myApplet, GKeyFile *pKeyFile, GSList *pWidgetList)
{
	cd_debug ("%s (%s)", __func__, myIcon->cName);
	// ca c'est si on avait des valeurs a recuperer dans nos widgets personnalises, et a stocker dans le pKeyFile. mais ici ils sont simple, et donc tous pris en charge par le dock.
}

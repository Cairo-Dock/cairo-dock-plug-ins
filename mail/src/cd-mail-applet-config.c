/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "cd-mail-applet-struct.h"
#include "cd-mail-applet-config.h"
#include "cd-mail-applet-accounts.h"
#include "cd-mail-applet-etpan.h"


struct storage_type_def {
  char * name;
  char * description;
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
#if ( __WORDSIZE == 64 )
/* in 64bit libetpan crashes with RSS, so... avoid it. */
#warning "Compilation 64bits: avoiding RSS accounts"
#else
  {"feed", "RSS/Feed", cd_mail_retrieve_feed_params, cd_mail_create_feed_params },
#endif
};

const int MAIL_NB_STORAGE_TYPES = sizeof(storage_tab) / sizeof(struct storage_type_def);

static void _get_mail_accounts (GKeyFile *pKeyFile, CairoDockModuleInstance *myApplet)
{
	//\_______________ On remet a zero les comptes mail.
	cd_mail_free_all_accounts (myApplet);
	
	myData.iPrevNbUnreadMails = 0;
	myData.iNbUnreadMails = 0;
	
	//\_______________ On recupere les comptes mail dans le fichier de conf.
	CDMailAccount *pMailAccount;
	gchar *cMailAccountName;
	int j, account_type;
	gsize i, length = 0;
	gchar **pGroupList = g_key_file_get_groups (pKeyFile, &length);
	myData.pMailAccounts = g_ptr_array_sized_new (length - 3);
	
	g_print ("recuperons les comptes ...\n");
	for (i = 3; i < length; i ++)  // Icon, Desklet, Configuration + mail groups
	{
		cMailAccountName = pGroupList[i];
		g_print ("+ on recupere le compte '%s'\n", cMailAccountName);
		
		/* Get the type of the account */
		if (! g_key_file_has_key (pKeyFile, cMailAccountName, "type", NULL))
			continue ;
		
		gchar *cMailAccountType = g_key_file_get_string (pKeyFile, cMailAccountName, "type", NULL);

		for( j = 0; j < MAIL_NB_STORAGE_TYPES; j++ )
		{
			if (g_strcasecmp(storage_tab[j].name, cMailAccountType) == 0)
			{
				account_type = j;
				break;
			}
		}
		/* in case the account type is unknown, just ignore... */
		if( j >= MAIL_NB_STORAGE_TYPES )
			continue;
		g_print ("  mail type : %d\n", j);
		
		pMailAccount = g_new0 (CDMailAccount, 1);
		g_ptr_array_add (myData.pMailAccounts, pMailAccount);

		pMailAccount->name = g_strdup (cMailAccountName);
		
		pMailAccount->pAppletInstance = myApplet;
		(storage_tab[account_type].pfillFunc)( pMailAccount, pKeyFile, cMailAccountName );
	}
	g_strfreev (pGroupList);
}

CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	gchar *path;
	path = CD_CONFIG_GET_STRING ("Configuration", "no mail image");
	myConfig.cNoMailUserImage = (path?cairo_dock_generate_file_path (path):NULL);
	g_free (path);
	path = CD_CONFIG_GET_STRING ("Configuration", "has mail image");
	myConfig.cHasMailUserImage = (path?cairo_dock_generate_file_path (path):NULL);
	g_free (path);
	path = CD_CONFIG_GET_STRING ("Configuration", "new mail sound");
	myConfig.cNewMailUserSound = (path?cairo_dock_generate_file_path (path):NULL);
	g_free (path);

	myConfig.cMailApplication = CD_CONFIG_GET_STRING ("Configuration", "mail application");
	myConfig.cMailClass = CD_CONFIG_GET_STRING ("Configuration", "mail class");
	myConfig.bStealTaskBarIcon = myConfig.cMailApplication && CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "inhibate appli", TRUE);
	myConfig.bShowMessageContent = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "show content", TRUE);
	myConfig.bShowMessageContent = FALSE;
	
	myConfig.cThemePath = CD_CONFIG_GET_THEME_PATH ("Configuration", "theme", "themes", "Default");
	
	myConfig.cRenderer = CD_CONFIG_GET_STRING ("Configuration", "renderer");
	
	myConfig.bCheckOnStartup = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "check", TRUE);
	
	//\_________________ On recupere les comptes mail.
	if (myConfig.bCheckOnStartup)
		_get_mail_accounts (CD_APPLET_MY_KEY_FILE, myApplet);
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free( myConfig.cNoMailUserImage );
	g_free( myConfig.cHasMailUserImage );
	g_free( myConfig.cNewMailUserSound );
	g_free( myConfig.cMailApplication );
	g_free( myConfig.cMailClass );
	g_free (myConfig.cThemePath);
	g_free (myConfig.cRenderer);
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
CD_APPLET_RESET_DATA_END



static void _cd_mail_add_new_account (GtkComboBox *pMailTypesCombo, GtkEntry *pMailNameEntry, CairoDockModuleInstance *myApplet)
{
	cd_debug ("");
	
	//\____________ On recupere le type et le nom du nouveau compte.
	gint iChosenAccountType = gtk_combo_box_get_active(pMailTypesCombo);
	if( iChosenAccountType < 0 || iChosenAccountType >= MAIL_NB_STORAGE_TYPES )
	{
		cd_warning ("while trying get chosen account type (%d) : out of range.", iChosenAccountType);
		cairo_dock_show_temporary_dialog_with_icon (D_("Please choose an account type."), myIcon, myContainer, 3000, "same icon");
		return ;
	}
	
	const gchar *pMailAccountName = gtk_entry_get_text(pMailNameEntry);
	if( !pMailNameEntry || *pMailAccountName == '\0' || strcmp (pMailAccountName, D_("New account's name")) == 0)
	{
		cd_warning ("while trying get name of account to create : empty name");
		cairo_dock_show_temporary_dialog_with_icon (D_("Please enter a name for this account."), myIcon, myContainer, 3000, "same icon");
		return ;
	}
	
	//\____________ On ouvre notre fichier de conf.
	GKeyFile* pKeyFile = cairo_dock_open_key_file (CD_APPLET_MY_CONF_FILE);
	g_return_if_fail (pKeyFile != NULL);
	
	if (g_key_file_has_group (pKeyFile, pMailAccountName))
	{
		cairo_dock_show_temporary_dialog_with_icon (D_("This account already exists.\nPlease choose another name for the new account."), myIcon, myContainer, 5000, "same icon");
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
	
	cairo_dock_reload_current_group_widget_full (myApplet, length-1);  // on se place sur le dernier onglet, qui est celui du nouveau compte.
	
	g_key_file_free (pKeyFile);
}
static void _cd_mail_activate_account (GtkEntry *pEntry, CairoDockModuleInstance *myApplet)
{
	GtkComboBox *pMailTypesCombo = GTK_COMBO_BOX(g_object_get_data(G_OBJECT (pEntry), "MailTypesCombo"));
	_cd_mail_add_new_account (pMailTypesCombo, pEntry, myApplet);
}
static void _cd_mail_add_account (GtkButton *pButton, CairoDockModuleInstance *myApplet)
{
	GtkComboBox *pMailTypesCombo = GTK_COMBO_BOX(g_object_get_data(G_OBJECT (pButton), "MailTypesCombo"));
	GtkEntry *pMailNameEntry = GTK_ENTRY(g_object_get_data(G_OBJECT (pButton), "MailNameEntry"));
	_cd_mail_add_new_account (pMailTypesCombo, pMailNameEntry, myApplet);
}

static void _cd_mail_remove_account (GtkButton *pButton, CairoDockModuleInstance *myApplet)
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
	cairo_dock_reload_current_group_widget (myApplet);
	
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
			CairoContainer *pContainer = CD_APPLET_MY_ICONS_LIST_CONTAINER;
			if (myDock)
				cairo_dock_remove_one_icon_from_dock (CAIRO_DOCK (pContainer), pIcon);
			else
			{
				CairoDesklet *pDesklet = CAIRO_DESKLET (pContainer);
				pDesklet->icons = g_list_remove (pDesklet->icons, pIcon);
				cairo_dock_redraw_container (pContainer);
			}
			cd_debug ("mail : delete old icon");
			cairo_dock_free_icon (pIcon);
			cd_debug ("mail : delete old account");
			cd_mail_free_account (pRemovedMailAccount);
		}
	}
	
	g_strfreev (pGroupList);
}

void cd_mail_load_custom_widget (CairoDockModuleInstance *myApplet, GKeyFile* pKeyFile)
{
	cd_debug ("");
	//\____________ On recupere notre widget personnalise (un simple container vide qu'on va remplir avec nos trucs).
	GtkWidget *pCustomWidgetBox = cairo_dock_get_widget_from_name ("Configuration", "add account");
	g_return_if_fail (pCustomWidgetBox != NULL);
	
	//\____________ On cree un combo pour selectionner le type de compte mail qu'on voudrait ajouter
	GtkWidget *pMailTypesCombo = gtk_combo_box_new_text();
	if( pMailTypesCombo )
	{
        for( int j = 0; j < MAIL_NB_STORAGE_TYPES; j++ )
        {
          gtk_combo_box_append_text( GTK_COMBO_BOX (pMailTypesCombo), storage_tab[j].description );
          //gtk_widget_set_tooltip_text (pMenuItem, D_("description du type de compte"));
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
	GtkWidget *pButton = gtk_button_new_from_stock (GTK_STOCK_ADD);
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
		g_print ("- on ajoute le bouton remove au compte '%s'\n", cMailAccountName);
		if (! g_key_file_has_group (pKeyFile, cMailAccountName))
		{
			cd_warning ("mail : no group for mail account '%s'", cMailAccountName);
			continue;
		}
		
		//\____________ On recupere notre widget personnalise (un simple container vide qu'on va remplir avec nos trucs).
		GtkWidget *pCustomWidgetBox = cairo_dock_get_widget_from_name (cMailAccountName, "remove account");
		if( pCustomWidgetBox == NULL )
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
		gtk_box_pack_start (GTK_BOX (pCustomWidgetBox),
			pButton,
			FALSE,
			FALSE,
			0);
	}
	g_strfreev (pGroupList);
}


void cd_mail_save_custom_widget (CairoDockModuleInstance *myApplet, GKeyFile *pKeyFile)
{
	g_print ("%s (%s)\n", __func__, myIcon->acName);
	// ca c'est si on avait des valeurs a recuperer dans nos widgets personnalises, et a stocker dans le pKeyFile. mais ici ils sont simple, et donc tous pris en charge par le dock.
}

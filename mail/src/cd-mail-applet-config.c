/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "cd-mail-applet-struct.h"
#include "cd-mail-applet-config.h"
#include "cd-mail-applet-etpan.h"

CD_APPLET_INCLUDE_MY_VARS

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
  {"feed", "RSS/Feed", cd_mail_retrieve_feed_params, cd_mail_create_feed_params },
};

const int MAIL_NB_STORAGE_TYPES = sizeof(storage_tab) / sizeof(struct storage_type_def);

CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.

	myConfig.cNoMailUserImage = cairo_dock_generate_file_path (CD_CONFIG_GET_STRING ("Configuration", "no mail image"));
	myConfig.cHasMailUserImage = cairo_dock_generate_file_path (CD_CONFIG_GET_STRING ("Configuration", "has mail image"));
	myConfig.cNewMailUserSound = cairo_dock_generate_file_path (CD_CONFIG_GET_STRING ("Configuration", "new mail sound"));

  myConfig.cMailApplication = CD_CONFIG_GET_STRING ("Configuration", "mail application");

  myConfig.cThemePath = CD_CONFIG_GET_THEME_PATH ("Configuration", "theme", "themes", "Default");

  if( myConfig.cThemePath == NULL )
    cd_warning ("Attention : couldn't find theme path, or this theme is not valid");

  myConfig.timeEndOfSound = 0;

  if( myData.pMailAccounts )
  {
    CDMailAccount *pMailAccount;
    guint i;
    for (i = 0; i < myData.pMailAccounts->len; i ++)
    {
      pMailAccount = g_ptr_array_index (myData.pMailAccounts, i);
      cd_mail_free_account (pMailAccount);
    }
    g_ptr_array_free (myData.pMailAccounts, TRUE);
    myData.pMailAccounts = NULL;
  }
  if( !myData.pMailAccounts )
  {
    myData.iNbUnreadMails = 0;
    myData.bNewMailFound = FALSE;

    //\_______________ On recupere les comptes mail.
    myData.pMailAccounts = g_ptr_array_new ();
    CDMailAccount *pMailAccount;
    gboolean bAlarmOK;
    gint i, j, nmailboxes, account_type;
    GString *sKeyName = g_string_new ("");

    nmailboxes = CD_CONFIG_GET_INTEGER_WITH_DEFAULT("Configuration", "nmailboxes", 0);

    for( i = 0; i < nmailboxes; i++ )
    {
      g_string_printf (sKeyName, "mailbox %d name", i);
      if (! g_key_file_has_key (pKeyFile, "Configuration", sKeyName->str, NULL))
        continue ;

      gchar *cMailAccountName = CD_CONFIG_GET_STRING ("Configuration", sKeyName->str);
        
      if (g_key_file_has_group (pKeyFile, cMailAccountName))
      {
        pMailAccount = g_new0 (CDMailAccount, 1);
        g_ptr_array_add (myData.pMailAccounts, pMailAccount);

        pMailAccount->name = g_strdup(cMailAccountName);

        /* Get the type of the account */
        if (! g_key_file_has_key (pKeyFile, cMailAccountName, "type", NULL))
          continue ;
        gchar *cMailAccountType = CD_CONFIG_GET_STRING (cMailAccountName, "type");

        for( j = 0; j < MAIL_NB_STORAGE_TYPES; j++ )
        {
          if (g_strcasecmp(storage_tab[j].name, cMailAccountType) == 0)
          {
            account_type = j;
            break;
          }
        }

        /* in case the account type is unknown, just don't crash... */
        if( j >= MAIL_NB_STORAGE_TYPES ) continue;

        pMailAccount->iNbUnseenMails = 0;
        pMailAccount->dirtyfied = FALSE;
        (storage_tab[account_type].pfillFunc)( pMailAccount, pKeyFile, cMailAccountName );
      }

	  g_free( cMailAccountName );
    }
    g_string_free (sKeyName, TRUE);
  }

  cd_mail_init_accounts(myData.pMailAccounts);

CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
    if( myConfig.cNoMailUserImage )
      g_free( myConfig.cNoMailUserImage );
    if( myConfig.cHasMailUserImage )
      g_free( myConfig.cHasMailUserImage );
    if( myConfig.cNewMailUserSound )
      g_free( myConfig.cNewMailUserSound );
    if( myConfig.cMailApplication )
      g_free( myConfig.cMailApplication );    
CD_APPLET_RESET_CONFIG_END

CD_APPLET_RESET_DATA_BEGIN
	if (myIcon->pSubDock != NULL)
	{
		CD_APPLET_DESTROY_MY_SUBDOCK;
	}
  myData.iNbUnreadMails = 0;
  myData.bNewMailFound = FALSE;

	CDMailAccount *pMailAccount;
	guint i;
	if (myData.pMailAccounts != NULL)
	{
		for (i = 0; i < myData.pMailAccounts->len; i ++)
		{
			pMailAccount = g_ptr_array_index (myData.pMailAccounts, i);
			cd_mail_free_account (pMailAccount);
		}
		g_ptr_array_free (myData.pMailAccounts, TRUE);
	}
CD_APPLET_RESET_DATA_END

static void _cd_mail_add_account (GtkButton *pButton, CairoDockModuleInstance *myApplet)
{
	cd_debug ("%s\n", __func__);

	//\____________ On ouvre notre fichier de conf.
	GError *erreur = NULL;
	GKeyFile* pKeyFile = g_key_file_new();
	g_key_file_load_from_file (pKeyFile, myApplet->cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("while trying to load %s : %s", myApplet->cConfFilePath, erreur->message);
		g_error_free (erreur);
		return ;
	}

	//\____________ recuperer le combo et le nom du nouveau compte
	GtkComboBox *pMailTypesCombo = GTK_COMBO_BOX(g_object_get_data(pButton, "MailTypesCombo"));
	GtkEntry *pMailNameEntry = GTK_ENTRY(g_object_get_data(pButton, "MailNameEntry"));
	//
	gint lChosenAccountType = gtk_combo_box_get_active(pMailTypesCombo);
	if( lChosenAccountType < 0 || lChosenAccountType >= MAIL_NB_STORAGE_TYPES )
	{
		cd_warning ("while trying get chosen account type (%d) : out of range.", lChosenAccountType);
		return ;
	}
	//
	gchar *pMailAccountName = gtk_entry_get_text(pMailNameEntry);
	if( !pMailNameEntry || strlen(pMailAccountName)==0 )
	{
		cd_warning ("while trying get name of account to create.", lChosenAccountType);
		return ;
	}

  gint nmailboxes = g_key_file_get_integer(pKeyFile, "Configuration", "nmailboxes", &erreur);
	if (erreur != NULL)
	{
		nmailboxes = 0;
	}
  
  GString *sKeyName = g_string_new ("");
  g_string_printf (sKeyName, "mailbox %d name", nmailboxes);
  g_key_file_set_string (pKeyFile, "Configuration", sKeyName->str, pMailAccountName);
  g_string_free(sKeyName, TRUE);

	//\____________ On rajoute les champs du nouveau compte mail.
  (storage_tab[lChosenAccountType].pcreateFunc)( pKeyFile, pMailAccountName );

  g_key_file_set_string (pKeyFile, pMailAccountName, "remove account", "");
  g_key_file_set_comment(pKeyFile, pMailAccountName, "remove account", "_ Remove this account", NULL);

  g_key_file_set_integer (pKeyFile, "Configuration", "nmailboxes", nmailboxes+1);
	
	cairo_dock_write_keys_to_file (pKeyFile, myApplet->cConfFilePath);
	g_key_file_free (pKeyFile);
	
	//\____________ On recharge le panneau de config.
	cairo_dock_reload_current_group_widget (myApplet);
}
static void _cd_mail_remove_account (GtkButton *pButton, CairoDockModuleInstance *myApplet)
{
	cd_debug ("%s\n", __func__);

	//\____________ On ouvre notre fichier de conf.
	GError *erreur = NULL;
	GKeyFile* pKeyFile = g_key_file_new();
	g_key_file_load_from_file (pKeyFile, myApplet->cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("while trying to load %s : %s", myApplet->cConfFilePath, erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	//\____________ On recupere le nom du compte a supprimer
	gint lMailAccountIndex = -1;
	lMailAccountIndex = GPOINTER_TO_INT(g_object_get_data(pButton, "AccountIndex"));
  
  GString *sKeyName = g_string_new ("");
	
  //\____________ On synchronise les index des comptes avec leur nom
  gint nmailboxes = g_key_file_get_integer(pKeyFile, "Configuration", "nmailboxes", NULL);
  gint i = 0;

  for( i = 0; i < nmailboxes; i++ )
  {
    if( i == lMailAccountIndex )
    {
      // c'est le compte a supprimer
      g_string_printf (sKeyName, "mailbox %d name", i);
      gchar *cMailAccountName = g_key_file_get_string (pKeyFile, "Configuration", sKeyName->str, NULL);
      
      //\____________ On le supprime.
      g_key_file_remove_comment (pKeyFile, cMailAccountName, NULL, NULL);
      g_key_file_remove_group (pKeyFile, cMailAccountName, NULL);
    }
    if( i >= lMailAccountIndex && i < nmailboxes-1 )
    {
      // ce sont les comptes dont les index sont apres le compte a supprimer, donc on decale les noms
      g_string_printf (sKeyName, "mailbox %d name", i+1);
      gchar *cNextMailAccountName = g_key_file_get_string (pKeyFile, "Configuration", sKeyName->str, NULL);
      g_string_printf (sKeyName, "mailbox %d name", i);
      g_key_file_set_string (pKeyFile, "Configuration", sKeyName->str, cNextMailAccountName);
      g_free( cNextMailAccountName );
    }
    else if( i == nmailboxes-1 )
    {
      // le dernier compte  ne sert plus a rien
      g_string_printf (sKeyName, "mailbox %d name", i);
      g_key_file_remove_key (pKeyFile, "Configuration", sKeyName->str, NULL);
    }
	}
	
  g_string_free(sKeyName, TRUE);

  // \__________ On decremente le nombre de comptes mail
  if( nmailboxes > 0 )
  {
    g_key_file_set_integer (pKeyFile, "Configuration", "nmailboxes", nmailboxes-1);
  }
  
  cairo_dock_write_keys_to_file (pKeyFile, myApplet->cConfFilePath);
	
	g_key_file_free (pKeyFile);
	
	//\____________ On recharge le panneau de config.
	cairo_dock_reload_current_group_widget (myApplet);
}
void cd_mail_load_custom_widget (CairoDockModuleInstance *myApplet, GKeyFile* pKeyFile)
{
	g_print ("%s (%s)\n", __func__, myIcon->acName);
	//\____________ On recupere notre widget personnalise (un simple container vide qu'on va remplir avec nos trucs).
	GtkWidget *pCustomWidgetBox = cairo_dock_get_widget_from_name ("Configuration", "add account");
	g_return_if_fail (pCustomWidgetBox != NULL);
	
	//\____________ On cree un combo pour selectionner le type de compte mail qu'on voudrait ajouter
	GtkComboBox *pMailTypesCombo = gtk_combo_box_new_text();
	if( pMailTypesCombo )
	{
        for( int j = 0; j < MAIL_NB_STORAGE_TYPES; j++ )
        {
          gtk_combo_box_append_text( pMailTypesCombo, storage_tab[j].description );
        }
	}
	gtk_box_pack_start (GTK_BOX (pCustomWidgetBox),
		pMailTypesCombo,
		FALSE,
		FALSE,
		0);
	
	//\____________ On cree une entree de texte pour le nom du compte mail et on l'ajoute dans notre container.
	GtkWidget *pEntry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (pCustomWidgetBox),
		pEntry,
		FALSE,
		FALSE,
		0);

	//\____________ On cree un bouton pour ajouter un compte mail et on l'ajoute dans notre container.
	GtkWidget *pButton = gtk_button_new_from_stock (GTK_STOCK_ADD);
    g_object_set_data(pButton, "MailTypesCombo",pMailTypesCombo); // associer le bouton add avec le combo
    g_object_set_data(pButton, "MailNameEntry",pEntry); // associer le bouton add avec le texte du nom
	g_signal_connect (G_OBJECT (pButton),
		"clicked",
		G_CALLBACK (_cd_mail_add_account),
		myApplet);
	gtk_box_pack_start (GTK_BOX (pCustomWidgetBox),
		pButton,
		FALSE,
		FALSE,
		0);

	// Pour chaque compte mail, on va creer un bouton "Remove" dans l'onglet correspondant
  gint i, nmailboxes;
  GString *sKeyName = g_string_new ("");

	GError *erreur = NULL;
  nmailboxes = g_key_file_get_integer(pKeyFile, "Configuration", "nmailboxes", &erreur);
  if (erreur != NULL)
  {
		g_error_free (erreur);
    nmailboxes = 0;
  }

  for( i = 0; i < nmailboxes; i++ )
  {
    g_string_printf (sKeyName, "mailbox %d name", i);
    if (! g_key_file_has_key (pKeyFile, "Configuration", sKeyName->str, NULL))
      continue ;

    gchar *cMailAccountName = g_key_file_get_string(pKeyFile, "Configuration", sKeyName->str, NULL);
      
    if (g_key_file_has_group (pKeyFile, cMailAccountName))
    {
      //\____________ On recupere notre widget personnalise (un simple container vide qu'on va remplir avec nos trucs).
      GtkWidget *pCustomWidgetBox = cairo_dock_get_widget_from_name (cMailAccountName, "remove account");
      if( pCustomWidgetBox == NULL ) continue;

      //\____________ On cree un bouton pour supprimer une alarme et on l'ajoute dans notre container.
      pButton = gtk_button_new_with_label (_("Remove Account"));
      g_object_set_data(pButton, "AccountIndex", GINT_TO_POINTER(i));
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
    g_free( cMailAccountName );
  }
  g_string_free (sKeyName, TRUE);
}


void cd_mail_save_custom_widget (CairoDockModuleInstance *myApplet, GKeyFile *pKeyFile)
{
	g_print ("%s (%s)\n", __func__, myIcon->acName);
	// ca c'est si on avait des valeurs a recuperer dans nos widgets personnalises, et a stocker dans le pKeyFile. mais ici ils sont simple, et donc tous pris en charge par le dock.
}


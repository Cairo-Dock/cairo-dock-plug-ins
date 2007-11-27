#include <stdlib.h>
#include "rhythmbox-menu-functions.h"

extern Icon *myIcon;

//*********************************************************************************
// rhythmbox_previous : Joue la piste précédante
//*********************************************************************************
void rhythmbox_previous (GtkMenuItem *menu_item, gpointer *data)
{
	g_print ("%s ()\n", __func__);
	
	gchar *command = g_strdup_printf ("rhythmbox-client --previous");
	system (command);
	g_free (command);
}


//*********************************************************************************
// rhythmbox_next : Joue la piste suivante
//*********************************************************************************
void rhythmbox_next (GtkMenuItem *menu_item, gpointer *data)
{
	g_print ("%s ()\n", __func__);
	
	gchar *command = g_strdup_printf ("rhythmbox-client --next");
	system (command);
	g_free (command);
}


//*********************************************************************************
// rhythmbox_pause : Stop la lecture
//*********************************************************************************
void rhythmbox_pause (GtkMenuItem *menu_item, gpointer *data)
{
	g_print ("%s ()\n", __func__);
	
	gchar *command = g_strdup_printf ("rhythmbox-client --pause");
	system (command);
	g_free (command);
}


//*********************************************************************************
// rhythmbox_play : Joue la piste
//*********************************************************************************
void rhythmbox_play (GtkMenuItem *menu_item, gpointer *data)
{
	g_print ("%s ()\n", __func__);
	
	gchar *command = g_strdup_printf ("rhythmbox-client --play");
	system (command);
	g_free (command);
}


//*********************************************************************************
// rhythmbox_about : Informations sur l'auteur
//*********************************************************************************
void rhythmbox_about (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget *pWidget = data[0];
	GtkWidget *pMessageDialog = gtk_message_dialog_new (GTK_WINDOW (pWidget),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_CLOSE,
		"This is the rhythmbox applet made by Necropotame for Cairo-Dock");
	
	gtk_dialog_run (GTK_DIALOG (pMessageDialog));
	gtk_widget_destroy (pMessageDialog);
}


gboolean rhythmbox_notification_build_menu (gpointer *data)
{
	if (data[0] == myIcon)
	{
		GtkWidget *pMenu = data[2];
		
		GtkWidget *menu_item;
		//Bouton "Previous"
		menu_item = gtk_menu_item_new_with_label ("Previous");
		gtk_menu_shell_append  (GTK_MENU_SHELL (pMenu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK (rhythmbox_previous), NULL);
		//Bouton "Next"
		menu_item = gtk_menu_item_new_with_label ("Next");
		gtk_menu_shell_append  (GTK_MENU_SHELL (pMenu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK (rhythmbox_next), NULL);
		//Bouton "Pause"
		menu_item = gtk_menu_item_new_with_label ("Pause");
		gtk_menu_shell_append  (GTK_MENU_SHELL (pMenu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK (rhythmbox_pause), NULL);
		//Bouton "Play"
		menu_item = gtk_menu_item_new_with_label ("Play");
		gtk_menu_shell_append  (GTK_MENU_SHELL (pMenu), menu_item);
		g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK (rhythmbox_play), NULL);
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


//*********************************************************************************
// rhythmbox_action() : Fonction appelée au clique sur l'icone
// Cette fonction met le lecteur en pause ou en lecture selon son état
//*********************************************************************************
gboolean rhythmbox_action (gpointer *data)
{
	if (data[0] == myIcon)
	{
		g_print ("%s ()\n", __func__);
		
		if(rhythmbox_getPlaying())
		{
			gchar *command = g_strdup_printf ("rhythmbox-client --pause");
			system (command);
			g_free (command);
		}
		else
		{
			gchar *command = g_strdup_printf ("rhythmbox-client --play");
			system (command);
			g_free (command);
		}
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>

#include "applet-struct.h"
#include "applet-menu.h"

static GList *s_pItemList = NULL;

static void _on_activate_item (GtkWidget *pMenuItem, CDQuickBrowserItem *pItem);

static int _sort_item (CDQuickBrowserItem *pItem1, CDQuickBrowserItem *pItem2)
{
	if (pItem1 == NULL)
		return -1;
	if (pItem2 == NULL)
		return 1;
	CairoDockModuleInstance *myApplet = pItem2->pApplet;
	if (myConfig.bFoldersFirst)
	{
		if (pItem1->pSubMenu && ! pItem2->pSubMenu)
			return -1;
		if (! pItem1->pSubMenu && pItem2->pSubMenu)
			return 1;
	}
	if (myConfig.bCaseUnsensitive)
		return g_ascii_strcasecmp (pItem1->cTmpFileName, pItem2->cTmpFileName);
	else
		return strcmp (pItem1->cTmpFileName, pItem2->cTmpFileName);
}
static GList *_list_dir (const gchar *cDirPath, CairoDockModuleInstance *myApplet)
{
	//\______________ On ouvre le repertoire en lecture.
	GError *erreur = NULL;
	GDir *dir = g_dir_open (cDirPath, 0, &erreur);
	if (erreur != NULL)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
		return NULL;
	}
	
	//\______________ On recupere chaque item, qu'on classe dans une liste temporaire.
	CDQuickBrowserItem *pItem;
	const gchar *cFileName;
	GList *pLocalItemList = NULL;
	do
	{
		cFileName = g_dir_read_name (dir);
		if (cFileName == NULL)
			break ;
		if (! myConfig.bShowHiddenFiles && *cFileName == '.')
			continue;
		pItem = g_new0 (CDQuickBrowserItem, 1);
		pItem->cPath = g_strdup_printf ("%s/%s", cDirPath, cFileName);
		pItem->cTmpFileName = cFileName;  // valable uniquement dans cette boucle, ca tombe bien le classement se fait ici.
		pItem->pApplet = myApplet;
		if (g_file_test (pItem->cPath, G_FILE_TEST_IS_DIR))
			pItem->pSubMenu = gtk_menu_new ();
		
		pLocalItemList = g_list_insert_sorted (pLocalItemList,
			pItem,
			(GCompareFunc)_sort_item);
	}
	while (1);
	g_dir_close (dir);
	
	return pLocalItemList;
}
static void _init_fill_menu_from_dir (CDQuickBrowserItem *pItem)
{
	const gchar *cDirPath = pItem->cPath;
	GtkWidget *pMenu = pItem->pSubMenu;
	CairoDockModuleInstance *myApplet = pItem->pApplet;
	
	//\______________ On recupere les items du repertoire.
	GList *pLocalItemList = _list_dir (cDirPath, myApplet);
	
	//\______________ On rajoute en premier une entree pour ouvrir le repertoire.
	CDQuickBrowserItem *pOpenDirItem = g_new0 (CDQuickBrowserItem, 1);
	pOpenDirItem->cPath = g_strdup (cDirPath);
	pOpenDirItem->pApplet = myApplet;
	pItem->pLocalItemList = g_list_prepend (pLocalItemList, pOpenDirItem);
	pItem->pCurrentItem = pItem->pLocalItemList->next;  // on la rajoute au menu ici, pas en meme temps que les autres.
	
	//\______________ On ajoute cette entree dans le menu des maintenant.
	GtkWidget *pMenuItem;
	if (myConfig.bHasIcons)
	{
		pMenuItem = gtk_image_menu_item_new_with_label (D_("Open this folder"));
		GtkWidget *image = gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (pMenuItem), image);
	}
	else
	{
		pMenuItem = gtk_menu_item_new_with_label (D_("Open this folder"));
	}
	gtk_menu_shell_append  (GTK_MENU_SHELL (pMenu), pMenuItem);
	g_signal_connect (G_OBJECT (pMenuItem), "activate", G_CALLBACK(_on_activate_item), pOpenDirItem);
}
static void _fill_submenu_with_items (CDQuickBrowserItem *pRootItem, int iNbSubItemsAtOnce)
{
	CairoDockModuleInstance *myApplet = pRootItem->pApplet;
	GtkWidget *pMenu = pRootItem->pSubMenu;
	GList *pFirstItem = pRootItem->pCurrentItem;
	
	CDQuickBrowserItem *pItem;
	gchar *cFileName;
	GtkWidget *pMenuItem;
	gboolean bSetImage;
	gchar *cName = NULL, *cURI = NULL, *cIconName = NULL;
	gboolean bIsDirectory;
	int iVolumeID;
	double fOrder;
	GList *l;
	int i;
	for (l = pFirstItem, i = 0; l != NULL && i < iNbSubItemsAtOnce; l = l->next, i ++)
	{
		pItem = l->data;
		
		//\______________ On cree l'entree avec son icone si necessaire.
		bSetImage = FALSE;
		if (myConfig.bHasIcons)
		{
			cairo_dock_fm_get_file_info (pItem->cPath, &cName, &cURI, &cIconName, &bIsDirectory, &iVolumeID, &fOrder, 0);
			g_free (cName);
			cName = NULL;
			g_free (cURI);
			cURI = NULL;
		}
		cFileName = strrchr (pItem->cPath, '/');
		if (cFileName)
			cFileName ++;
		if (cIconName != NULL)
		{
			pMenuItem = gtk_image_menu_item_new_with_label (cFileName);
			GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size (cIconName, 32, 32, NULL);
			GtkWidget *image = gtk_image_new_from_pixbuf (pixbuf);
			g_object_unref (pixbuf);
			gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (pMenuItem), image);
			g_free (cIconName);
			cIconName = NULL;
		}
		else
		{
			pMenuItem = gtk_menu_item_new_with_label (cFileName);
		}
		
		//\______________ On l'insere dans le menu.
		gtk_menu_shell_append  (GTK_MENU_SHELL (pMenu), pMenuItem);
		g_signal_connect (G_OBJECT (pMenuItem), "activate", G_CALLBACK(_on_activate_item), pItem);
		
		if (pItem->pSubMenu != NULL)
		{
			gtk_menu_item_set_submenu (GTK_MENU_ITEM (pMenuItem), pItem->pSubMenu);
		}
	}
	pRootItem->pCurrentItem = l;
}
static gboolean _fill_submenu_idle (CDQuickBrowserItem *pItem)
{
	CairoDockModuleInstance *myApplet = pItem->pApplet;
	if (pItem->pLocalItemList == NULL)
	{
		_init_fill_menu_from_dir (pItem);
		if (pItem->pLocalItemList == NULL)  // cas particulier d'un repertoire vide, inutile de revenir ici pour rien faire.
			pItem->bMenuBuilt = TRUE;
	}
	else
	{
		_fill_submenu_with_items (pItem, myConfig.iNbSubItemsAtOnce);
		if (pItem->pCurrentItem == NULL)
			pItem->bMenuBuilt = TRUE;
	}

	if (pItem->bMenuBuilt)
	{
		CairoDockModuleInstance *myApplet = pItem->pApplet;
		myData.iSidFillDirIdle = 0;
		gtk_widget_show_all (pItem->pSubMenu);
		return FALSE;
	}
	return TRUE;
}
static void _on_activate_item (GtkWidget *pMenuItem, CDQuickBrowserItem *pItem)
{
	//g_print ("%s (%s, %x)\n", __func__, pItem->cPath, pItem->pSubMenu);
	CairoDockModuleInstance *myApplet = pItem->pApplet;
	if (pItem->pSubMenu != NULL)
	{
		if (! pItem->bMenuBuilt)
		{
			//g_print ("  c'est un repertoire (tache courante : %d)\n", myData.iSidFillDirIdle);
			if (myData.iSidFillDirIdle != 0)
				g_source_remove (myData.iSidFillDirIdle);
			myData.iSidFillDirIdle = g_idle_add ((GSourceFunc) _fill_submenu_idle, pItem);
		}
	}
	else
	{
		cairo_dock_fm_launch_uri (pItem->cPath);
		cd_quick_browser_destroy_menu (myApplet);
	}
}


CDQuickBrowserItem *cd_quick_browser_make_menu_from_dir (const gchar *cDirPath, CairoDockModuleInstance *myApplet)
{
	CDQuickBrowserItem *pRootItem = g_new0 (CDQuickBrowserItem, 1);
	pRootItem->cPath = g_strdup (cDirPath);
	pRootItem->pApplet = myApplet;
	pRootItem->pSubMenu = gtk_menu_new ();
	
	_init_fill_menu_from_dir (pRootItem);
	_fill_submenu_with_items (pRootItem, 1e6);
	pRootItem->bMenuBuilt = TRUE;
	gtk_widget_show_all (pRootItem->pSubMenu);
	
	return pRootItem;
}

static void _free_item (CDQuickBrowserItem *pItem)
{
	g_free (pItem->cPath);
	if (pItem->pLocalItemList != NULL)
	{
		g_list_foreach (pItem->pLocalItemList, (GFunc) _free_item, NULL);
		g_list_free (pItem->pLocalItemList);
	}
	g_free (pItem);
}
void cd_quick_browser_destroy_menu (CairoDockModuleInstance *myApplet)
{
	if (myData.iSidFillDirIdle != 0)
		g_source_remove (myData.iSidFillDirIdle);
	myData.iSidFillDirIdle = 0;
	
	if (myData.pRootItem != NULL)
	{
		gtk_widget_destroy (myData.pRootItem->pSubMenu);  // detruit tous les pSubMenu en cascade.
		_free_item (myData.pRootItem);
		myData.pRootItem = NULL;
	}
}

void cd_quick_browser_show_menu (CairoDockModuleInstance *myApplet)
{
	cd_quick_browser_destroy_menu (myApplet);
	
	myData.pRootItem = cd_quick_browser_make_menu_from_dir (myConfig.cDirPath, myApplet);
	g_return_if_fail (myData.pRootItem != NULL && myData.pRootItem->pSubMenu != NULL);
	
	g_signal_connect (G_OBJECT (myData.pRootItem->pSubMenu),
		"deactivate",
		G_CALLBACK (cairo_dock_delete_menu),
		myContainer);
	
	if (myDock)
		myDock->bMenuVisible = TRUE;
	gtk_menu_popup (GTK_MENU (myData.pRootItem->pSubMenu),
		NULL,
		NULL,
		NULL,
		NULL,
		1,
		gtk_get_current_event_time ());
}

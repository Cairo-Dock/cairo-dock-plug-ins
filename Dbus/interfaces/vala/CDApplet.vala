/* This is a part of the external applet for Cairo-Dock

Copyright : (C) 2010-2011 by Fabounet
E-mail : fabounet@glx-dock.org

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
http://www.gnu.org/licenses/licenses.html#GPL */

/// To compile it manually:
/// valac -q -C --disable-warnings --disable-dbus-transformation --pkg gio-2.0 --vapi=CDApplet-simple.vapi --internal-vapi=CDApplet.vapi --header=CDApplet-simple.h --internal-header=CDApplet.h CDApplet.vala
/// gcc --shared -fPIC -o CDApplet.so $(pkg-config --cflags --libs gobject-2.0 gio-2.0) CDApplet.c

  /////////////////////////
 ///// dependancies //////
/////////////////////////
using GLib;

namespace CairoDock.Applet
{
[DBus (name = "org.cairodock.CairoDock.applet")]
public interface IApplet : Object {
	public signal void on_click (int iState);
	public signal void on_middle_click ();
	public signal void on_build_menu ();
	public signal void on_menu_select(int iNumEntry);
	public signal void on_scroll(bool bScrollUp);
	public signal void on_drop_data(string cReceivedData);
	public signal void on_answer(Variant answer);
	public signal void on_answer_dialog(int iButton, Variant answer);
	public signal void on_shortkey(string cKey);
	public signal void on_change_focus(bool bIsActive);
	public signal void on_stop_module ();
	public signal void on_reload_module (bool bConfigHasChanged);
	public abstract Variant Get (string cProperty) throws IOError;
	public abstract HashTable<string,Variant> GetAll () throws IOError;
	public abstract void SetQuickInfo (string cQuickInfo) throws IOError;
	public abstract void SetLabel (string cLabel) throws IOError;
	public abstract void SetIcon (string cImage) throws IOError;
	public abstract void SetEmblem (string cImage, int iPosition) throws IOError;
	public abstract void Animate (string cAnimation, int iRounds) throws IOError;
	public abstract void DemandsAttention (bool bStart, string cAnimation) throws IOError;
	public abstract void ShowDialog (string cMessage, int iDuration) throws IOError;
	public abstract void PopupDialog (HashTable<string,Variant> hDialogAttributes, HashTable<string,Variant> hWidgetAttributes) throws IOError;
	public abstract void AddDataRenderer (string cType, int iNbValues, string cTheme) throws IOError;
	public abstract void RenderValues (double[] pValues) throws IOError;
	public abstract void ControlAppli (string cApplicationClass) throws IOError;
	public abstract void ShowAppli (bool bShow) throws IOError;
	public abstract void AddMenuItems (HashTable<string,Variant>[] pItems) throws IOError;
	public abstract void BindShortkey (string[] cShortkeys) throws IOError;
}

[DBus (name = "org.cairodock.CairoDock.subapplet")]
public interface ISubApplet : Object {
	public signal void on_click_sub_icon (int iState, string cIconID);
	public signal void on_middle_click_sub_icon (string cIconID);
	public signal void on_scroll_sub_icon (bool bScrollUp, string cIconID);
	public signal void on_drop_data_sub_icon (string cReceivedData, string cIconID);
	public signal void on_build_menu_sub_icon (string cIconID);
	public abstract void SetQuickInfo(string cQuickInfo, string cIconID) throws IOError;
	public abstract void SetLabel(string cLabel, string cIconID) throws IOError;
	public abstract void SetIcon(string cImage, string cIconID) throws IOError;
	public abstract void SetEmblem(string cImage, int iPosition, string cIconID) throws IOError;
	public abstract void Animate(string cAnimation, int iNbRounds, string cIconID) throws IOError;
	public abstract void ShowDialog(string message, int iDuration, string cIconID) throws IOError;
	public abstract void AddSubIcons(string[] pIconFields) throws IOError;
	public abstract void RemoveSubIcon(string cIconID) throws IOError;
}

public class CDApplet : GLib.Object
{
	// internal data.
	public IApplet icon;
	public ISubApplet sub_icons;
	public string cAppletName;
	public string cConfFile;
	public string cParentAppName;
	public string cBusPath;
	private MainLoop loop;
	private string _cMenuIconId;
	
	public enum ScreenPosition {
		BOTTOM = 0,
		TOP,
		RIGHT,
		LEFT
	}
	public enum ContainerType {
		DOCK = 0,
		DESKLET
	}
	public enum EmblemPosition {
		UPPER_LEFT = 0,
		LOWER_RIGHT,
		LOWER_LEFT,
		UPPER_RIGHT,
		MIDDLE
	}
	public enum MenuItemType {
		MENU_ENTRY = 0,
		MENU_SUB_MENU,
		MENU_SEPARATOR,
		MENU_CHECKBOX,
		MENU_RADIO_BUTTON
	}
	
	public enum MenuItemId {
		MAIN_MENU_ID = 0
	}

	public enum DialogKey {
		DIALOG_KEY_ENTER = -1,
		DIALOG_KEY_ESCAPE = -2
    }
	
	public CDApplet(string[] argv)
	{
		this.cAppletName = argv[0].substring(2,999);
		this.cBusPath = argv[2];
		this.cConfFile = argv[3];
		this.cParentAppName = argv[4];
		
		this._get_config();
		this._connect_to_bus();
		this._cMenuIconId = null;
	}
	
	public void run()
	{
		this.begin();
		this.loop = new MainLoop();
		this.loop.run();
	}
	
	  ////////////////////////////////////////
	 ////// callbacks on the main icon //////
	////////////////////////////////////////
	public virtual void on_click(int iState)
	{
		print (">>> clic !\n");
	}
	public virtual void on_middle_click()
	{
		print (">>> middle clic !\n");
	}
	private void _on_build_menu()
	{
		this._cMenuIconId = null;
		this.on_build_menu();
	}
	public virtual void on_build_menu()
	{
		print (">>> build menu !\n");
	}
	private void _on_menu_select(int iNumEntry)
	{
		if (this._cMenuIconId == null)
			this.on_menu_select (iNumEntry);
		else
			this.on_menu_select_sub_icon (iNumEntry, this._cMenuIconId);
	}
	public virtual void on_menu_select(int iNumEntry)
	{
		print (">>> choice %d has been selected !\n", iNumEntry);
	}
	public virtual void on_scroll(bool bScrollUp)
	{
		print (">>> scroll (up:%d)\n", (int)bScrollUp);
	}
	public virtual void on_drop_data(string cReceivedData)
	{
		print (">>> received : %s\n",cReceivedData);
	}
	public virtual void on_answer(Variant answer)
	{
		print (">>> answer\n");
	}
	public virtual void on_answer_dialog(int iButton, Variant answer)
	{
		print (">>> answer dialog\n");
	}
	public virtual void on_shortkey(string cKey)
	{
		print (">>> shortkey : %s\n", cKey);
	}
	public virtual void on_change_focus(bool bIsActive)
	{
		print (">>> changed focus -> %d\n", (int)bIsActive);
	}

	  ////////////////////////////////////////
	 ////// callbacks on the sub-icons //////
	////////////////////////////////////////
	public virtual void on_click_sub_icon(int iState, string cIconID)
	{
		print ("clic on the sub-icon '%s' !\n", cIconID);
	}
	
	public virtual void on_middle_click_sub_icon(string cIconID)
	{
		print ("middle-clic on the sub-icon '%s' !\n", cIconID);
	}
	
	public virtual void on_scroll_sub_icon(bool bScrollUp, string cIconID)
	{
		print ("scroll on the sub-icon '%s' !\n", cIconID);
	}
	
	public virtual void on_drop_data_sub_icon(string cReceivedData, string cIconID)
	{
		print ("drop on the sub-icon '%s' !\n", cIconID);
	}
	
	private void _on_build_menu_sub_icon(string cIconID)
	{
		this._cMenuIconId = cIconID;
		this.on_build_menu_sub_icon (cIconID);
	}
	public virtual void on_build_menu_sub_icon(string cIconID)
	{
		print ("build menu on the sub-icon '%s' !\n", cIconID);
	}
	public virtual void on_menu_select_sub_icon(int iNumEntry, string cIconID)
	{
		print (">>> choice %d has been selected on icon %s !\n", iNumEntry, cIconID);
	}
	
	  /////////////////////////////////////
	 ////// callbacks on the applet //////
	/////////////////////////////////////
	public virtual void begin()
	{
	}
	
	public virtual void end()
	{
	}
	private void _on_stop()
	{
		print (">>> applet '%s' is stopped\n", this.cAppletName);
		this.end();
		loop.quit();
	}
	
	public virtual void reload()
	{
	}
	private void _on_reload(bool bConfigHasChanged)
	{
		print (">>> our module is reloaded");
		if (bConfigHasChanged)
		{
			print (">>>  and our config has changed");
			this._get_config();
			this.reload();
		}
	}
	
	public virtual void get_config(GLib.KeyFile keyfile)
	{
	}
	private void _get_config()
	{
		GLib.KeyFile keyfile = new GLib.KeyFile();
		try
		{
			keyfile.load_from_file(this.cConfFile, GLib.KeyFileFlags.NONE);
		}
		catch (Error e)
		{
			warning (e.message);
		}
		finally
		{
			this.get_config(keyfile);
		}
	}
	
	private void _connect_to_bus()
	{
		///string cBusPath = "/org/cairodock/CairoDock/"+cAppletName;  // path where our object is stored on the bus.
		try
		{
			this.icon = Bus.get_proxy_sync (BusType.SESSION,
				"org.cairodock.CairoDock",
				this.cBusPath);
		}
		catch (IOError e)
		{
			GLib.error (">>> module '%s' can't be found on the bus, exit.\nError was: %s", this.cAppletName, e.message);
		}
		try
		{
			this.sub_icons = Bus.get_proxy_sync (BusType.SESSION,
				"org.cairodock.CairoDock",
				this.cBusPath+"/sub_icons");
		}
		catch (IOError e)
		{
			GLib.error (">>> module '%s' can't be found on the bus, exit.\nError was: %s", this.cAppletName, e.message);
		}
		this.icon.on_click.connect(on_click);  // when the user left-clicks on our icon.
		this.icon.on_middle_click.connect(on_middle_click);  // when the user middle-clicks on our icon.
		this.icon.on_build_menu.connect(_on_build_menu);  // when the user right-clicks on our applet (which builds the menu)
		this.icon.on_menu_select.connect(_on_menu_select);  // when the user selects an entry of this menu.
		this.icon.on_scroll.connect(on_scroll);  // when the user scroll up or down on our icon.
		this.icon.on_drop_data.connect(on_drop_data);  // when the user drops something on our icon.
		this.icon.on_answer.connect(on_answer);  // when the user answer a question (deprecated).
		this.icon.on_answer_dialog.connect(on_answer_dialog);  // when the user answer a dialog.
		this.icon.on_shortkey.connect(on_shortkey);  // when the user presses a shortkey.
		this.icon.on_change_focus.connect(on_change_focus);  // when the focus of the applet's window changes.
		this.icon.on_stop_module.connect(_on_stop);  // when the user deactivate our applet (or the DBus plug-in, or when the Cairo-Dock is stopped).
		this.icon.on_reload_module.connect(_on_reload);  // when the user changes something in our config, or when the desklet is resized (with no change in the config).
		this.sub_icons.on_click_sub_icon.connect(on_click_sub_icon);  // when the user left-clicks on a sub-icon.
		this.sub_icons.on_middle_click_sub_icon.connect(on_middle_click_sub_icon);  // when the user middle-clicks on a sub-icon.
		this.sub_icons.on_scroll_sub_icon.connect(on_scroll_sub_icon);  // when the user scrolls on a sub-icon.
		this.sub_icons.on_drop_data_sub_icon.connect(on_drop_data_sub_icon);  // when the user drops sth on a sub-icon.
		this.sub_icons.on_build_menu_sub_icon.connect(_on_build_menu_sub_icon);  // when the user drops sth on a sub-icon.
	}
}
}

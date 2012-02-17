//
//
//This is a part of the external demo applet for Cairo-Dock
//
//Copyright : (C) 2010-2011 by Fabounet
//E-mail : fabounet@glx-dock.org
//
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either version 2
//of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//GNU General Public License for more details.
//http://www.gnu.org/licenses/licenses.html//GPL

// Base class for Cairo-Dock's applets.
// Make your own class derive from a CDApplet, and override the functions you need (the ones which don't start with an underscore).

// Compile it with the following command, then rename 'demo_mono.exe' to 'demo_mono'.
// gmcs /target:library CDApplet.cs ISubApplet.cs IApplet.cs -pkg:glib-sharp-2.0 -pkg:ndesk-dbus-1.0 -pkg:ndesk-dbus-glib-1.0
  //////////////////////////
 ////// dependancies //////
//////////////////////////
using System;  // Environment
using System.IO;  // Path, Directory
using System.Reflection;
using GLib;
using NDesk.DBus;
using CairoDock.Applet;

//namespace CairoDock.Applet
//{
public class CDApplet
{
	public string cAppletName = null;
	public string cParentAppName = null;
	public string cBusPath = null;
	public string cConfFile = null;
	public string cShareDataDir = null;
	public string cRootDataDir = null;
	public IApplet icon = null;
	public ISubApplet sub_icons= null;
	private GLib.MainLoop loop = null;
	private string cMenuIconId = null;
	
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
		EMBLEM_TOP_LEFT = 0,
		EMBLEM_BOTTOM_RIGHT,
		EMBLEM_BOTTOM_LEFT,
		EMBLEM_TOP_RIGHT,
		EMBLEM_MIDDLE,
		EMBLEM_BOTTOM,
		EMBLEM_TOP,
		EMBLEM_RIGHT,
		EMBLEM_LEFT
	}
	public enum EmblemModifier {
		EMBLEM_PERSISTENT = 0,
		EMBLEM_PRINT = 9
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
	
	public CDApplet()
	{
		String[] argv = Environment.GetCommandLineArgs();
		this.cAppletName = argv[0].Substring(2);
		this.cBusPath = argv[2];
		this.cConfFile = argv[3];
		this.cRootDataDir = argv[4];
		this.cParentAppName = argv[5];
		//this.cShareDataDir = Environment.CurrentDir();  // not sure of the exact syntax...
		
		this._get_config ();
		this._connect_to_dock ();
	}
	
	public void run()
	{
		this.begin();
		loop = new GLib.MainLoop();
		loop.Run();
		Console.WriteLine(">>> Applet " + this.cAppletName + " terminated");
	}
	
	//////////////////////////////////
	/// callbacks on the main icon ///
	//////////////////////////////////
	
	public virtual void on_click (int iClickState)
	{
		Console.WriteLine(">>> click");
	}
	public virtual void on_middle_click ()
	{
		Console.WriteLine(">>> middle click");
	}
	public virtual void on_scroll (bool bScrollUp)
	{
		Console.WriteLine(">>> scroll up " + bScrollUp);
	}
	private void _on_build_menu ()
	{
		this.cMenuIconId = null;
		this.on_build_menu ();
	}
	public virtual void on_build_menu ()
	{
		Console.WriteLine(">>> build menu");
	}
	private void _on_menu_select (int iNumEntry)
	{
		if (this.cMenuIconId == null)
			this.on_menu_select (iNumEntry);
		else
			this.on_menu_select_sub_icon (iNumEntry, this.cMenuIconId);
	}
	public virtual void on_menu_select (int iNumEntry)
	{
		Console.WriteLine(">>> select entry : "+iNumEntry);
	}
	public virtual void on_drop_data (string cReceivedData)
	{
		Console.WriteLine(">>> drop : "+cReceivedData);
	}
	public virtual void on_answer (System.Object answer)
	{
		Console.WriteLine(">>> answer : "+answer);
	}
	public virtual void on_answer_dialog (int iButton, System.Object answer)
	{
		Console.WriteLine(">>> answer : "+answer);
	}
	public virtual void on_shortkey (string cKey)
	{
		Console.WriteLine(">>> key : "+cKey);
	}
	public virtual void on_change_focus (bool bIsActive)
	{
		Console.WriteLine(">>> focus changed : "+bIsActive);
	}
	
	//////////////////////////////////
	/// callbacks on the sub-icons ///
	//////////////////////////////////
	
	public virtual void on_click_sub_icon (int iClickState, string cIconID)
	{
		Console.WriteLine(">>> click on sub-icon "+cIconID);
	}
	
	public virtual void on_middle_click_sub_icon (string cIconID)
	{
		Console.WriteLine(">>> middle-click on sub-icon "+cIconID);
	}
	
	public virtual void on_scroll_sub_icon (bool bScrollUp, string cIconID)
	{
		Console.WriteLine(">>> scroll on sub-icon "+cIconID);
	}
	
	private void _on_build_menu_sub_icon (string cIconID)
	{
		this.cMenuIconId = cIconID;
		this.on_build_menu_sub_icon (cIconID);
	}
	public virtual void on_build_menu_sub_icon (string cIconID)
	{
		Console.WriteLine(">>> menu on sub-icon "+cIconID);
	}
	
	public virtual void on_menu_select_sub_icon (int iNumEntry, string cIconID)
	{
		Console.WriteLine(">>> menu entry "+iNumEntry+" selected on sub-icon "+cIconID);
	}
	
	public virtual void on_drop_data_sub_icon (string cReceivedData, string cIconID)
	{
		Console.WriteLine(">>> data "+cReceivedData+" dropped on sub-icon "+cIconID);
	}
	
	public virtual void on_answer_sub_icon (System.Object answer, string cIconID)
	{
		Console.WriteLine(">>> answer "+answer+" from sub-icon "+cIconID);
	}
	
	///////////////////////////////
	/// callbacks on the applet ///
	///////////////////////////////
	
	public virtual void begin ()
	{
		
	}
	
	public virtual void end ()
	{
		
	}
	
	private void _on_stop ()
	{
		this.end();
		this.loop.Quit();
	}
	
	public virtual void reload ()
	{
		
	}
	
	private void _on_reload (bool bConfigHasChanged)
	{
		if (bConfigHasChanged)
		{
			this._get_config();
			this.reload();
		}
	}
	
	public virtual void get_config (string cConfFile_path)
	{
		
	}
	
	private void _get_config()
	{
		this.get_config(this.cConfFile);
	}
	
	private void _connect_to_dock ()
	{
		NDesk.DBus.BusG.Init();
		NDesk.DBus.Bus bus = NDesk.DBus.Bus.Session;
		this.icon = bus.GetObject<IApplet> ("org.cairodock.CairoDock", new ObjectPath (this.cBusPath));
		this.icon.on_click 			+= new OnClickEvent (on_click);
		this.icon.on_middle_click 	+= new OnMiddleClickEvent (on_middle_click);
		this.icon.on_scroll 		+= new OnScrollEvent (on_scroll);
		this.icon.on_build_menu 	+= new OnBuildMenuEvent (_on_build_menu);
		this.icon.on_menu_select 	+= new OnMenuSelectEvent (_on_menu_select);
		this.icon.on_drop_data 		+= new OnDropDataEvent (on_drop_data);
		this.icon.on_answer 		+= new OnAnswerEvent (on_answer);
		this.icon.on_answer_dialog 	+= new OnAnswerDialogEvent (on_answer_dialog);
		this.icon._on_stop 			+= new OnStopModuleEvent (_on_stop);
		this.icon._on_reload 		+= new OnReloadModuleEvent (_on_reload);
		
		this.sub_icons = bus.GetObject<ISubApplet>("org.cairodock.CairoDock", new ObjectPath(this.cBusPath + "/sub_icons"));
		this.sub_icons.on_click_sub_icon			+= new OnClickSubIconEvent (on_click_sub_icon);
		this.sub_icons.on_middle_click_sub_icon 	+= new OnMiddleClickSubIconEvent (on_middle_click_sub_icon);
		this.sub_icons.on_scroll_sub_icon 		+= new OnScrollSubIconEvent (on_scroll_sub_icon);
		this.sub_icons.on_build_menu_sub_icon 	+= new OnBuildMenuSubIconEvent (_on_build_menu_sub_icon);
		this.sub_icons.on_menu_select_sub_icon 	+= new OnMenuSelectSubIconEvent (on_menu_select_sub_icon);
		this.sub_icons.on_drop_data_sub_icon 	+= new OnDropDataSubIconEvent (on_drop_data_sub_icon);
		this.sub_icons.on_answer_sub_icon 		+= new OnAnswerSubIconEvent (on_answer_sub_icon);
	}
}
//}

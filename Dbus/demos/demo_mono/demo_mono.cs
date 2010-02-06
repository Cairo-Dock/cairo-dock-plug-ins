//
//
//This is a part of the external demo applet for Cairo-Dock
//
//Copyright : (C) 2010 by Fabounet
//E-mail : fabounet@users.berlios.de
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

//The name of this applet is "demo_mono"; it is placed in a folder named "demo_mono", with a file named "auto-load.conf" which describes it.
//Copy this folder into ~/.config/cairo-dock/third-party to let the dock register it automatically.
//In the folder we have :
//"demo_mono" (the executable script), "demo_mono.conf" (the default config file), "auto-load.conf" (the file describing our applet), "icon" (the default icon of the applet) and "preview" (a preview of this applet)
//
//This very simple applet features a counter from 0 to iMaxValue It displays the counter on the icon with a gauge and a quick info.
//Scroll on the icon increase or decrease the counter.
//The menu offers the possibility to set some default value.
//Left click on the icon will set a random value.
//Middle click on the icon will raise a dialog asking you to set the value you want.
//If you drop some text on the icon, it will be used as the icon's label.

// Compile it with the following command, then rename 'demo_mono.exe' to 'demo_mono'.
//gmcs demo_mono.cs -r:/usr/lib/mono/gtk-sharp-2.0/glib-sharp.dll -r:/usr/lib/cli/ndesk-dbus-glib-1.0/NDesk.DBus.GLib.dll -r:/usr/lib/cli/ndesk-dbus-1.0/NDesk.DBus.dll

  //////////////////////////
 ////// dependancies //////
//////////////////////////
using System;
using GLib;
using NDesk.DBus;

  ////////////////////////////
 ////// DBus Interface //////
////////////////////////////
public delegate void OnClickEvent (int i);
public delegate void OnMiddleClickEvent ();
public delegate void OnScrollEvent (bool b);
public delegate void OnBuildMenuEvent ();
public delegate void OnMenuSelectEvent (int i);
public delegate void OnDropDataEvent (string s);
public delegate void OnChangeFocusEvent (bool b);
public delegate void OnAnswerEvent (GLib.Value v);
public delegate void OnStopModuleEvent ();
public delegate void OnReloadModuleEvent (bool b);
[NDesk.DBus.Interface ("org.cairodock.CairoDock.applet")]
public interface IIcon
{
	GLib.Value Get (string s);
	void GetAll ();
	void SetLabel (string s);
	void SetIcon (string s);
	void SetEmblem (string s, int i);
	void Animate (string s, int i);
	void SetQuickInfo (string s);
	void ShowDialog (string s, int i);
	void AskQuestion (string s);
	void AskValue (string s, double x, double y);
	void AskText (string s, string t);
	void AddDataRenderer (string s, int i, string t);
	void RenderValues (double[] x);
	void ControlAppli (string s);
	void PopulateMenu (string[] s);
	event OnClickEvent on_click; 
	event OnMiddleClickEvent on_middle_click; 
	event OnScrollEvent on_scroll; 
	event OnBuildMenuEvent on_build_menu; 
	event OnMenuSelectEvent on_menu_select; 
	event OnDropDataEvent on_drop_data; 
	event OnChangeFocusEvent on_change_focus; 
	event OnAnswerEvent on_answer; 
	event OnStopModuleEvent on_stop_module; 
	event OnReloadModuleEvent on_reload_module; 
}

  ////////////////////////////////////////
 ////// callbacks on the sub-icons //////
////////////////////////////////////////


  /////////////////////////////////////
 ////// callbacks on the applet //////
/////////////////////////////////////


public class Applet
{
	public static string applet_name = "demo_mono";  // the name of the applet must the same as the folder.
	public static string applet_path = "/org/cairodock/CairoDock/"+applet_name;  // path where our object is stored on the bus.
	public IIcon icon = null;
	private int count = 0;
	
	public void begin()
	{
		connect_to_dock ();
		this.icon.ShowDialog ("pouet pouet", 5);
		this.icon.SetQuickInfo ("123");
	}
	public void connect_to_dock()
	{
		BusG.Init ();
		Bus bus = Bus.Session;
		this.icon = bus.GetObject<IIcon> ("org.cairodock.CairoDock", new ObjectPath (applet_path));
		this.icon.on_click 			+= new OnClickEvent (action_on_click);
		this.icon.on_middle_click 	+= new OnMiddleClickEvent (action_on_middle_click);
		this.icon.on_scroll 		+= new OnScrollEvent (action_on_scroll);
		this.icon.on_build_menu 	+= new OnBuildMenuEvent (action_on_build_menu);
		this.icon.on_menu_select 	+= new OnMenuSelectEvent (action_on_menu_select);
		this.icon.on_drop_data 		+= new OnDropDataEvent (action_on_drop_data);
		this.icon.on_change_focus 	+= new OnChangeFocusEvent (action_on_change_focus);
		this.icon.on_answer 		+= new OnAnswerEvent (action_on_answer);
	}
	
	  ////////////////////////////////////////
	 ////// callbacks on the main icon //////
	////////////////////////////////////////
	private void action_on_click (int iClickState)
	{
		Console.WriteLine(">>> click");
	}
	private void action_on_middle_click ()
	{
		Console.WriteLine(">>> middle click");
		this.icon.AskValue("Set the value you want", this.count, 100);
	}
	private void action_on_scroll (bool bScrollUp)
	{
		Console.WriteLine(">>> scroll up " + bScrollUp);
		int n;
		if (bScrollUp)
			n = Math.Min(100, this.count+1);
		else
			n = Math.Max(0, this.count-1);
		this.set_counter(n);
	}
	private void action_on_build_menu ()
	{
		Console.WriteLine(">>> build menu");
	}
	private void action_on_menu_select (int iNumEntry)
	{
		Console.WriteLine(">>> select entry : "+iNumEntry);
		if (iNumEntry == 0)
			this.set_counter(0);
		else if (iNumEntry == 1)
			this.set_counter(100/2);
		else if (iNumEntry == 2)
			this.set_counter(100);
	}
	private void action_on_drop_data (string cReceivedData)
	{
		Console.WriteLine(">>> drop : "+cReceivedData);
		this.icon.SetLabel(cReceivedData);
	}
	private void action_on_change_focus (bool b)
	{
		Console.WriteLine(">>> focus changed : ",b);
	}
	private void action_on_answer (GLib.Value answer)
	{
		Console.WriteLine(">>> answer : ");
	}
	public void set_counter(int n)
	{
		this.count = n;
		double[] percent = new double [1];
		percent[0] = n/100.0;
		this.icon.RenderValues(percent);
		this.icon.SetQuickInfo(String.Format(n.ToString()));
	}
	
	  //////////////////
	 ////// main //////
	//////////////////
	public static void Main ()
	{
		Console.WriteLine(">>> main");
		GLib.MainLoop loop = new GLib.MainLoop();
		Applet myApplet = new Applet ();
		myApplet.begin();
		loop.Run();
		Console.WriteLine(">>> bye");
	}
}



// Compile it with the following command, then rename 'demo_mono.exe' to 'demo_mono'.
// gmcs demo_mono.cs -pkg:glib-sharp-2.0 -pkg:ndesk-dbus-1.0 -pkg:ndesk-dbus-glib-1.0

  //////////////////////////
 ////// dependancies //////
//////////////////////////
using System;
using System.Collections.Generic;
using System.IO;  // Path, Directory
using GLib;
using NDesk.DBus;

  ////////////////////////////
 ////// DBus Interface //////
////////////////////////////
public delegate void OnBuildMenuEvent ();
public delegate void OnStopModuleEvent ();
[NDesk.DBus.Interface ("org.cairodock.CairoDock.applet")]
public interface IIcon
{
	void AddMenuItems(Dictionary<string, object>[] pItems);
	event OnBuildMenuEvent on_build_menu;
	event OnStopModuleEvent on_stop_module;
}

  ////////////////////
 ////// Applet //////
////////////////////
public class Applet
{
	public static string applet_name = Path.GetFileName(Directory.GetCurrentDirectory());
	public static string applet_path = "/org/cairodock/CairoDock/"+applet_name;  // path where our object is stored on the bus.
	public IIcon myIcon = null;
	private GLib.MainLoop loop = null;
	
	public void begin()
	{
		connect_to_dock ();
		loop = new GLib.MainLoop();
		loop.Run();
	}
	public void action_on_stop_module()
	{
		loop.Quit();
	}
	public void connect_to_dock()
	{
		BusG.Init ();
		Bus bus = Bus.Session;
		myIcon = bus.GetObject<IIcon> ("org.cairodock.CairoDock", new ObjectPath (applet_path));
		myIcon.on_build_menu 	+= new OnBuildMenuEvent (action_on_build_menu);
		myIcon.on_stop_module 	+= new OnStopModuleEvent (action_on_stop_module);
	}
	
	private void action_on_build_menu ()
	{
		Console.WriteLine(">>> build menu");
		Dictionary<string, object>[] pItems = new Dictionary<string, object>[] {
			new Dictionary<string, object>()
			{
				{"label", "set min value"},
				{"icon", "gtk-zoom-out"},
				{"id", 1}
			},
			new Dictionary<string, object>()
			{
				{"label", "set medium value"},
				{"icon", "gtk-zoom-fit"},
				{"id", 2}
			},
			new Dictionary<string, object>()
			{
				{"label", "set max value"},
				{"icon", "gtk-zoom-in"},
				{"id", 3}
			}
		};
		Console.WriteLine("*** item 1:"+pItems[1]["icon"]);
		myIcon.AddMenuItems(pItems);
	}
	
	  //////////////////
	 ////// main //////
	//////////////////
	public static void Main ()
	{
		Applet myApplet = new Applet ();
		myApplet.begin();
		Console.WriteLine(">>> bye");
	}
}


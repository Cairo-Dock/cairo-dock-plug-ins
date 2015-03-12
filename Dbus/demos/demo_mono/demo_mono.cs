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
// gmcs demo_mono.cs -r:/usr/lib/cli/CDApplet.dll
// ln -s demo_mono.exe demo_mono

  //////////////////////////
 ////// dependancies //////
//////////////////////////
using System;
using System.Collections.Generic;
using CairoDock.Applet;

public struct Config {
	public string cTheme;
	public int iMaxValue;
	public bool yesno;
}

////////////////////
/// Applet class ///
////////////////////
public class Applet : CDApplet
{
	private Config config;
	private int count = 0;
	
	public Applet()
	{
		
	}
	
	  ////////////////////////////
	 ////// private methods //////
	////////////////////////////
	
	private void set_counter(int n)
	{
		this.count = n;
		this.icon.RenderValues(new double[] {(double)n/this.config.iMaxValue});
		this.icon.SetQuickInfo(String.Format(n.ToString()));
	}
	
	  ///////////////////////////////
	 ////// applet definition //////
	///////////////////////////////
	
	public override void get_config (string cConfFilePath)
	{
		/// read cConfFilePath...
		this.config.iMaxValue = 100;
		this.config.cTheme = "Turbo-night";
		this.config.yesno = true;
	}
	
	public override void end()
	{
		Console.WriteLine("*** Bye !");
	}
	
	public override void begin()
	{
		this.icon.ShowDialog ("I'm connected to Cairo-Dock !", 4);
		this.icon.AddDataRenderer("gauge", 1, this.config.cTheme);
		this.set_counter (0);
	}
	
	public override void reload()
	{
		this.icon.AddDataRenderer("gauge", 1, this.config.cTheme);
		this.set_counter (Math.Min (this.count, this.config.iMaxValue));
	}
	
	  ////////////////////////////////////////
	 ////// callbacks on the main icon //////
	////////////////////////////////////////
	
	public override void on_click (int iClickState)
	{
		Console.WriteLine("*** click");
	}
	
	public override void on_middle_click ()
	{
		Console.WriteLine("*** middle click");
		Dictionary<string, object> dialog_attributes = new Dictionary<string, object> () {
			{"icon" , "stock_properties"},
			{"message" , "Set the value you want"},
			{"buttons" , "ok;cancel"} };
		Dictionary<string, object> widget_attributes = new Dictionary<string, object> () {
			{"widget-type" , "scale"},
			{"max-value" , this.config.iMaxValue},
			{"message" , "Set the value you want"} };
		
		this.icon.PopupDialog(dialog_attributes, widget_attributes);
	}
	
	public override void on_scroll (bool bScrollUp)
	{
		Console.WriteLine("*** scroll up " + bScrollUp);
		int n;
		if (bScrollUp)
			n = Math.Min(100, this.count+1);
		else
			n = Math.Max(0, this.count-1);
		this.set_counter(n);
	}
	
	public override void on_build_menu ()
	{
		Console.WriteLine("*** build menu");
		/// Warning : the AddMenuItems fails with DBus Sharp 0.6.0; until this is fixed, use the PopulateMenu method.
		/**Dictionary<string, object>[] pItems = new Dictionary<string, object>[] {
			new Dictionary<string, object>()
			{
				{"label", "set min value"},
				{"icon", "gtk-zoom-out"},
				{"id", 0}
			},
			new Dictionary<string, object>()
			{
				{"label", "set medium value"},
				{"icon", "gtk-zoom-fit"},
				{"id", 1}
			},
			new Dictionary<string, object>()
			{
				{"label", "set max value"},
				{"icon", "gtk-zoom-in"},
				{"id", 2}
			}
		};
		Console.WriteLine("*** item 1:"+pItems[0]["icon"]);
		this.icon.AddMenuItems(pItems);*/
		this.icon.PopulateMenu(new string[] {"set min value", "set medium value", "set max value"});
	}
	
	public override void on_menu_select (int iNumEntry)
	{
		Console.WriteLine("*** select entry : "+iNumEntry);
		if (iNumEntry == 0)
			this.set_counter(0);
		else if (iNumEntry == 1)
			this.set_counter(this.config.iMaxValue/2);
		else if (iNumEntry == 2)
			this.set_counter(this.config.iMaxValue);
	}
	public override void on_drop_data (string cReceivedData)
	{
		Console.WriteLine("*** drop : "+cReceivedData);
		this.icon.SetLabel(cReceivedData);
	}
	public override void on_answer_dialog (int iButton, System.Object answer)
	{
		Console.WriteLine("*** answer : "+(double)answer);
		if (iButton == 0)
		{
			double x = (double)answer;
			this.set_counter((int) x);
		}
	}
	
	  //////////////////
	 ////// main //////
	//////////////////
	public static void Main (string[] args)
	{
		Applet myApplet = new Applet();
		myApplet.run();
	}
}


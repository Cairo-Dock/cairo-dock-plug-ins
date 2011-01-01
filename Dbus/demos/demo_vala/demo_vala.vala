/* This is a part of the external demo applet for Cairo-Dock

Copyright : (C) 2010 by Fabounet
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

/// The name of this applet is "demo_vala"; it is placed in a folder named "demo_vala", with a file named "auto-load.conf" which describes it.
/// Copy this folder into ~/.config/cairo-dock/third-party to let the dock register it automatically.
/// In the folder we have :
/// "demo_vala" (the executable script), "demo_vala.conf" (the default config file), "auto-load.conf" (the file describing our applet), "icon" (the default icon of the applet) and "preview" (a preview of this applet)

/// This very simple applet features a counter from 0 to iMaxValue It displays the counter on the icon with a gauge and a quick info.
/// Scroll on the icon increase or decrease the counter.
/// The menu offers the possibility to set some default value.
/// Left click on the icon will set a random value.
/// Middle click on the icon will raise a dialog asking you to set the value you want.
/// If you drop some text on the icon, it will be used as the icon's label.

/// Compile it with (you may have to install valac) :
/// valac --pkg CDApplet -o demo_vala demo_vala.vala
/// ou
/// valac -q -C --disable-warnings --pkg CDApplet demo_vala.vala
/// gcc -o demo_vala $(pkg-config --cflags --libs CDApplet) demo_vala.c


  /////////////////////////
 ///// dependancies //////
/////////////////////////
using GLib;
using CairoDock.Applet;

struct Config {
    public string cTheme;
    public int iMaxValue;
    public bool yesno;
}

public class MyApplet : CDApplet
{
	// my config.
	private Config config;
	// my data.
	private int count;
	
	public MyApplet()
	{
		//this.config = Config();
		base(null);
	}
	
	  ///////////////////////
	 /// private methods ///
	///////////////////////
	
	private void set_counter(int count)
	{
		this.count = count;
		double[] percent = {1.0*this.count/this.config.iMaxValue};
		base.icon.RenderValues(percent);
		this.icon.SetQuickInfo("%d".printf(this.count));
	}
	
	  ////////////////////////////////////////
	 ////// callbacks on the main icon //////
	////////////////////////////////////////
	public override void on_click(int iState)
	{
		print ("*** clic !\n");
		set_counter(GLib.Random.int_range(0,this.config.iMaxValue+1));
	}
	public override void on_middle_click()
	{
		print ("*** middle clic !\n");
		//this.icon.AskValue("Set the value you want", (double)this.count, (double)this.config.iMaxValue);
		HashTable<string,Variant>dialog_attributes = new HashTable<string,Variant>(str_hash, str_equal);
		dialog_attributes.insert("icon", "stock_properties");
		dialog_attributes.insert("message", "Set the value you want");
		dialog_attributes.insert("buttons", "ok;cancel");
		HashTable<string,Variant> widget_attributes = new HashTable<string,Variant>(str_hash, str_equal);  // even if you don't have widget attributes, you must fill the hash-table with at least 1 value, otherwise vala will crash :-/
		widget_attributes.insert("widget-type","scale");
		widget_attributes.insert("max-value",this.config.iMaxValue);
		widget_attributes.insert("message","Set the value you want");
		this.icon.PopupDialog(dialog_attributes, widget_attributes);
	}
	public override void on_build_menu()
	{
		print ("*** build menu !\n");
		HashTable<string,Variant>[] pItems = {};
		HashTable<string,Variant> pItem;
		
		pItem = new HashTable<string,Variant?>(str_hash, str_equal);
		pItem.insert("label", "set min value");
		pItem.insert("icon", "gtk-zoom-out");
		pItem.insert("id", 0);
		pItems += pItem;
		
		pItem = new HashTable<string,Variant?>(str_hash, str_equal);
		pItem.insert("label", "set medium value");
		pItem.insert("icon", "gtk-zoom-fit");
		pItem.insert("id", 1);
		pItems += pItem;
		
		pItem = new HashTable<string,Variant?>(str_hash, str_equal);
		pItem.insert("label", "set max value");
		pItem.insert("icon", "gtk-zoom-in");
		pItem.insert("id", 2);
		pItems += pItem;
		
		this.icon.AddMenuItems(pItems);
	}
	public override void on_menu_select(int iNumEntry)
	{
		print ("*** choice %d has been selected !\n", iNumEntry);
		if (iNumEntry == 0)
			this.set_counter(0);
		else if (iNumEntry == 1)
			this.set_counter(this.config.iMaxValue/2);
		else if (iNumEntry == 2)
			this.set_counter(this.config.iMaxValue);
	}
	public override void on_scroll(bool bScrollUp)
	{
		print ("*** scroll !\n");
		int count;
		if (bScrollUp)
			count = int.min(this.config.iMaxValue, this.count+1);
		else
			count = int.max(0, this.count-1);
		this.set_counter(count);
	}
	public override void on_drop_data(string cReceivedData)
	{
		print ("*** received : %s\n",cReceivedData);
		this.icon.SetLabel(cReceivedData);
	}
	public override void on_answer_dialog(int iButton, Variant answer)
	{
		print ("*** answer : %d\n",(int)answer.get_double());
		if (iButton == 0)  // ok
			this.set_counter((int)answer.get_double());
	}
	
	  ////////////////////////////////////////
	 ////// callbacks on the sub-icons //////
	////////////////////////////////////////
	public override void on_click_sub_icon(int iState, string cIconID)
	{
		print ("clic on the sub-icon '%s' !\n", cIconID);
	}
	
	  ///////////////////////////////
	 ////// applet definition //////
	///////////////////////////////
	public override void end()
	{
		print ("*** our module is stopped\n");
	}

	public override void reload()
	{
		this.icon.AddDataRenderer("gauge", 1, this.config.cTheme);
		double[] percent = {1.0*this.count/this.config.iMaxValue};
		this.icon.RenderValues(percent);
		this.sub_icons.RemoveSubIcon("any");
		string[] subicons = {"icon 1", "firefox-3.0", "id1", "icon 3", "thunderbird", "id3", "icon 4", "nautilus", "id4"};
		this.sub_icons.AddSubIcons(subicons);
	}
	public override void get_config(GLib.KeyFile keyfile)
	{
		this.config.cTheme 	= keyfile.get_string("Configuration", "theme");
		print ("cTheme: %s\n", this.config.cTheme);
		this.config.iMaxValue 	= keyfile.get_integer("Configuration", "max value");
		this.config.yesno 	= keyfile.get_boolean("Configuration", "yesno");
	}
	public override void begin()
	{
		this.icon.ShowDialog("I'm connected to Cairo-Dock !", 4);  // show a dialog with this message for 4 seconds.
		this.icon.AddDataRenderer("gauge", 1, this.config.cTheme);  // set a gauge with the theme read in config to display the value of the counter.
		double[] percent  = {1.0*this.count/this.config.iMaxValue};
		this.icon.RenderValues(percent);  // draw the gauge with an initial value.
		string[] subicons ={"icon 1", "firefox-3.0", "id1", "icon 2", "trash", "id2", "icon 3", "thunderbird", "id3", "icon 4", "nautilus", "id4"};
		this.sub_icons.AddSubIcons(subicons);  // add 4 icons in our sub-dock. The tab contains triplets of {label, image, ID}.
		this.sub_icons.RemoveSubIcon("id2");  // remove the 2nd icon of our sub-dock.
		this.sub_icons.SetQuickInfo("1", "id1");  // write the ID on each icon of the sub-dock.
		this.sub_icons.SetQuickInfo("3", "id3");
		this.sub_icons.SetQuickInfo("4", "id4");
		print ("DEMO %s\n", this.config.cTheme);
	}
}

  //////////////////
 ////// main //////
//////////////////
static int main (string[] args)
{	
	var myApplet = new MyApplet();
	myApplet.run();
	return 0;
}

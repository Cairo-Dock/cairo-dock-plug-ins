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
/// valac --disable-dbus-transformation --pkg dbus-glib-1 -o demo_vala demo_vala.vala

  /////////////////////////
 ///// dependancies //////
/////////////////////////
using GLib;

struct Config {
    public string cTheme;
    public int iMaxValue;
    public bool yesno;
}

public class Applet : GLib.Object
{
	// internal data.
	private dynamic DBus.Object icon;
	private dynamic DBus.Object sub_icons;
	private MainLoop loop;
	private string applet_name;
	private string conf_file;
	// my config.
	private Config config;
	// my data.
	private int count;
	
	public Applet()
	{
		this.applet_name = GLib.Path.get_basename(GLib.Environment.get_current_dir());  // the name of the applet must the same as the folder.
		this.conf_file = GLib.Environment.get_home_dir()+"/.config/cairo-dock/current_theme/plug-ins/"+applet_name+"/"+applet_name+".conf";  // path to the conf file of our applet.
		this.config = Config();
	}
	
	public void run()
	{
		this.connect_to_bus();
		this.get_config();
		this.load();
		this.loop = new MainLoop(null, false);
		this.loop.run();
	}
	
	  ////////////////////////////////////////
	 ////// callbacks on the main icon //////
	////////////////////////////////////////
	private void action_on_click(dynamic DBus.Object myIcon, int iState)
	{
		print (">>> clic !\n");
		set_counter(GLib.Random.int_range(0,this.config.iMaxValue+1));
	}
	private void action_on_middle_click(dynamic DBus.Object myIcon)
	{
		print (">>> middle clic !\n");
		myIcon.AskValue("Set the value you want", (double)this.count, (double)this.config.iMaxValue);
	}
	private void action_on_build_menu(dynamic DBus.Object myIcon)
	{
		print (">>> build menu !\n");
		string[] entries = {"set min value", "set medium value", "set max value"};
		myIcon.PopulateMenu(entries);
	}
	private void action_on_menu_select(dynamic DBus.Object myIcon, int iNumEntry)
	{
		print (">>> choice %d has been selected !\n", iNumEntry);
		if (iNumEntry == 0)
			this.set_counter(0);
		else if (iNumEntry == 1)
			this.set_counter(this.config.iMaxValue/2);
		else if (iNumEntry == 2)
			this.set_counter(this.config.iMaxValue);
	}
	private void action_on_scroll(dynamic DBus.Object myIcon, bool bScrollUp)
	{
		print (">>> scroll !\n");
		if (bScrollUp)
			count = int.min(this.config.iMaxValue, this.count+1);
		else
			count = int.max(0, this.count-1);
		this.set_counter(count);
	}
	private void action_on_drop_data(dynamic DBus.Object myIcon, string cReceivedData)
	{
		print (">>> received : %s\n",cReceivedData);
		this.icon.SetLabel(cReceivedData);
	}
	private void action_on_answer(dynamic DBus.Object myIcon, Value answer)
	{
		print (">>> answer : %d\n",(int)answer.get_double());
		this.set_counter((int)answer.get_double());
	}
	  ////////////////////////////////////////
	 ////// callbacks on the sub-icons //////
	////////////////////////////////////////
	private void action_on_click_sub_icon(dynamic DBus.Object mySubIcons, int iState, string cIconID)
	{
		print ("clic on the sub-icon '%s' !\n", cIconID);
	}
	  /////////////////////////////////////
	 ////// callbacks on the applet //////
	/////////////////////////////////////
	private void action_on_stop(dynamic DBus.Object myIcon)
	{
		print (">>> our module is stopped\n");
		loop.quit();
	}

	private void action_on_reload(dynamic DBus.Object myIcon, bool bConfigHasChanged)
	{
		print (">>> our module is reloaded");
		if (bConfigHasChanged)
		{
			print (">>>  and our config has changed");
			this.get_config();
			this.icon.AddDataRenderer("gauge", 1, this.config.cTheme);
			double[] percent = {1.0*this.count/this.config.iMaxValue};
			this.icon.RenderValues(percent);
			this.sub_icons.RemoveSubIcon("any");
			string[] subicons = {"icon 1", "firefox-3.0", "id1", "icon 3", "thunderbird", "id3", "icon 4", "nautilus", "id4"};
			this.sub_icons.AddSubIcons(subicons);
		}
	}
	  //////////////////////////////////////
	 ////// get our applet on the bus /////
	//////////////////////////////////////
	private void connect_to_bus()
	{
		string applet_path = "/org/cairodock/CairoDock/"+applet_name;  // path where our object is stored on the bus.
		DBus.Connection bus = DBus.Bus.get (DBus.BusType.SESSION);
		if (bus == null)
		{
			GLib.error (">>> module '%s' can't be found on the bus, exit.", this.applet_name);
		}
		this.icon = bus.get_object ("org.cairodock.CairoDock",
			applet_path,
			"org.cairodock.CairoDock.applet");  // this object represents our applet and also our icon. It can be either in a dock or in a desklet, we don't have to care.

		////// we'll have a sub-dock, so we also get the sub-icons object //////
		this.sub_icons = bus.get_object ("org.cairodock.CairoDock",
			applet_path+"/sub_icons",
			"org.cairodock.CairoDock.subapplet");  // this object represents the list of icons contained in our sub-dock, or in our desklet. We'll add them one by one later, giving them a unique ID, which will be used to identify each of them.
		
		this.icon.on_click += action_on_click;  // when the user left-clicks on our icon.
		this.icon.on_middle_click += action_on_middle_click;  // when the user middle-clicks on our icon.
		this.icon.on_build_menu += action_on_build_menu;  // when the user right-clicks on our applet (which builds the menu)
		this.icon.on_menu_select += action_on_menu_select;  // when the user selects an entry of this menu.
		this.icon.on_scroll += action_on_scroll;  // when the user scroll up or down on our icon.
		this.icon.on_drop_data += action_on_drop_data;  // when the user drops something on our icon.
		this.icon.on_answer += action_on_answer;  // when the user answer a question.
		this.icon.on_stop_module += action_on_stop;  // when the user deactivate our applet (or the DBus plug-in, or when the Cairo-Dock is stopped).
		this.icon.on_reload_module += action_on_reload;  // when the user changes something in our config, or when the desklet is resized (with no change in the config).
		this.sub_icons.on_click_sub_icon += action_on_click_sub_icon;  // when the user left-clicks on a sub-icon.
	}
	private void get_config()
	{
		GLib.KeyFile keyfile = new GLib.KeyFile();
		keyfile.load_from_file(this.conf_file, GLib.KeyFileFlags.NONE);
		
		this.config.cTheme 	= keyfile.get_string("Configuration", "theme");
		this.config.iMaxValue 	= keyfile.get_integer("Configuration", "max value");
		this.config.yesno 	= keyfile.get_boolean("Configuration", "yesno");
	}
	private void load()
	{
		this.icon.ShowDialog("I'm connected to Cairo-Dock !", 4);  // show a dialog with this message for 4 seconds.
		this.icon.SetQuickInfo("%d".printf(this.count));  // write the counter value on the icon.
		this.icon.AddDataRenderer("gauge", 1, this.config.cTheme);  // set a gauge with the theme read in config to display the value of the counter.
		double[] percent  = {1.0*this.count/this.config.iMaxValue};
		this.icon.RenderValues(percent);  // draw the gauge with an initial value.
		string[] subicons ={"icon 1", "firefox-3.0", "id1", "icon 2", "trash", "id2", "icon 3", "thunderbird", "id3", "icon 4", "nautilus", "id4"};
		this.sub_icons.AddSubIcons(subicons);  // add 4 icons in our sub-dock. The tab contains triplets of {label, image, ID}.
		this.sub_icons.RemoveSubIcon("id2");  // remove the 2nd icon of our sub-dock.
		this.sub_icons.SetQuickInfo("1", "id1");  // write the ID on each icon of the sub-dock.
		this.sub_icons.SetQuickInfo("2", "id2");
		this.sub_icons.SetQuickInfo("3", "id3");
		this.sub_icons.SetQuickInfo("4", "id4");
	}
	private void set_counter(int count)
	{
		this.count = count;
		double[] percent = {1.0*this.count/this.config.iMaxValue};
		this.icon.RenderValues(percent);
		this.icon.SetQuickInfo("%d".printf(this.count));
	}
}

  //////////////////
 ////// main //////
//////////////////
static int main (string[] args)
{	
	var myApplet = new Applet();
	myApplet.run();
	print("bye\n");
	return 0;
}

### We assume the name of this applet is "demo"
### It is installed in a directory, like for instance ~/demo.
### This script should be placed in this folder, alongside with 3 files :
### demo.conf (the default config file), "icon" (the default icon of the applet) and "preview" (a preview of this applet)

### import ###
import sys
import gobject
import glib
import gtk
import dbus
import os.path
from dbus.mainloop.glib import DBusGMainLoop


### let's connect to the dock.###
DBusGMainLoop(set_as_default=True)
bus = dbus.SessionBus()
try:
	dock_object = bus.get_object("org.cairodock.CairoDock",
			"/org/cairodock/CairoDock")
except dbus.DBusException:
	print "Cairo-Dock not found on bus (did you activate its 'DBus' plug-in ?)"
	sys.exit(1)
dock_iface = dbus.Interface(dock_object, "org.cairodock.CairoDock")


### let's register our applet !###
applet_name=os.path.basename(os.path.abspath(".")) # for instance; we can give any name as long as the .conf file follows it.
applet_share_data_dir=os.path.abspath(".")
dock_iface.RegisterNewModule(applet_name, 3, "This is a distant applet\nIt simulate a counter\nScroll up/down to increase/decrease the counter,\nClick/middle-click to increase/decrease the counter by 10\nDrop some text to set it as the label.\n  by Fabounet", applet_share_data_dir)

try:
	applet_object = bus.get_object("org.cairodock.CairoDock",
			"/org/cairodock/CairoDock/"+applet_name)
except dbus.DBusException:
	print "the '"+applet_name+"' module has not been registerd"
	sys.exit(2)
applet_iface = dbus.Interface(applet_object, "org.cairodock.CairoDock.applet")
	

### init ###
def init():
	# register to the notifications on our applet
	applet_iface.connect_to_signal("on_click_icon", action_on_click)
	applet_iface.connect_to_signal("on_middle_click_icon", action_on_middle_click)
	applet_iface.connect_to_signal("on_build_menu", action_on_build_menu)
	applet_iface.connect_to_signal("on_menu_select", action_on_menu_select)
	applet_iface.connect_to_signal("on_scroll_icon", action_on_scroll)
	applet_iface.connect_to_signal("on_drop_data", action_on_drop_data)
	applet_iface.connect_to_signal("on_init_module", action_on_init)
	applet_iface.connect_to_signal("on_stop_module", action_on_stop)
	applet_iface.connect_to_signal("on_reload_module", action_on_reload)


### stop ###
def stop():
	print "STOP"
	dock_iface.UnregisterModule(applet_name)
	del dock_object
	del dock_iface
	del applet_object
	del applet_iface


################################################
### Add your own code to the functions below ###
################################################
### global vars ###
count=0

### callbacks ###
def action_on_click(iState):
	print "clic !"
	count += 10
	applet_iface.SetQuickInfo(""+count)

def action_on_middle_click():
	print "middle clic !"
	count -= 10
	applet_iface.SetQuickInfo(""+count)

def action_on_build_menu():
	print "build menu !"
	applet_iface.PopulateMenu(["set min value", "set medium value", "set max value"])
	
def action_on_menu_select(iNumEntry):
	print "choice",iNumEntry,"has been selected !"
	if iNumEntry == 0:
		count = 0
	else if iNumEntry == 1:
		count = 50
	else if iNumEntry == 2:
		count = 100
	applet_iface.SetQuickInfo(""+count)

def action_on_scroll(bScrollUp):
	if bScrollUp:
		count ++
	else
		count --
	applet_iface.SetQuickInfo(""+count)

def action_on_drop_data(cReceivedData):
	print "received",cReceivedData
	applet_iface.SetLabel(cReceivedData)
	

def action_on_init():
	print "our module is started"
	count=1
	applet_iface.ShowDialog("I'm connected to Cairo-Dock !", 4)
	applet_iface.SetQuickInfo(""+count)
	applet_iface.AddSubIcons(["icon 1", "firefox-3.0", "echo pouet", "icon 2", "trash", "abc", "icon 3", "thunderbird", "def"])
	applet_iface.RemoveSubIcon("abc")
	
def action_on_stop():
	print "our module is stopped"
	count=0
	
def action_on_reload(bConfigHasChanged):
	print "our module is reloaded"
	if bConfigHasChanged:
		print " and our config has changed"
	

### main ###
if __name__ == '__main__':
	init()
	gtk.main()
	stop()
	sys.exit(0)

### We assume the name of this applet is "demo"
### It is installed in a directory, like for instance ~/demo.
### This script should be placed in this folder, alongside with 2 files :
### "icon" (the default icon of the applet) and "preview" (a preview of this applet)

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
dock_iface.RegisterNewModule("demo", 3, "This is a distant applet\nIt handles right and middle click\n  by Fabounet", os.path.abspath("."))

try:
	applet_object = bus.get_object("org.cairodock.CairoDock",
			"/org/cairodock/CairoDock/demo")
except dbus.DBusException:
	print "the 'demo' module has not been registerd"
	sys.exit(2)
applet_iface = dbus.Interface(applet_object, "org.cairodock.CairoDock.applet")
	

### init ###
def init():
	# register to the notifications on our applet
	applet_iface.connect_to_signal("on_click_icon", action_on_click)
	applet_iface.connect_to_signal("on_middle_click_icon", action_on_middle_click)
	applet_iface.connect_to_signal("on_build_menu", action_on_build_menu)
	applet_iface.connect_to_signal("on_menu_select", action_on_menu_select)
	applet_iface.connect_to_signal("on_drop_data", action_on_drop_data)
	applet_iface.connect_to_signal("on_init_module", action_on_init)
	applet_iface.connect_to_signal("on_stop_module", action_on_stop)
	applet_iface.connect_to_signal("on_reload_module", action_on_reload)
	
	# a little test
	applet_iface.ShowDialog("I'm connected to Cairo-Dock !", 4)
	applet_iface.SetQuickInfo("123")
	applet_iface.AddSubIcons(["icon 1", "firefox-3.0", "echo pouet", "icon 2", "thunderbird", "echo pouic"])


### stop ###
def stop():
	print "STOP"
	bus = dbus.SessionBus()
	remote_object = bus.get_object("org.cairodock.CairoDock", "/org/cairodock/CairoDock")
	iface = dbus.Interface(remote_object, "org.cairodock.CairoDock")
	iface.UnregisterModule("demo")
	del iface
	del remote_object
	del bus
	

### callbacks ###
def action_on_click(iState):
	print "clic !"

def action_on_middle_click():
	print "middle clic !"

def action_on_build_menu():
	print "build menu !"
	applet_iface.PopulateMenu(["choice 0", "choice 1"])
	
def action_on_menu_select(iNumEntry):
	print "choice",iNumEntry,"has been selected !"
	
def action_on_drop_data(cReceivedData):
	print "received",cReceivedData
	

def action_on_init():
	print "our module is started"
	
def action_on_stop():
	print "our module is stopped"
	
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

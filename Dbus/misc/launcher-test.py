#!/usr/bin/env python
# A simple test case.

from gi.repository import Unity, Gio, GObject, Dbusmenu

loop = GObject.MainLoop()

# Pretend to be evolution for the sake of the example

launcher = Unity.LauncherEntry.get_for_desktop_id ("thunderbird.desktop")

# Show a count of 123 on the icon

launcher.set_property("count", 123)

launcher.set_property("count_visible", True)

# Set progress to 40%

launcher.set_property("progress", 0.40)

launcher.set_property("progress_visible", True)


#launcher.set_property("emblem", "play")

#launcher.set_property("emblem_visible", True)


# We also want a quicklist

def on_item_selected(self, menuitem, index):
	print "selected",index
	
ql = Dbusmenu.Menuitem.new ()

item1 = Dbusmenu.Menuitem.new ()

item1.property_set (Dbusmenu.MENUITEM_PROP_LABEL, "Item 1")

item1.property_set_bool (Dbusmenu.MENUITEM_PROP_VISIBLE, True)

item1.connect ("item-activated", on_item_selected, 1)

item2 = Dbusmenu.Menuitem.new ()

item2.property_set (Dbusmenu.MENUITEM_PROP_LABEL, "Item 2")

item2.property_set_bool (Dbusmenu.MENUITEM_PROP_VISIBLE, True)

item2.connect ("item-activated", on_item_selected, 2)

ql.child_append (item1)

ql.child_append (item2)

launcher.set_property("quicklist", ql)

n = 3
prev_item = item2

def update_urgency():
	global prev_item, n
	if launcher.get_property("urgent"):
		print "Removing urgent flag"
		launcher.set_property("urgent", False)
	else:
		print "setting urgent flag"
		launcher.set_property("urgent", True)

	if prev_item == None:
		item = Dbusmenu.Menuitem.new ()
		
		item.property_set (Dbusmenu.MENUITEM_PROP_LABEL, "Item "+str(n))
		
		item.property_set_bool (Dbusmenu.MENUITEM_PROP_VISIBLE, True)
		
		item.connect ("item-activated", on_item_selected, n)
		ql.child_append (item)
		prev_item = item
	else:
		ql.child_delete (prev_item)
		prev_item = None
		item1.property_set (Dbusmenu.MENUITEM_PROP_LABEL, "Item 1  (%d)"%n)
		n = n+1
	
	return True

GObject.timeout_add_seconds(4, update_urgency)

loop.run()

project(
	'testtray',
	'c',
	version: '0.0.1',
	license: 'GPL2+',
	meson_version: '>=1.0.0',
	default_options: [
		'c_std=c11',
		'warning_level=2',
		'werror=false',
	],
)

cairo    = dependency('cairo')
glib     = dependency('glib-2.0')
gobject  = dependency('gobject-2.0')
gtk      = dependency('gtk+-3.0')
gdk      = dependency('gdk-3.0')
X11      = dependency('x11')

testtray_sources = ['testtray.c', 'na-tray.c', 'na-tray-manager.c', 'na-tray-child.c', 'na-marshal.c', 'fixedtip.c']
tt1 = executable('testtray', testtray_sources,
        dependencies: [cairo, glib, gobject, gtk, gdk, X11],
        install: false)

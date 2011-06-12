# This is a part of the StatusNotifier applet for Cairo-Dock
# Copyright : (C) 2011 by Fabounet
# E-mail : fabounet@glx-dock.org
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
# http://www.gnu.org/licenses/licenses.html#GPL

from distutils.core import setup
setup(name='status-notifier-watcher',
		version='1.0',
		license='GPL-3',
		author='Fabrice Rey',
		author_email='fabounet@glx-dock.org',
		description='Python Status-Notifier watcher (systray daemon), to be used on systems not having one (KDE and ubuntu have their own)',
		url='https://launchpad.net/cairo-dock',
		py_modules=['status-notifier-watcher'],
		scripts=['status-notifier-watcher.py']
	)

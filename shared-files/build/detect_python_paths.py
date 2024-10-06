#!/usr/bin/python3
# detect a correct installation path

import sys

args = sys.argv
if len(sys.argv) < 2:
	sys.exit(1)

prefix = args[1]
if not prefix.endswith('/'):
	prefix += '/'
prefix += 'lib' # to distinguish /usr and /usr/local

paths = list(x for x in sys.path if x.startswith(prefix) and x.endswith('packages'))
if len(paths) > 0:
	print(paths[0])
	sys.exit(0)
else:
	sys.exit(1)



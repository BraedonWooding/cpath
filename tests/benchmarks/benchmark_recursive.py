# Recursive tree search in python
import os
import sys

rootdir = sys.argv[1]

for root, dirs, files in os.walk(rootdir):
  level = root.replace(rootdir, '').count(os.sep)
  indent = ' ' * 4 * (level)
  print('{}{}/'.format(indent, os.path.basename(root)))
  subindent = ' ' * 4 * (level + 1)
  for f in files:
    print('{}{}'.format(subindent, f))
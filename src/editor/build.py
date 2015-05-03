#!/usr/bin/env python
import sys
import os
import shutil
import re

def copy(source, destdir):
  print("Copying {0} to {1}.".format(os.path.abspath(source), os.path.abspath(destdir)))
  if (os.path.isfile(source)):
    shutil.copy(source, destdir)
  elif (os.path.isdir(source)):
    destname = os.path.basename(source.rstrip('\\').rstrip('/'))
    shutil.copytree(source, os.path.join(destdir, destname))
    
def copyall(source, destdir, working_dir='.'):
  for file_name in source:
    copy(os.path.join(working_dir, file_name), destdir)
    
def recursive_file_delete(root, basename_regex):
  for root, subdirs, files in os.walk(root):
    for file in files:
      if (basename_regex.match(os.path.basename(file))):
        print("Deleting {0}.".format(os.path.abspath(os.path.join(root, file))))
        os.remove(os.path.join(root, file))


if (len(sys.argv) != 2):
  raise RuntimeError('Wrong number of arguments!')

# Copy Editor
print("Copying editor files...")

destdir = sys.argv[1]
os.makedirs(destdir)

copyall(["classes", "styles", "images", "app.js", "index.html", "init.js"], destdir)  
copy("libs/jquery", os.path.join(destdir, "libs"))


# Copy CodeMirror
print("Copying CodeMirror files...")

cm_destdir = os.path.join(destdir, "libs/codemirror")
os.makedirs(cm_destdir)

copyall(["addon", "keymap", "lib", "mode", "theme", "LICENSE"], cm_destdir, "./libs/codemirror")

recursive_file_delete(os.path.join(cm_destdir, "mode"), re.compile(r"^.*\.html$"))
recursive_file_delete(os.path.join(cm_destdir, "mode"), re.compile(r"^test\.js$"))
recursive_file_delete(os.path.join(cm_destdir, "mode/m4"), re.compile(r"^.*\.txt$"))
recursive_file_delete(os.path.join(cm_destdir, "mode/m4"), re.compile(r"^.*\.sh$"))
recursive_file_delete(os.path.join(cm_destdir, "mode/m4"), re.compile(r"^.*\.in$"))
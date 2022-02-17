#!/usr/bin/env python3

import glob
import re
from os.path import realpath, exists

def globr(regexp):
  return glob.glob('src/' + regexp, recursive=True) + glob.glob('include/' + regexp, recursive=True)

def main():
  script_dir = '/'.join(realpath(__file__).split('/')[:-1])
  resource_dir = script_dir + '/resources'
  
  if not exists(resource_dir) and not exists(resource_dir + '/HEADER.txt'):
    print('Required resources do not exist. (resources/HEADER.txt)')
    exit(1)
  
  header = open(resource_dir + '/HEADER.txt', 'r').read()

  # .c/.cpp -> C/C++ Source Files
  # .h/.hpp -> C/C++ Header Files
  # .h.in/.hpp.in -> C++ Header Template Files (For CMake)
  files = globr('**/*.[ch]') + globr('**/*.[ch]pp') + globr('**/*.[ch]pp.in')

  for file in files:
    if 'build/' in file:
      continue

    # Replace header templating with actual header
    h = header.replace(
      '{filename}',
      file.split('/')[-1]
    ).replace(
      '{name}',
      file.split('/')[-1].split('.')[0]
    ).replace(
      '{impl}',
      'interface' if
      file.endswith('.hpp')
      or file.endswith('.h')
      or file.endswith('.hpp.in')
      or file.endswith('.h.in')
      else 'implementation'
    )

    with open(file, 'r') as f:
      content = f.read()
    if not content.startswith(h):
      print('Adding header to {}'.format(file))
      with open(file, 'w') as f:
        # Don't overwrite all the content
        f.write(h + '\n' + content)
    else:
      print('Header already added to {}'.format(file))


if __name__ == '__main__':
  main()

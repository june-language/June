#!/usr/bin/env python3

import glob
from os.path import realpath, exists

def main():
  script_dir = '/'.join(realpath(__file__).split('/')[:-1])
  resource_dir = script_dir + '/resources'
  
  if not exists(resource_dir) and not exists(resource_dir + '/HEADER.txt'):
    print('Required resources do not exist. (resources/HEADER.txt)')
    exit(1)
  
  header = open(resource_dir + '/HEADER.txt', 'r').read()

  for file in (glob.glob('**/*.cpp', recursive=True) + glob.glob('**/*.hpp', recursive=True)):
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

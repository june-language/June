#!/usr/bin/env python3

import glob
import os

def globr(regexp):
  return glob.glob(regexp, recursive=True)

def main():
  """
  Search for all files in the current directory and all subdirectories
  with the extension .cpp or .hpp and format them with clang-format.
  """
  for path in (globr('**/*.[ch]') + globr('**/*.[ch]pp')):
    if 'build/' in path:
      continue

    print(f"Formatting {path}")
    os.system('clang-format -style=file -i {}'.format(path))


if __name__ == '__main__':
  main()

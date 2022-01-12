#!/usr/bin/env python3

import glob
import os

def main():
  """
  Search for all files in the current directory and all subdirectories
  with the extension .cpp or .hpp and format them with clang-format.
  """
  for path in glob.iglob('**/*.{cpp,hpp}', recursive=True):
    os.system('clang-format -style=file -i {}'.format(path))


if __name__ == '__main__':
  main()

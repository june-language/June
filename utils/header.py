#!/usr/bin/env python3

import glob
from os.path import realpath, exists


def globr(regexp):
    return glob.glob('lib/' + regexp, recursive=True)   \
        + glob.glob('src/' + regexp, recursive=True)    \
        + glob.glob('include/' + regexp, recursive=True)


def titlecase(s):
    return s[0].upper() + s[1:]


def main():
    print("Nothing to be done, file copyright headers aren't currently being used.")
    return

    script_dir = '/'.join(realpath(__file__).split('/')[:-1])
    resource_dir = script_dir + '/resources'

    if not exists(resource_dir) and not exists(resource_dir + '/HEADER.txt'):
        print('Required resources do not exist. (resources/HEADER.txt)')
        exit(1)

    header = open(resource_dir + '/HEADER.txt', 'r').read()

    # .c/.cpp -> C/C++ Source Files
    # .h/.hpp -> C/C++ Header Files
    # .h.in/.hpp.in -> C++ Header Template Files (For CMake)
    files = globr('**/*.h') + globr('**/*.hpp') + \
        globr('**/*.hpp.in') + globr('**/*.c') + globr('**/*.cpp')

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
            '{Name}',
            titlecase(file.split('/')[-1].split('.')[0])
        ).replace(
            '{impl}',
            'interface' if
            file.endswith('.hpp')
            or file.endswith('.h')
            or file.endswith('.hpp.in')
            or file.endswith('.h.in')
            else 'implementation'
        ).replace(
            '{Impl}',
            'Interface' if
            file.endswith('.hpp')
            or file.endswith('.h')
            or file.endswith('.hpp.in')
            or file.endswith('.h.in')
            else 'Implementation'
        ).replace(
            '{lang}',
            'C++' if
            file.endswith('.hpp')
            or file.endswith('.hpp.in')
            or file.endswith('.cpp')
            or file.endswith('.cpp.in')
            else 'C'
        )

        with open(file, 'r') as f:
            content = f.read()
        skip_marker = "// -*- JuneSkipHeader -*-"
        if not content.startswith(h) and content[:len(skip_marker)] != skip_marker:
            print('Adding header to {}'.format(file))
            with open(file, 'w') as f:
                # Don't overwrite all the content
                f.write(h + '\n' + content)
        else:
            print('Header already added to {}'.format(file))


if __name__ == '__main__':
    main()

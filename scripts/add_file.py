import re
from __future__ import print_function

HEADER_TEMPLATE=r"""/*
Copyright 2016, Austen Satterlee

This file is part of VOSIMProject.

VOSIMProject is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

VOSIMProject is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with VOSIMProject. If not, see <http://www.gnu.org/licenses/>.
*/
{guard}
"""

IFNDEF_TEMPLATE = r"""
#ifndef __{guardname}__
#define __{guardname}__

#endif
"""

PRAGMA_ONCE_TEMPLATE = r"""
#pragma once
"""

SOURCE_TEMPLATE="#include \"{filename:}.h\""

def find_valid_directories(source_dir, include_dir):
    sofar = []
    def _walker(ret_list, dirname, fnames):
        if (source_dir in fnames) and (include_dir in fnames):
            ret_list.append(dirname)
        # m_dirname = re.sub(r'\binclude\b|\bsrc\b', '', dirname)
        # if m_dirname in sofar:
            # ret_list.append(dirname)
        # else:
            # sofar.append(m_dirname)
    valid_directories = []
    os.path.walk(".", _walker, valid_directories)
    return valid_directories


if __name__=="__main__":
    import argparse as ap
    import os,sys,datetime
    parser = ap.ArgumentParser(formatter_class=ap.ArgumentDefaultsHelpFormatter)
    parser.add_argument("-n", "--dry-run", action="store_true",
            help="Don't perform any actions, just print would be done")
    parser.add_argument("-f", "--force", action="store_true",
            help="Overwrite existing files")
    parser.add_argument("--guard",choices=["pragma","ifndef"], default="pragma",
            help="Choose the type of header guard to use")

    subparsers = parser.add_subparsers(title="commands")

    parser_auto_add = subparsers.add_parser("auto-add",
        help="Add source and include files to their respective directories. Automatically detect source and\
              include directories given a base directory")
    parser_auto_add.add_argument("directory", nargs='?', type=str, help="Directory that contains 'src' and 'include' dirs")
    parser_auto_add.add_argument("filenames", nargs='*', type=str, help="Name of the new files to add (without extension)")
    parser_auto_add.add_argument("--list", "-l", action="store_true", help="List valid directories and exit")
    parser_auto_add.set_defaults(command="auto-add")

    parser_add = subparsers.add_parser("add", help="Add source and include files to the specified directories")
    parser_add.add_argument("source_dir", type=str)
    parser_add.add_argument("include_dir", type=str)
    parser_add.add_argument("filenames", nargs='+', type=str)
    parser_add.set_defaults(command="add")

    parsed = parser.parse_args()

    if parsed.command=="auto-add":
        if parsed.list:
            print('\n'.join(find_valid_directories('src', 'include')))
            sys.exit(1)
        if not parsed.directory:
            sys.stderr.write("ERROR: Please provide a directory\n")
            sys.exit(1)
        if not parsed.filenames:
            sys.stderr.write("ERROR: Please provide at least one filename\n")
            sys.exit(1)

        directory = os.path.normpath(parsed.directory)
        dir_contents = os.listdir(directory)
        if ('src' not in dir_contents) or ('include' not in dir_contents):
            raise RuntimeError("'%s' and '%s' directories not found" % (parsed.source_dir,parsed.include_dir))

        include_dir = os.path.join(directory,"include")
        src_dir = os.path.join(directory,"src")

    if parsed.command=="add":
        include_dir = os.path.normpath(parsed.include_dir)
        src_dir = os.path.normpath(parsed.source_dir)

    # Check that directories exist
    if not os.path.exists(src_dir):
        sys.stderr.write("ERROR: Source directory '%s' does not exist\n" % source_dir)
        sys.exit(1)
    if not os.path.exists(include_dir):
        sys.stderr.write("ERROR: Include directory '%s' does not exist\n" % include_dir)
        sys.exit(1)

    for filename in parsed.filenames:
        include_fname = os.path.join(include_dir, filename+".h")
        src_fname = os.path.join(src_dir, filename+".cpp")

        if not parsed.force and (os.path.exists(include_fname) or os.path.exists(src_fname)):
            sys.stderr.write("ERROR: '%s' or '%s' already exists!\n" % (include_fname,src_fname))
            sys.exit(1)

        guard_str = PRAGMA_ONCE_TEMPLATE if parsed.guard=="pragma" else IFNDEF_TEMPLATE.format(guardname=filename.toupper())
        include_contents = HEADER_TEMPLATE.format(
            filename=filename,
            guard=guard_str,
            date=datetime.date.today().strftime("%m/%Y")
            )

        src_contents = SOURCE_TEMPLATE.format(filename=filename)

        if not parsed.dry_run:
            with open(include_fname,"w") as fp:
                fp.write(include_contents)
        sys.stdout.write("Added header file to {}\n".format(include_fname))

        if not parsed.dry_run:
            with open(src_fname,"w") as fp:
                fp.write(src_contents)
        sys.stdout.write("Added source file to {}\n".format(src_fname))

    sys.exit()


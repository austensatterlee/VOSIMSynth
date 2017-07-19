import re

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
    subparsers = parser.add_subparsers()

    parser_auto_add = subparsers.add_parser("auto-add",
        help="Add source and include files to their respective directories. Automatically detect source and\
              include directories given a base directory.")
    parser_auto_add.add_argument("directory", nargs='?', type=str, help="Directory that contains 'src' and 'include' dirs")
    parser_auto_add.add_argument("filename", nargs='?', type=str, help="Name of the new files to add (without extension)")
    parser_auto_add.add_argument("--list", "-l", action="store_true", help="List valid directories and exit.")
    parser_auto_add.set_defaults(command="auto-add")

    parser_add = subparsers.add_parser("add",  help="Add source and include files to the specified directories.")
    parser_add.add_argument("source_dir", type=str)
    parser_add.add_argument("include_dir", type=str)
    parser_add.add_argument("filename",type=str)
    parser_add.set_defaults(command="add")

    parser.add_argument("-f", "--force", action="store_true", help="Overwrite when adding.")
    parser.add_argument("--guard",choices=["pragma","ifndef"], default="pragma", help="Use either #ifndef\
            guards or #pragma once (default: pragma).")

    parsed = parser.parse_args()

    if parsed.command=="auto-add":
        if parsed.list:
            print '\n'.join( find_valid_directories('src', 'include') )
            sys.exit()
        if parsed.directory is None:
            raise RuntimeError("\"directory\" argument must be specified")
        if parsed.filename is None:
            raise RuntimeError("\"filename\" argument must be specified")

        dir_contents = os.listdir(parsed.directory)
        if ('src' not in dir_contents) or ('include' not in dir_contents):
            raise RuntimeError("\"%s\" and \"%s\" directories not found"%(parsed.source_dir,parsed.include_dir))

        include_dir = os.path.join(parsed.directory,"include")
        src_dir = os.path.join(parsed.directory,"src")

    if parsed.command=="add":
        if not os.path.exists(parsed.source_dir):
            raise RuntimeError("Source directory \"{}\" does not exist".format(parsed.source_dir))
        if not os.path.exists(parsed.include_dir):
            raise RuntimeError("Include directory \"{}\" does not exist".format(parsed.include_dir))

        include_dir = parsed.include_dir
        src_dir = parsed.source_dir


    include_fname = os.path.join(include_dir, parsed.filename+".h")
    src_fname = os.path.join(src_dir, parsed.filename+".cpp")

    if not parsed.force and (os.path.exists(include_fname) or os.path.exists(src_fname)):
        raise RuntimeError("\"%s\" or \"%s\" already exists!"%(include_fname,src_fname))

    guard_str = PRAGMA_ONCE_TEMPLATE if parsed.guard=="pragma" else IFNDEF_TEMPLATE.format(guardname=parsed.filename.toupper())
    include_contents = HEADER_TEMPLATE.format(
        filename=parsed.filename,
        guard=guard_str,
        date=datetime.date.today().strftime("%m/%Y")
        )

    src_contents = SOURCE_TEMPLATE.format(filename=parsed.filename)

    with open(include_fname,"w") as fp:
        fp.write(include_contents)
        sys.stdout.write("Added header file to {}\n".format(include_fname))

    with open(src_fname,"w") as fp:
        fp.write(src_contents)
        sys.stdout.write("Added source file to {}\n".format(src_fname))

    sys.exit()


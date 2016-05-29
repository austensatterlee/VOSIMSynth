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

/**
 *  \file {filename:}.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date {date:}
 */

#ifndef __{guardname:}__
#define __{guardname:}__


#endif
"""

SOURCE_TEMPLATE="#include \"{filename:}.h\""

INCLUDE_DIRNAME = "include"
SOURCES_DIRNAME = "src"

def find_valid_directories(valid_dirs, dirname, fnames):
    if (SOURCES_DIRNAME in fnames) and (INCLUDE_DIRNAME in fnames):
        valid_dirs.append(dirname)

if __name__=="__main__":
    import argparse as ap
    import os,sys,datetime
    parser = ap.ArgumentParser()
    valid_directories = []
    os.path.walk(".",find_valid_directories,valid_directories)
    parser.add_argument("directory",type=str,help="Detected valid directories: {}".format(' '.join(valid_directories)))
    parser.add_argument("filename",type=str)
    parser.add_argument("-f","--force",action="store_true",default=False)
    parsed = parser.parse_args()

    dir_contents = os.listdir(parsed.directory)
    if (SOURCES_DIRNAME not in dir_contents) or (INCLUDE_DIRNAME not in dir_contents):
        raise RuntimeError("\"%s\" and \"%s\" directories not found"%(SOURCES_DIRNAME,INCLUDE_DIRNAME))

    include_fname = os.path.join(parsed.directory,INCLUDE_DIRNAME,parsed.filename+".h")
    src_fname = os.path.join(parsed.directory,SOURCES_DIRNAME,parsed.filename+".cpp")

    if not parsed.force and (os.path.exists(include_fname) or os.path.exists(src_fname)):
        raise RuntimeError("\"%s\" or \"%s\" already exists!"%(include_fname,src_fname))

    include_contents = HEADER_TEMPLATE.format(
        filename=parsed.filename,
        guardname=parsed.filename.upper(),
        date=datetime.date.today().strftime("%m/%Y")
        )

    src_contents = SOURCE_TEMPLATE.format(filename=parsed.filename)

    with open(include_fname,"w") as fp:
        fp.write(include_contents)

    with open(src_fname,"w") as fp:
        fp.write(src_contents)

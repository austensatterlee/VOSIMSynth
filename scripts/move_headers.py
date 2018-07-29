from __future__ import print_function
import json
import re
import os
import fnmatch

def update_includes(srctxt, oldincl, newincl):
    """
    Update the code in `srctxt` so that any include directives to a file named
    `oldincl` are changed to `newincl`. If `srctxt` includes two distinct files
    named `oldincl`, both will be changed.

    :param oldincl: Old include filename.
    :param newincl: New include filename.
    """
    offset = 0
    inclregex0 = re.compile("^\\s*#include\\s+([\"'])(.*?)\\b{}\\1\\s*?$".format(re.escape(oldincl)), re.MULTILINE)
    inclregex1 = re.compile("^\\s*#include\\s+<(.*?)\\b{}>\\s*?$".format(re.escape(oldincl)), re.MULTILINE)
    srctxt, count = inclregex0.subn("#include \"%s\""%newincl, srctxt, count=1)
    if not count:
        srctxt, count = inclregex1.subn("#include <%s>"%newincl, srctxt, count=1)
    return srctxt

if __name__=="__main__":
    import argparse as ap
    parser = ap.ArgumentParser(formatter_class=ap.ArgumentDefaultsHelpFormatter)
    parser.description="Move header files, then update the #include statements in their source files."
    parser.add_argument("include_directory", type=str, help="The base include\
            directory of the project. Used to generate relative paths to the new\
            header file locations.")
    parser.add_argument("source", type=str, help="Current header location. May\
            be either a directory or a single file. If it is a directory, all\
            headers (*.h *.hpp) in that directory (but not subdirectories) will\
            be moved to the target directory.")
    parser.add_argument("target", type=str, help="Target location. Interpreted\
            as a directory if `source` is a directory. Interpreted as a file if\
            `source` is a file.")
    parser.add_argument("dependencies", type=str, nargs='+', help="Files that\
            depend on the headers specified in `source` (other than the files in\
            `source` themselves). References in these files to any of the\
            specified headers will be updated to reflect the header's new\
            location.")
    parser.add_argument("-d", "--dry", action="store_true", help="List files\
            that would be modified, but don't actually modify anything.")
    parser.add_argument("--update-only", action="store_true", help="Don't\
            actually move the header files.")
    args = parser.parse_args()

    oldincludes = []
    newincludes = []
    if os.path.isfile(args.source):
        oldincludes.append(os.path.basename(args.source))
        newincludes.append(os.path.relpath(args.target, args.include_directory))
        if not args.update_only:
            if args.dry:
                print("Moving {} -> {}".format(args.source, args.target))
            else:
                os.renames(args.source, args.target)
    elif os.path.isdir(args.source):
        # Collect headers from source directory
        basenames = set()
        for ext in ("h", "hpp"):
            basenames.update(fnmatch.filter(os.listdir(args.source), "*.%s"%ext))
        oldincludes.extend(basenames)
        for fn in oldincludes:
            oldpath = os.path.join(args.source, fn)
            newpath = os.path.join(args.target, fn)
            newincludes.append(os.path.relpath(newpath, args.include_directory))
            if not args.update_only:
                if args.dry:
                    print("Moving {} -> {}".format(oldpath, newpath))
                else:
                    os.renames(oldpath, newpath)
                    args.dependencies.append(newpath)
            elif os.path.isfile(newpath):
                args.dependencies.append(newpath)
    else:
        raise ValueError("{} does not exist!".format(args.source))

    # Update include directives in dependencies
    for fn in args.dependencies:
        if not os.path.isfile(fn):
            print("Dependency file '{}' does not exist...skipping".format(fn))
            continue
        if args.dry:
            print("Modifying {}".format(fn))
        else:
            contents = open(fn, 'r').read()
            for old,new in zip(oldincludes, newincludes):
                old = old.replace("\\", "/")
                new = new.replace("\\", "/")
                contents = update_includes(contents, old, new)
            with open(fn, 'w') as fp:
                fp.write(contents)

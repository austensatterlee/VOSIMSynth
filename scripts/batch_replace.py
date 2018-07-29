import json
import re
import os

if __name__=="__main__":
    import argparse as ap
    parser = ap.ArgumentParser()
    parser.add_argument("--map-file", type=str,
            help="Translation map json file, where instances of a key in an input file will be \
            replaced by its respective value.")
    parser.add_argument("-d", "--dry", action="store_true", help="List files\
            that would be modified, but don't actually modify anything.")
    parser.add_argument("-i", "--inline-map", type=str, action='append', nargs=2,
            metavar=('old','new'), help="Replace all occurences of `old` with `new`. \
            Mappings declared on the command line will override conflicting mappings \
            declared in the map file.")
    parser.add_argument("files", nargs='+', type=str, help="Input file paths.")
    args = parser.parse_args()

    word_map = {}
    if args.map_file:
        with open(args.map_file, 'r') as fp:
            word_map.update(json.load(fp))
    for k,v in args.inline_map:
        word_map[k] = v

    # validate map before we start editing files
    for k,v in word_map.iteritems():
        if not isinstance(k,str) and not isinstance(k,unicode):
            errmsg = "Error: {}. Expected all keys to be strings. Found {}".format(k, type(k))
            raise ValueError(errmsg)
        if not isinstance(v,str) and not isinstance(v,unicode):
            errmsg = "Error: {}. Expected all values to be strings Found {}.".format(v, type(v))
            raise ValueError(errmsg)

    # rewrite files
    for fn in args.files:
        fn = os.path.realpath(fn)
        text = open(fn, 'r').read()
        match_count = 0
        for k,v in word_map.iteritems():
            rexp = r"\b{}\b".format(k)
            match_count += len(re.findall(rexp, text))
            text = re.sub(rexp, v, text)

        msg = "Rewriting {}...({} replacements)".format(fn, match_count)
        if not args.dry:
            with open(fn, 'w') as fp:
                fp.write(text)
            print msg
        else:            
            print "(DRY RUN)", msg

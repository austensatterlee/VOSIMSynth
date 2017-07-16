import json
import re
import os

if __name__=="__main__":
    import argparse as ap
    parser = ap.ArgumentParser(formatter_class=ap.ArgumentDefaultsHelpFormatter)
    parser.add_argument("map", type=str, \
            help="Translation map, where instances of a key in an input file will be \
            replaced by its respective value.")
    parser.add_argument("files", nargs='+', type=str, help="Input file paths.")
    args = parser.parse_args()

    with open(args.map, 'r') as fp:
        word_map = json.load(fp)

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
        with open(fn, 'w') as fp:
            fp.write(text)
        print "Rewriting {}...({} replacements)".format(fn, match_count)

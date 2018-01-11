import re
import os
import pandas as pd
import numpy as np

def compute_bench_stats(data, verbose=False):
    tags = np.unique([''.join(re.findall(r"(\[.*?\])",x)) for x in data.columns])
    all_stats = {}
    for x in tags:
        fulltag = "".join(x)
        xdata = data.filter(like=fulltag)
        stats = {}
        stats["mean"] = xdata.mean()
        stats["std"] = xdata.std()
        for s in stats.keys():
            stats["norm "+s] = stats[s]/stats[s].min()
        stats_df = pd.DataFrame(stats).T
        stats_df.columns=[re.sub("\[.*?\] ?","",x) for x in stats_df.columns]
        all_stats[fulltag] = stats_df
        if verbose:
            stats_df_str = str(stats_df.T.sort_values(by="mean"))
            str_width = max(map(len, stats_df_str.split("\n")))
            tag_width = len(fulltag)
            num_spaces = 8 # str_width/2-tag_width/2
            print " "*num_spaces + "-"*len(fulltag)
            print " "*num_spaces + fulltag
            print " "*num_spaces + "-"*len(fulltag)
            print stats_df_str
            print
    return all_stats

if __name__=="__main__":
    import argparse as ap
    parser = ap.ArgumentParser()
    parser.add_argument("input_file", type=str,
            help="Input CSV file containing timing info")
    parser.add_argument("-f", "--filter", type=str, nargs="+", help="Only show results that match these strings")
    args = parser.parse_args()
    data = pd.read_csv(args.input_file)
    if args.filter:
        data = data.filter(regex='|'.join(args.filter))
    compute_bench_stats(data, verbose=True)

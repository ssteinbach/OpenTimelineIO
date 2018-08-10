#!/usr/bin/env python
import opentimelineio as otio

import argparse, copy, StringIO,pstats

def parse_args():
    """ parse arguments out of sys.argv """
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "-d",
        "--dryrun",
        action="store_true",
        default=False,
        help="dryrun mode - print what *would* be done"
    )
    parser.add_argument(
        'num',
        type=int,
        default=0,
        help='Number of clips to add'
    )
    parser.add_argument(
        '-t',
        '--test',
        nargs='+',
        choices =  TESTS.keys(),
        help="Which choices to run.",
    )

    return parser.parse_args()

def construct_track(g, trange, base_track, num):
    return base_track.extend(copy.deepcopy(g) for i in range(num))

def compute_range(obj):
    for thing in obj:
        thing.range_in_parent()

TESTS = {
}

def profile_test(func):
    TESTS[func.__name__] = func

    return func

@profile_test
def range_of(prof, args, g, trange, tr):
    prof.runcall(construct_track, g, trange, tr, args.num)
    prof.runcall(compute_range, tr)


def main():
    """main function for module"""
    args = parse_args()

    tr = otio.schema.Track()

    trange = otio.opentime.TimeRange(
        otio.opentime.RationalTime(0, 24),
        otio.opentime.RationalTime(10, 24)
    )

    g = otio.schema.Gap(source_range=trange)

    import cProfile

    for testname in args.test:
        prof = cProfile.Profile()

        TESTS[testname](prof, args, g, trange, tr)

        s = StringIO.StringIO()
        sortby = 'cumulative'
        ps = pstats.Stats(prof, stream=s).sort_stats(sortby)
        ps.print_stats()
        print s.getvalue()

if __name__ == '__main__':
    main()

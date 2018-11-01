#!/usr/bin/env python
import cProfile
import StringIO
import pstats
import opentimelineio as otio

pr = cProfile.Profile()
pr.enable()
tl = otio.adapters.read_from_file("./good_dino.otio")
# tl = otio.adapters.read_from_file("./p165_180531amt_LAYLOCK.aaf.otio")
pr.disable()
s = StringIO.StringIO()
sortby = 'cumulative'
ps = pstats.Stats(pr, stream=s).sort_stats(sortby)
ps.print_stats()
print s.getvalue()

pr = cProfile.Profile()
pr.enable()
flat = otio.algorithms.flatten_stack(tl.tracks)
pr.disable()
s = StringIO.StringIO()
sortby = 'cumulative'
ps = pstats.Stats(pr, stream=s).sort_stats(sortby)
ps.print_stats()
print s.getvalue()

# pr = cProfile.Profile()
# pr.enable()
# otio.adapters.write_to_file(tl, "flattened.dino.otio")
# pr.disable()
# s = StringIO.StringIO()
# sortby = 'cumulative'
# ps = pstats.Stats(pr, stream=s).sort_stats(sortby)
# ps.print_stats()
# print s.getvalue()

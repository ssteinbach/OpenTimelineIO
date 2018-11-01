#
# Copyright 2017 Pixar Animation Studios
#
# Licensed under the Apache License, Version 2.0 (the "Apache License")
# with the following modification; you may not use this file except in
# compliance with the Apache License and the following modification to it:
# Section 6. Trademarks. is deleted and replaced with:
#
# 6. Trademarks. This License does not grant permission to use the trade
#    names, trademarks, service marks, or product names of the Licensor
#    and its affiliates, except as required to comply with Section 4(c) of
#    the License and to reproduce the content of the NOTICE file.
#
# You may obtain a copy of the Apache License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the Apache License with the above modification is
# distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied. See the Apache License for the specific
# language governing permissions and limitations under the Apache License.
#

__doc__ = """ Algorithms for stack objects. """

import copy

from .. import (
    schema,
    opentime,
)
from . import (
    track_algo
)


def flatten_stack(in_stack):
    """Flatten a Stack, or a list of Tracks, into a single Track.
    Note that the 1st Track is the bottom one, and the last is the top.
    """

    flat_track = schema.Track()
    flat_track.name = "Flattened"

    # map of item to range_in_parent
    range_map = {}
    skips = [0]
    for track in in_stack:
        last_end = opentime.RationalTime(0, track[0].trimmed_range().start_time.rate)
        for item in (i for i in track if not isinstance(i, schema.Transition)):
            dur = item.trimmed_range().duration
            range_map[item] = opentime.TimeRange(
                last_end,
                dur
            )
            last_end += dur


    def _get_next_item(
            in_stack,
            track_index=None,
            trim_range=None
    ):
        if track_index is None:
            # start with the top-most track
            track_index = len(in_stack) - 1
        if track_index < 0:
            # if you get to the bottom, you're done
            return

        track = in_stack[track_index]
        if trim_range is not None:
            track = track_algo.track_trimmed_to_range(track, trim_range)
        for item in track:
            if (
                    item.visible()
                    or track_index == 0
                    or isinstance(item, schema.Transition)
            ):
                yield item
            else:
                if item not in range_map:
                    print "skip, ", skips[0]
                    skips[0] += 1 
                    continue
                trim = range_map[item]
                if trim_range is not None:
                    trim.start_time += trim_range.start_time
                for more in _get_next_item(in_stack, track_index - 1, trim):
                    yield more

    for item in _get_next_item(in_stack):
        flat_track.append(copy.deepcopy(item))

    return flat_track

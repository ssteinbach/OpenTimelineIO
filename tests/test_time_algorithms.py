#!/usr/bin/env python
#
# Copyright 2019 Pixar Animation Studios
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


import unittest

import opentimelineio as otio


class TestClipSpaces(unittest.TestCase):
    def setUp(self):
        # No effects, just a trim
        self.cl = otio.schema.Clip(
            source_range=otio.opentime.TimeRange(
                otio.opentime.RationalTime(450, 24),
                otio.opentime.RationalTime(30, 24),
            )
        )
        self.cl.media_reference = otio.schema.ExternalReference(
            # go 100 frames starting at frame 400
            available_range=otio.opentime.TimeRange(
                otio.opentime.RationalTime(400, 24),
                otio.opentime.RationalTime(100, 24),
            )
        )

    def test_same_space(self):
        self.assertEqual(
            otio.algorithms.transform_time(
                otio.opentime.RationalTime(
                    450,
                    24
                ),
                self.cl.internal_space(),
                self.cl.internal_space()
            ).value,
            450
        )

    def test_not_a_child_error(self):
        t1 = otio.schema.Track()
        t2 = otio.schema.Track()
        with self.assertRaises(otio.exceptions.NotAChildError):
            otio.algorithms.transform_time(
                otio.opentime.RationalTime(100, 24),
                t1.internal_space(),
                t2.internal_space()
            )

    def test_spaces_from_bottom_to_top(self):
        media_space = self.cl.media_space()
        self.assertIsNotNone(media_space)

        internal_space = self.cl.internal_space()
        self.assertIsNotNone(internal_space)

        # media references don't directly participate in the coordinate
        # hierarchy, rather their space is accessible via the media_space()
        # accessor on the clip.
        self.assertEqual(internal_space, media_space)

        trimmed_space = self.cl.trimmed_space()
        self.assertIsNotNone(trimmed_space)

        # the external space represents the space after all the transformations
        external_space = self.cl.external_space()
        self.assertIsNotNone(external_space)

        # the hidden method to transform within scopes in an object.
        result = self.cl._transform_time(
            # time to transform
            otio.opentime.RationalTime(460, 24),
            internal_space,
            internal_space
        )
        self.assertEqual(result.value, 460)

        # the hidden method to transform within scopes in an object.
        result = self.cl._transform_time(
            # time to transform
            otio.opentime.RationalTime(460, 24),
            internal_space,
            external_space
        )

        self.assertEqual(result.value, 10)

    def test_spaces_from_bottom_to_top_with_effects(self):
        internal_space = self.cl.internal_space()
        self.assertIsNotNone(internal_space)

        trimmed_space = self.cl.trimmed_space()
        self.assertIsNotNone(trimmed_space)

        effects_space = self.cl.effects_space()
        self.assertIsNotNone(effects_space)

        external_space = self.cl.external_space()
        self.assertIsNotNone(external_space)

        # the hidden method to transform within scopes in an object.
        result = self.cl._transform_time(
            # time to transform
            otio.opentime.RationalTime(460, 24),
            internal_space,
            internal_space
        )
        self.assertEqual(result.value, 460)

        result = self.cl._transform_time(
            # time to transform
            otio.opentime.RationalTime(460, 24),
            internal_space,
            effects_space
        )

        self.cl.effects.append(
            otio.schema.LinearTimeWarp(
                time_scalar=2
            )
        )

        # the hidden method to transform within scopes in an object.
        result = self.cl._transform_time(
            # time to transform
            otio.opentime.RationalTime(460, 24),
            internal_space,
            external_space
        )

        self.assertEqual(result.value, 5)

    def test_spaces_from_top_to_bottom_with_effects(self):
        internal_space = self.cl.internal_space()
        effects_space = self.cl.effects_space()
        external_space = self.cl.external_space()

        result = self.cl._transform_time(
            # time to transform
            otio.opentime.RationalTime(20, 24),
            effects_space,
            internal_space,
        )
        self.assertEqual(result.value, 470)

        self.cl.effects.append(
            otio.schema.LinearTimeWarp(
                time_scalar=2
            )
        )

        # the hidden method to transform within scopes in an object.
        result = self.cl._transform_time(
            # time to transform
            otio.opentime.RationalTime(20, 24),
            external_space,
            internal_space,
            False
        )

        self.assertEqual(result.value, 490)

    def test_spaces_from_top_to_bottom_with_trims(self):
        internal_space = self.cl.internal_space()
        external_space = self.cl.external_space()

        result = self.cl._transform_time(
            # time to transform
            otio.opentime.RationalTime(-20, 24),
            external_space,
            internal_space,
        )
        self.assertEqual(result, None)


class ChildToParentTests(unittest.TestCase):
    def setUp(self):
        self.tr = otio.schema.Track(
            source_range=otio.opentime.TimeRange(
                # trim 5 frames off the front and end
                otio.opentime.RationalTime(5, 24),
                otio.opentime.RationalTime(50, 24),
            )
        )
        self.st = otio.schema.Track(
            source_range=otio.opentime.TimeRange(
                # trim 5 frames off the front and end
                otio.opentime.RationalTime(5, 24),
                otio.opentime.RationalTime(50, 24),
            )
        )
        self.top_tr = otio.schema.Track(
            source_range=otio.opentime.TimeRange(
                # trim 5 frames off the front and end
                otio.opentime.RationalTime(5, 24),
                otio.opentime.RationalTime(40, 24),
            )
        )
        self.top_tr.append(self.tr)
        # @TODO: test this one as well ^

        for i in range(1, 4):
            self.tr.append(
                otio.schema.Clip(
                    source_range=otio.opentime.TimeRange(
                        otio.opentime.RationalTime(i * 20, 24),
                        otio.opentime.RationalTime(i * 10, 24)
                    )
                )
            )
            self.st.append(
                otio.schema.Clip(
                    source_range=otio.opentime.TimeRange(
                        otio.opentime.RationalTime(i * 20, 24),
                        otio.opentime.RationalTime(i * 10, 24)
                    )
                )
            )

    def test_transform_child_to_parent_matrix(self):
        result = self.tr.transform_child_to_parent(self.tr[1])
        self.assertEqual(
            result.offset,
            self.tr[0].trimmed_range().duration
        )

    def test_child_to_parent(self):
        result = otio.algorithms.transform_time(
            otio.opentime.RationalTime(10, 24),
            self.tr[1].external_space(),
            self.tr.internal_space(),
        )
        self.assertEqual(result.value, 20)

        result = otio.algorithms.transform_time(
            otio.opentime.RationalTime(10, 24),
            self.tr[1].external_space(),
            self.tr.trimmed_space(),
        )
        self.assertEqual(result.value, 15)

    def test_parent_to_child(self):
        # to the external space of the child
        result = otio.algorithms.transform_time(
            otio.opentime.RationalTime(15, 24),
            self.tr.trimmed_space(),
            self.tr[1].external_space(),
        )
        self.assertEqual(result.value, 10)

        result = otio.algorithms.transform_time(
            otio.opentime.RationalTime(5, 24),
            self.st.trimmed_space(),
            self.st[0].external_space(),
        )
        self.assertEqual(result.value, 10)

    def test_deep_track(self):
        last = otio.schema.Track()
        first = last

        for i in range(10):
            last.append(
                otio.schema.Clip(
                    source_range=otio.opentime.TimeRange(
                        otio.opentime.RationalTime(10 * (i + 1), 24),
                        otio.opentime.RationalTime(1, 24),
                    )
                )
            )
            # test checks the last track
            if i < 9:
                last.append(
                    otio.schema.Track()
                )
                last = last[1]

        tl = otio.schema.Timeline()
        tl.tracks.append(first)

        self.assertEqual(
            otio.algorithms.transform_time(
                otio.opentime.RationalTime(9, 24),
                first.external_space(),
                last[0].internal_space()
            ).value,
            100
        )

        self.assertEqual(
            otio.algorithms.transform_time(
                otio.opentime.RationalTime(100, 24),
                last[0].internal_space(),
                first.external_space(),
            ).value,
            9
        )


class TimelineTests(unittest.TestCase):
    def setUp(self):
        # a simple timeline with one track, with one clip, but with a global
        # offset of 1 hour.
        self.tl = otio.schema.Timeline(name="test")
        self.tl.global_start_time = otio.opentime.RationalTime(86400, 24)

        self.tl.tracks.append(otio.schema.Track())
        self.tl.tracks[0].append(otio.schema.Clip())

        self.tr = self.tl.tracks[0]
        self.cl = self.tl.tracks[0][0]

        # media reference goes 100 frames.
        self.cl.media_reference = otio.schema.MissingReference(
            available_range=otio.opentime.TimeRange(
                otio.opentime.RationalTime(0, 24),
                otio.opentime.RationalTime(100, 24),
            )
        )

    def test_single_timeline_object(self):
        some_frame = otio.opentime.RationalTime(86410, 24)

        # transform within the same object first, from global to internal
        result = otio.algorithms.transform_time(
            some_frame,
            self.tl.global_space(),
            self.tl.internal_space()
        )

        self.assertEqual(result.value, 10)

        # back into the global space
        result = otio.algorithms.transform_time(
            result,
            self.tl.internal_space(),
            self.tl.global_space(),
        )

        self.assertEqual(result, some_frame)

    def test_multi_object(self):
        some_frame = otio.opentime.RationalTime(86410, 24)

        result = otio.algorithms.transform_time(
            some_frame,
            self.tl.global_space(),
            self.cl.media_space()
        )

        self.assertEqual(result, otio.opentime.RationalTime(10, 24))

    def SKIP_test_multi_object_with_trim(self):
        some_frame = otio.opentime.RationalTime(0, 24)

        result = otio.algorithms.transform_time(
            some_frame,
            self.tl.global_space(),
            self.cl.media_space(),
            False
        )

        self.assertEqual(result, otio.opentime.RationalTime(-86400, 24))

        result = otio.algorithms.transform_time(
            some_frame,
            self.tl.global_space(),
            self.cl.media_space(),
            True
        )

        self.assertEqual(result, None)


if __name__ == '__main__':
    unittest.main()

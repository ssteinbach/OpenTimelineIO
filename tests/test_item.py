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

"""Test harness for Item."""

import unittest

import opentimelineio as otio


# add Item to the type registry for the purposes of unit testing
otio.core.register_type(otio.core.Item)


class GapTester(unittest.TestCase, otio.test_utils.OTIOAssertions):

    def test_str_gap(self):
        gp = otio.schema.Gap()
        self.assertMultiLineEqual(
            str(gp),
            "Gap(" +
            str(gp.name) + ", " +
            str(gp.source_range) + ", " +
            str(gp.effects) + ", " +
            str(gp.markers) + ", " +
            str(gp.metadata) +
            ")"
        )
        self.assertMultiLineEqual(
            repr(gp),
            "otio.schema.Gap("
            "name={}, "
            "source_range={}, "
            "effects={}, "
            "markers={}, "
            "metadata={}"
            ")".format(
                repr(gp.name),
                repr(gp.source_range),
                repr(gp.effects),
                repr(gp.markers),
                repr(gp.metadata),
            )
        )

        encoded = otio.adapters.otio_json.write_to_string(gp)
        decoded = otio.adapters.otio_json.read_from_string(encoded)
        self.assertJsonEqual(gp, decoded)

    def test_convert_from_filler(self):
        gp = otio.schema.Gap()
        gp._serializable_label = "Filler.1"
        encoded = otio.adapters.otio_json.write_to_string(gp)
        decoded = otio.adapters.otio_json.read_from_string(encoded)
        isinstance(decoded, otio.schema.Gap)

    def test_not_both_source_range_and_duration(self):
        with self.assertRaises(RuntimeError):
            otio.schema.Gap(
                duration=otio.opentime.RationalTime(10, 24),
                source_range=otio.opentime.TimeRange(
                    otio.opentime.RationalTime(0, 24),
                    otio.opentime.RationalTime(10, 24)
                )
            )

        self.assertJsonEqual(
            otio.schema.Gap(
                duration=otio.opentime.RationalTime(10, 24),
            ),
            otio.schema.Gap(
                source_range=otio.opentime.TimeRange(
                    otio.opentime.RationalTime(0, 24),
                    otio.opentime.RationalTime(10, 24)
                )
            )
        )


class ItemTests(unittest.TestCase, otio.test_utils.OTIOAssertions):

    def test_constructor(self):
        tr = otio.opentime.TimeRange(
            otio.opentime.RationalTime(0, 1),
            otio.opentime.RationalTime(10, 1)
        )
        it = otio.core.Item(name="foo", source_range=tr)
        self.assertEqual(it.source_range, tr)
        self.assertEqual(it.name, "foo")

        encoded = otio.adapters.otio_json.write_to_string(it)
        decoded = otio.adapters.otio_json.read_from_string(encoded)
        self.assertIsOTIOEquivalentTo(it, decoded)

    def test_is_parent_of(self):
        it = otio.core.Item()
        it_2 = otio.core.Item()

        self.assertFalse(it.is_parent_of(it_2))
        it_2._set_parent(it)
        self.assertTrue(it.is_parent_of(it_2))

    def test_duration(self):
        it = otio.core.Item()

        tr = otio.opentime.TimeRange(
            otio.opentime.RationalTime(0, 1),
            otio.opentime.RationalTime(10, 1)
        )
        it = otio.core.Item(source_range=tr)

        self.assertEqual(it.duration(), tr.duration)

    def test_available_range(self):
        it = otio.core.Item()

        with self.assertRaises(NotImplementedError):
            it.available_range()

    def test_duration_and_source_range(self):
        it = otio.core.Item()

        with self.assertRaises(NotImplementedError):
            it.duration()

        self.assertEqual(None, it.source_range)

        tr = otio.opentime.TimeRange(
            otio.opentime.RationalTime(1, 1),
            otio.opentime.RationalTime(10, 1)
        )
        it2 = otio.core.Item(source_range=tr)

        self.assertEqual(tr, it2.source_range)
        self.assertEqual(tr.duration, it2.duration())
        self.assertIsNot(tr.duration, it2.duration())

    def test_trimmed_range(self):
        it = otio.core.Item()
        with self.assertRaises(NotImplementedError):
            it.trimmed_range()
        tr = otio.opentime.TimeRange(
            otio.opentime.RationalTime(1, 1),
            otio.opentime.RationalTime(10, 1)
        )
        it2 = otio.core.Item(source_range=tr)
        self.assertEqual(it2.trimmed_range(), tr)
        self.assertIsNot(it2.trimmed_range(), tr)

    def test_serialize(self):
        tr = otio.opentime.TimeRange(
            otio.opentime.RationalTime(0, 1),
            otio.opentime.RationalTime(10, 1)
        )
        it = otio.core.Item(source_range=tr)
        encoded = otio.adapters.otio_json.write_to_string(it)
        decoded = otio.adapters.otio_json.read_from_string(encoded)
        self.assertIsOTIOEquivalentTo(it, decoded)

    def test_stringify(self):
        tr = otio.opentime.TimeRange(
            duration=otio.opentime.RationalTime(10, 1))
        it = otio.core.Item(source_range=tr)
        self.assertMultiLineEqual(
            str(it),
            "Item("
            "{}, "
            "{}, "
            "{}, "
            "{}, "
            "{}"
            ")".format(
                str(it.name),
                str(it.source_range),
                str(it.effects),
                str(it.markers),
                str(it.metadata),
            )
        )

        self.assertMultiLineEqual(
            repr(it),
            "otio.core.Item("
            "name={}, "
            "source_range={}, "
            "effects={}, "
            "markers={}, "
            "metadata={}"
            ")".format(
                repr(it.name),
                repr(it.source_range),
                repr(it.effects),
                repr(it.markers),
                repr(it.metadata),
            )
        )

    def test_metadata(self):
        tr = otio.opentime.TimeRange(
            duration=otio.opentime.RationalTime(10, 1)
        )
        it = otio.core.Item(source_range=tr)
        it.metadata["foo"] = "bar"
        encoded = otio.adapters.otio_json.write_to_string(it)
        decoded = otio.adapters.otio_json.read_from_string(encoded)
        self.assertIsOTIOEquivalentTo(it, decoded)
        self.assertEqual(decoded.metadata["foo"], it.metadata["foo"])

    def test_add_effect(self):
        tr = otio.opentime.TimeRange(
            duration=otio.opentime.RationalTime(10, 1)
        )
        it = otio.core.Item(source_range=tr)
        it.effects.append(
            otio.schema.Effect(
                effect_name="blur",
                metadata={
                    'amount': '100'
                }
            )
        )
        encoded = otio.adapters.otio_json.write_to_string(it)
        decoded = otio.adapters.otio_json.read_from_string(encoded)
        self.assertIsOTIOEquivalentTo(it, decoded)
        self.assertJsonEqual(it.effects, decoded.effects)

    def test_add_marker(self):
        tr = otio.opentime.TimeRange(
            duration=otio.opentime.RationalTime(10, 1)
        )
        it = otio.core.Item(source_range=tr)
        it.markers.append(
            otio.schema.Marker(
                name="test_marker",
                marked_range=tr,
                metadata={
                    'some stuff to mark': '100'
                }
            )
        )
        encoded = otio.adapters.otio_json.write_to_string(it)
        decoded = otio.adapters.otio_json.read_from_string(encoded)
        self.assertIsOTIOEquivalentTo(it, decoded)
        self.assertJsonEqual(it.markers, decoded.markers)

    def test_copy(self):
        tr = otio.opentime.TimeRange(
            duration=otio.opentime.RationalTime(10, 1)
        )
        it = otio.core.Item(source_range=tr, metadata={"foo": "bar"})
        it.markers.append(
            otio.schema.Marker(
                name="test_marker",
                marked_range=tr,
                metadata={
                    'some stuff to mark': '100'
                }
            )
        )
        it.effects.append(
            otio.schema.Effect(
                effect_name="blur",
                metadata={
                    'amount': '100'
                }
            )
        )

        it_copy = it.copy()
        self.assertIsOTIOEquivalentTo(it, it_copy)
        it.metadata["foo"] = "bar2"
        # shallow copy, should change both dictionaries
        self.assertEqual(it_copy.metadata["foo"], "bar2")

        # name should be different
        it.name = "foo"
        self.assertNotEqual(it_copy.name, it.name)

        # deep copy should have different dictionaries
        it_dcopy = it.deepcopy()
        it_dcopy.metadata["foo"] = "not bar"
        self.assertNotEqual(it.metadata, it_dcopy.metadata)

    def test_copy_library(self):
        tr = otio.opentime.TimeRange(
            duration=otio.opentime.RationalTime(10, 1)
        )
        it = otio.core.Item(source_range=tr, metadata={"foo": "bar"})
        it.markers.append(
            otio.schema.Marker(
                name="test_marker",
                marked_range=tr,
                metadata={
                    'some stuff to mark': '100'
                }
            )
        )
        it.effects.append(
            otio.schema.Effect(
                effect_name="blur",
                metadata={
                    'amount': '100'
                }
            )
        )

        # shallow test
        import copy
        it_copy = copy.copy(it)
        self.assertIsOTIOEquivalentTo(it, it_copy)
        it.metadata["foo"] = "bar2"
        # shallow copy, should change both dictionaries
        self.assertEqual(it_copy.metadata["foo"], "bar2")

        # name should be different
        it.name = "foo"
        self.assertNotEqual(it_copy.name, it.name)

        # deep copy should have different dictionaries
        it_dcopy = copy.deepcopy(it)
        it_dcopy.metadata["foo"] = "not bar"
        self.assertNotEqual(it.metadata, it_dcopy.metadata)

    def test_visible_range(self):
        timeline = otio.schema.Timeline(
            tracks=[
                otio.schema.Track(
                    name="V1",
                    children=[
                        otio.schema.Clip(
                            name="A",
                            source_range=otio.opentime.TimeRange(
                                start_time=otio.opentime.RationalTime(
                                    value=1,
                                    rate=30
                                ),
                                duration=otio.opentime.RationalTime(
                                    value=50,
                                    rate=30
                                )
                            )
                        ),
                        otio.schema.Transition(
                            in_offset=otio.opentime.RationalTime(
                                value=7,
                                rate=30
                            ),
                            out_offset=otio.opentime.RationalTime(
                                value=10,
                                rate=30
                            ),
                        ),
                        otio.schema.Clip(
                            name="B",
                            source_range=otio.opentime.TimeRange(
                                start_time=otio.opentime.RationalTime(
                                    value=100,
                                    rate=30
                                ),
                                duration=otio.opentime.RationalTime(
                                    value=50,
                                    rate=30
                                )
                            )
                        ),
                        otio.schema.Transition(
                            in_offset=otio.opentime.RationalTime(
                                value=17,
                                rate=30
                            ),
                            out_offset=otio.opentime.RationalTime(
                                value=15,
                                rate=30
                            ),
                        ),
                        otio.schema.Clip(
                            name="C",
                            source_range=otio.opentime.TimeRange(
                                start_time=otio.opentime.RationalTime(
                                    value=50,
                                    rate=30
                                ),
                                duration=otio.opentime.RationalTime(
                                    value=50,
                                    rate=30
                                )
                            )
                        ),
                        otio.schema.Clip(
                            name="D",
                            source_range=otio.opentime.TimeRange(
                                start_time=otio.opentime.RationalTime(
                                    value=1,
                                    rate=30
                                ),
                                duration=otio.opentime.RationalTime(
                                    value=50,
                                    rate=30
                                )
                            )
                        )
                    ]
                )
            ]
        )
        self.maxDiff = None
        self.assertListEqual(
            ["A", "B", "C", "D"],
            [item.name for item in timeline.each_clip()]
        )
        self.assertListEqual(
            [
                otio.opentime.TimeRange(
                    start_time=otio.opentime.RationalTime(
                        value=1,
                        rate=30
                    ),
                    duration=otio.opentime.RationalTime(
                        value=50,
                        rate=30
                    )
                ),
                otio.opentime.TimeRange(
                    start_time=otio.opentime.RationalTime(
                        value=100,
                        rate=30
                    ),
                    duration=otio.opentime.RationalTime(
                        value=50,
                        rate=30
                    )
                ),
                otio.opentime.TimeRange(
                    start_time=otio.opentime.RationalTime(
                        value=50,
                        rate=30
                    ),
                    duration=otio.opentime.RationalTime(
                        value=50,
                        rate=30
                    )
                ),
                otio.opentime.TimeRange(
                    start_time=otio.opentime.RationalTime(
                        value=1,
                        rate=30
                    ),
                    duration=otio.opentime.RationalTime(
                        value=50,
                        rate=30
                    )
                ),
            ],
            [item.trimmed_range() for item in timeline.each_clip()]
        )

        self.assertListEqual(
            [
                otio.opentime.TimeRange(
                    start_time=otio.opentime.RationalTime(
                        value=1,
                        rate=30
                    ),
                    duration=otio.opentime.RationalTime(
                        value=50+10,
                        rate=30
                    )
                ),
                otio.opentime.TimeRange(
                    start_time=otio.opentime.RationalTime(
                        value=100-7,
                        rate=30
                    ),
                    duration=otio.opentime.RationalTime(
                        value=50+15+7,
                        rate=30
                    )
                ),
                otio.opentime.TimeRange(
                    start_time=otio.opentime.RationalTime(
                        value=33,
                        rate=30
                    ),
                    duration=otio.opentime.RationalTime(
                        value=50+17,
                        rate=30
                    )
                ),
                otio.opentime.TimeRange(
                    start_time=otio.opentime.RationalTime(
                        value=1,
                        rate=30
                    ),
                    duration=otio.opentime.RationalTime(
                        value=50,
                        rate=30
                    )
                ),
            ],
            [item.visible_range() for item in timeline.each_clip()]
        )

    def test_local_to_parent_transform(self):
        it = otio.core.Item()
        it.source_range = otio.opentime.TimeRange(
            otio.opentime.RationalTime(0, 24),
            otio.opentime.RationalTime(30, 24)
        )

        # @TODO: Should having no parent mean identity transform?
        # or exception?
        # disconnected from a parent, what should the behavior be?
        with self.assertRaises(RuntimeError):
            l2p = it.local_to_parent_transform()

        tr = otio.schema.Track()
        tr.append(
            otio.schema.Gap(
                source_range=otio.opentime.TimeRange(
                    otio.opentime.RationalTime(0, 24),
                    otio.opentime.RationalTime(10, 24)
                )
            )
        )
        tr.append(it)

        # without a source range offset
        l2p = it.local_to_parent_transform()
        self.assertEqual(l2p.offset, otio.opentime.RationalTime(10, 24))

        test_frame = otio.opentime.RationalTime(5, 24)
        frame_in_parent = l2p * test_frame
        self.assertEqual(frame_in_parent.value, 15)

        # with a source range offset
        it.source_range.start_time = otio.opentime.RationalTime(30, 24)
        l2p = it.local_to_parent_transform()
        self.assertEqual(l2p.offset, otio.opentime.RationalTime(-20, 24))

        test_frame = otio.opentime.RationalTime(35, 24)
        frame_in_parent = l2p * test_frame
        self.assertEqual(frame_in_parent.value, 15)

    def test_local_to_parent_transform_with_scale(self):
        # Track
        #    Gap               Clip (x2 effect)
        # [0             30][0       10]

        # Frame 35 of Track = frame 10 of clip
        # Frame 2 of clip = Frame 31 of Track

        it = otio.core.Item()
        it.source_range = otio.opentime.TimeRange(
            otio.opentime.RationalTime(0, 24),
            otio.opentime.RationalTime(30, 24)
        )

        tr = otio.schema.Track()
        tr.append(
            otio.schema.Gap(
                source_range=otio.opentime.TimeRange(
                    otio.opentime.RationalTime(0, 24),
                    otio.opentime.RationalTime(10, 24)
                )
            )
        )
        tr.append(it)

        it.effects.append(otio.schema.LinearTimeWarp(time_scalar=2))
        l2p = it.local_to_parent_transform()
        self.assertEqual(l2p.scale, 0.5)

        test_frame = otio.opentime.RationalTime(10, 24)
        self.assertEqual((l2p * test_frame).value, 15)

        l2c = tr.local_to_child_transform(it)
        test_frame_parent = otio.opentime.RationalTime(15, 24)
        self.assertEqual(l2c * test_frame_parent, test_frame)



if __name__ == '__main__':
    unittest.main()

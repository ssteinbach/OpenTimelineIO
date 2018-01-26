#!/usr/bin/env python
#
# Copyright 2018 Pixar Animation Studios
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

"""Test harness for the filter algorithms."""

import unittest

import opentimelineio as otio

class FilterTest(unittest.TestCase):
    maxDiff = None
    def test_copy_track(self):
        md = {'test':'bar'}
        tr = otio.schema.Track(name='foo', metadata=md)
        tr.append(otio.schema.Clip(name='cl1', metadata=md))

        known = otio.adapters.write_to_string(tr, 'otio_json')
        test = otio.adapters.write_to_string(
            otio.algorithms.filtered_items(tr, lambda _:_),
            'otio_json'
        )

        self.assertMultiLineEqual(known, test)

    def test_copy_stack(self):
        """Test a no op filter that copies the timeline."""
 
        md = {'test':'bar'}
        tr = otio.schema.Stack(name='foo', metadata=md)
        tr.append(otio.schema.Clip(name='cl1', metadata=md))

        known = otio.adapters.write_to_string(tr, 'otio_json')
        result = otio.algorithms.filtered_items(tr, lambda _:_)
        test = otio.adapters.write_to_string(result, 'otio_json')

        self.assertMultiLineEqual(known, test)
        self.assertIsNot(tr[0], result)

    def test_prune_clips(self):
        """test a filter that removes clips"""

        md = {'test':'bar'}
        tr = otio.schema.Track(name='foo', metadata=md)
        tr.append(otio.schema.Clip(name='cl1', metadata=md))

        def no_clips(thing):
            if not isinstance(thing, otio.schema.Clip):
                return thing
            return None

        result = otio.algorithms.filtered_items(tr, no_clips)
        self.assertEqual(0, len(result))
        self.assertEqual(tr.metadata, result.metadata)

        # emptying the track should have the same effect
        del tr[:]
        self.assertEqual(tr, result)

    def test_copy(self):
        md = {'test':'bar'}
        tl = otio.schema.Timeline(name='foo', metadata=md)
        tl.tracks.append(otio.schema.Track(name='track1', metadata=md))
        tl.tracks[0].append(otio.schema.Clip(name='cl1', metadata=md))

        known = otio.adapters.write_to_string(tl, 'otio_json')
        test = otio.adapters.write_to_string(
            otio.algorithms.filtered_items(tl, lambda _:_),
            'otio_json'
        )

        # make sure the original timeline didn't get nuked
        self.assertEqual(len(tl.tracks), 1)

        self.assertMultiLineEqual(known, test)


class ReduceTest(unittest.TestCase):
    maxDiff = None
    def test_copy_track(self):
        md = {'test':'bar'}
        tr = otio.schema.Track(name='foo', metadata=md)
        tr.append(otio.schema.Clip(name='cl1', metadata=md))

        known = otio.adapters.write_to_string(tr, 'otio_json')
        test = otio.adapters.write_to_string(
            otio.algorithms.reduced_items(tr, lambda __, _, ___:_),
            'otio_json'
        )

        self.assertMultiLineEqual(known, test)

    def test_copy_stack(self):
        """Test a no op reduce that copies the timeline."""
 
        md = {'test':'bar'}
        tr = otio.schema.Stack(name='foo', metadata=md)
        tr.append(otio.schema.Clip(name='cl1', metadata=md))

        known = otio.adapters.write_to_string(tr, 'otio_json')
        result = otio.algorithms.reduced_items(tr, lambda __, _, ___:_)
        test = otio.adapters.write_to_string(result, 'otio_json')

        self.assertMultiLineEqual(known, test)
        self.assertIsNot(tr[0], result)

    def test_prune_clips(self):
        """test a reduce that removes clips"""

        md = {'test':'bar'}
        tr = otio.schema.Track(name='foo', metadata=md)
        tr.append(otio.schema.Clip(name='cl1', metadata=md))

        def no_clips(_, thing, __):
            if not isinstance(thing, otio.schema.Clip):
                return thing
            return None

        result = otio.algorithms.reduced_items(tr, no_clips)
        self.assertEqual(0, len(result))
        self.assertEqual(tr.metadata, result.metadata)

        # emptying the track should have the same effect
        del tr[:]
        self.assertEqual(tr, result)

    def test_prune_clips_after_transitions(self):
        """test a reduce that removes clips that follow transitions"""

        md = {'test':'bar'}
        tr = otio.schema.Track(name='foo', metadata=md)
        for i in range(5):
            ind = str(i)
            if i in (2, 3):
                tr.append(otio.schema.Transition(name='should_be_pruned'+ind))
            tr.append(otio.schema.Clip(name='cl'+ind, metadata=md))

        def no_clips_after_transitions(prev, thing, __):
            if (
                isinstance(prev, otio.schema.Transition) or
                isinstance(thing, otio.schema.Transition)
            ):
                return None
            return thing

        result = otio.algorithms.reduced_items(tr, no_clips_after_transitions)
        self.assertEqual(3, len(result))
        self.assertEqual(tr.metadata, result.metadata)

        # ...but that things have been properly deep copied
        self.assertIsNot(tr.metadata, result.metadata)

        # emptying the track of transitions and the clips they follow and 
        # should have the same effect
        del tr[2:6]
        self.assertEqual(tr, result)


    def test_copy(self):
        """Test that a simple reduce results in a copy"""
        md = {'test':'bar'}
        tl = otio.schema.Timeline(name='foo', metadata=md)
        tl.tracks.append(otio.schema.Track(name='track1', metadata=md))
        tl.tracks[0].append(otio.schema.Clip(name='cl1', metadata=md))

        known = otio.adapters.write_to_string(tl, 'otio_json')
        test = otio.adapters.write_to_string(
            otio.algorithms.reduced_items(tl, lambda __, _, ___:_),
            'otio_json'
        )

        # make sure the original timeline didn't get nuked
        self.assertEqual(len(tl.tracks), 1)
        self.assertMultiLineEqual(known, test)


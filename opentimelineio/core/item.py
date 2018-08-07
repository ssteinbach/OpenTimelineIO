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

"""Implementation of the Item base class.  OTIO Objects that contain media."""

import copy

from .. import (
    opentime,
    exceptions,
)

from . import (
    serializable_object,
    composable,
    reference_frame,
)


class Item(composable.Composable):
    """An Item is a Composable that can be part of a Composition or Timeline.

    More specifically, it is a Composable that has meaningful duration.

    Can also hold effects and markers.

    Base class of:
        - Composition (and children)
        - Clip
        - Gap
    """

    _serializable_label = "Item.1"
    _class_path = "core.Item"

    def __init__(
        self,
        name=None,
        source_range=None,
        effects=None,
        markers=None,
        metadata=None,
    ):
        serializable_object.SerializableObject.__init__(self)

        self.name = name
        self.source_range = source_range
        self.effects = effects or []
        self.markers = markers or []
        self.metadata = metadata or {}
        self._parent = None

        # scopes
        self._before_effects = reference_frame.ReferenceFrame(
            self,
            reference_frame.TransformName.BeforeEffects
        )
        self._after_effects = reference_frame.ReferenceFrame(
            self,
            reference_frame.TransformName.AfterEffects
        )

    name = serializable_object.serializable_field("name", doc="Item name.")
    source_range = serializable_object.serializable_field(
        "source_range",
        opentime.TimeRange,
        doc="Range of source to trim to.  Can be None or a TimeRange."
    )

    @staticmethod
    def visible():
        """Return the visibility of the Item. By default True."""

        return True

    def duration(self, space=None):
        """Convience wrapper for the trimmed_range.duration of the item."""

        return self.trimmed_range(space).duration

    def available_range(self):
        """Implemented by child classes, available range of media."""

        raise NotImplementedError

    def trimmed_range(self, space=None):
        """The range after applying the source range."""

        space = space or self.after_effects

        try:
            if space.parent is not self:
                raise exceptions.NotSupportedError(
                    "space must be from this item, ie this_item.before_effects."
                    "  Parent is: {}".format(space.parent)
                )
        except AttributeError:
            raise exceptions.NotSupportedError(
                "space must be of type {}, got object of type {}".format(
                    reference_frame.ReferenceFrame, 
                    type(space)
                )
            )

        if self.source_range is not None:
            result_range = copy.copy(self.source_range)
        else:
            result_range = self.available_range()

        if space is self.after_effects:
            return result_range

        # build a local transform
        last_xform = self.effects_time_transform()

        if last_xform.is_identity():
            return result_range

        return last_xform * result_range

    def effects_time_transform(self):
        # build a local transform
        last_xform = opentime.TimeTransform()

        for ef in self.effects:
            # @TODO: move Effect/TimeEffect into core
            # @TODO: create a 'core.BlindEffect' to indicate that its an unknown,
            #        uninterpreted kind of effect that can go into schema
            if hasattr(ef, 'transform'):
                try:
                    last_xform = ef.transform().inverted()
                except:
                    continue

        return last_xform

    def visible_range(self):
        """The range of this item's media visible to its parent.
        Includes handles revealed by adjacent transitions (if any)."""
        result = self.trimmed_range()
        if self.parent():
            head, tail = self.parent().handles_of_child(self)
            if head:
                result.start_time -= head
                result.duration += head
            if tail:
                result.duration += tail
        return result

    def trimmed_range_in_parent(self):
        """Find and return the trimmed range of this item in the parent."""
        if not self.parent():
            raise exceptions.NotAChildError(
                "No parent of {}, cannot compute range in parent.".format(self)
            )

        return self.parent().trimmed_range_of_child(self)

    def range_in_parent(self):
        """Find and return the untrimmed range of this item in the parent."""
        if not self.parent():
            raise exceptions.NotAChildError(
                "No parent of {}, cannot compute range in parent.".format(self)
            )

        return self.parent().range_of_child(self)

    def transformed_time(self, t, to_item):
        """Converts time t in the coordinate system of self to coordinate
        system of to_item.

        Note that self and to_item must be part of the same timeline (they must
        have a common ancestor).

        Example:
        0                      20
        [------*----D----------]
        [--A--|*----B----|--C--]
             100 101    110
        101 in B = 6 in D

        * = t argument
        """

        # does not operate in place
        result = copy.copy(t)

        if to_item is None:
            return result

        root = self._root_parent()

        # transform t to root  parent's coordinate system
        item = self
        while item != root and item != to_item:

            parent = item._parent
            result -= item.trimmed_range().start_time
            result += parent.range_of_child(item).start_time

            item = parent

        ancestor = item

        # transform from root parent's coordinate system to to_item
        item = to_item
        while item != root and item != ancestor:

            parent = item._parent
            result += item.trimmed_range().start_time
            result -= parent.range_of_child(item).start_time

            item = parent

        assert(item == ancestor)

        return result

    def transformed_time_range(self, tr, to_item):
        """Transforms the timerange tr to the range of child or self to_item.

        """

        return opentime.TimeRange(
            self.transformed_time(tr.start_time, to_item),
            tr.duration
        )

    # @{ Transform functions
    def local_to_parent_transform(self):
        if not self._parent:
            raise RuntimeError(
                "Cannot compute local to parent transform with no parent."
            )

        result = opentime.TimeTransform()

        rng = self.range_in_parent()

        result.offset = rng.start_time - self.trimmed_range().start_time

        # handle scale
        for ef in reversed(self.effects):
            try:
                result.scale = 1.0 / float(ef.time_scalar)
                break
            except AttributeError:
                pass

        return result
    # @}

    markers = serializable_object.serializable_field(
        "markers",
        doc="List of markers on this item."
    )
    effects = serializable_object.serializable_field(
        "effects",
        doc="List of effects on this item."
    )
    metadata = serializable_object.serializable_field(
        "metadata",
        doc="Metadata dictionary for this item."
    )

    def __repr__(self):
        return (
            "otio.{}("
            "name={}, "
            "source_range={}, "
            "effects={}, "
            "markers={}, "
            "metadata={}"
            ")".format(
                self._class_path,
                repr(self.name),
                repr(self.source_range),
                repr(self.effects),
                repr(self.markers),
                repr(self.metadata)
            )
        )

    def __str__(self):
        return "{}({}, {}, {}, {}, {})".format(
            self._class_path.split('.')[-1],
            self.name,
            str(self.source_range),
            str(self.effects),
            str(self.markers),
            str(self.metadata)
        )

    @property
    def before_effects(self):
        return self._before_effects

    @property
    def after_effects(self):
        return self._after_effects

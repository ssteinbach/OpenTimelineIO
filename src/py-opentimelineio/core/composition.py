from . _core_utils import add_method
from opentimelineio import _otio

@add_method(_otio.Composition)
def __str__(self):
    return "{}({}, {}, {}, {})".format(
        self.__class__.__name__,
        str(self.name),
        str(list(self)),
        str(self.source_range),
        str(self.metadata)
    )

@add_method(_otio.Composition)
def __repr__(self):
    return (
        "otio.{}.{}("
        "name={}, "
        "children={}, "
        "source_range={}, "
        "metadata={}"
        ")".format(
            "core" if self.__class__ is _otio.Composition else "schema",
            self.__class__.__name__,
            repr(self.name),
            repr(list(self)),
            repr(self.source_range),
            repr(self.metadata)
        )
    )

@add_method(_otio.Composition)
def each_child(self, search_range=None, descended_from_type=_otio.Composable):
    is_descendant = descended_from_type is _otio.Composable
    for i, child in enumerate(self):
        # filter out children who are not in the search range
        if search_range and not self.range_of_child_at_index(i).overlaps(search_range):
            continue

        # filter out children who are not descended from the specified type

        if is_descendant or isinstance(child, descended_from_type):
            yield child

        # for children that are compositions, recurse into their children
        if hasattr(child, "each_child"):
            for c in child.each_child(search_range, descended_from_type):
                yield c
                

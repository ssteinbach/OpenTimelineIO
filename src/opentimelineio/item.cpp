#include "opentimelineio/item.h"
#include "opentimelineio/composition.h"
#include "opentimelineio/effect.h"
#include "opentimelineio/marker.h"

Item::Item(std::string const& name,
           optional<TimeRange> const& source_range,
           AnyDictionary const& metadata,
           std::vector<Effect*> const& effects,
           std::vector<Marker*> const& markers)
    : Parent(name, metadata),
      _source_range(source_range),
      _effects(effects.begin(), effects.end()),
      _markers(markers.begin(), markers.end())
{
}

Item::~Item() {
}

bool Item::visible() const {
    return true;
}

bool Item::overlapping() const {
    return false;
}

TimeRange Item::available_range(ErrorStatus* error_status) const {
    *error_status = ErrorStatus::NOT_IMPLEMENTED;
    return TimeRange();
}

TimeRange Item::visible_range(ErrorStatus* error_status) const {
    TimeRange result = trimmed_range(error_status);

    if (parent() && !(*error_status)) {
        auto head_tail = parent()->handles_of_child(this);
        if (head_tail.first) {
            result = TimeRange(result.start_time() - *head_tail.first,
                               result.duration() + *head_tail.first);
        }
        if (head_tail.second) {
            result = TimeRange(result.start_time(), result.duration() + *head_tail.second);
        }
    }
    return result;
}

bool Item::read_from(Reader& reader) {
    return reader.read("source_range", &_source_range) &&
        reader.read("effects", &_effects) &&
        reader.read("markers", &_markers) &&
        Parent::read_from(reader);
}

void Item::write_to(Writer& writer) const {
    Parent::write_to(writer);
    writer.write("source_range", _source_range);
    writer.write("effects", _effects);
    writer.write("markers", _markers);
}

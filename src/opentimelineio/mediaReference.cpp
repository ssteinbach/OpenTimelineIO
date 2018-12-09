#include "opentimelineio/mediaReference.h"

MediaReference::MediaReference(std::string const& name,
                               optional<TimeRange> const& available_range,
                               AnyDictionary const& metadata)
    : Parent(name, metadata),
      _available_range(available_range) {
}

MediaReference::~MediaReference() {
}


bool MediaReference::is_missing_reference() const {
    return false;
}

bool MediaReference::read_from(Reader& reader) {
    return reader.read("available_range", &_available_range) &&
        Parent::read_from(reader);
}

void MediaReference::write_to(Writer& writer) const {
    Parent::write_to(writer);
    writer.write("available_range", _available_range);
}

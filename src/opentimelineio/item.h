#ifndef OTIO_ITEM_H
#define OTIO_ITEM_H

#include "opentime/timeRange.h"
#include "opentimelineio/composable.h"
#include "opentimelineio/optional.h"
#include "opentimelineio/errorStatus.h"
    
class Effect;
class Marker;

class Item : public Composable {
public:
    struct Schema {
        static auto constexpr name = "Item";
        static int constexpr version = 1;
    };

    using Parent = Composable;

    Item(std::string const& name = std::string(),
         optional<TimeRange> const& source_range = nullopt,
         AnyDictionary const& metadata = AnyDictionary(),
         std::vector<Effect*> const& effects = std::vector<Effect*>(),
         std::vector<Marker*> const& markers = std::vector<Marker*>());

    virtual bool visible() const;
    virtual bool overlapping() const;

    optional<TimeRange> const& source_range () const {
        return _source_range;
    }

    void set_source_range(optional<TimeRange> const& source_range) {
        _source_range = source_range;
    }

    std::vector<Retainer<Effect>>& effects() {
        return _effects;
    }

    std::vector<Retainer<Effect>> const& effects() const {
        return _effects;
    }

    std::vector<Retainer<Marker>>& markers() {
        return _markers;
    }

    std::vector<Retainer<Marker>> const& markers() const {
        return _markers;
    }

    virtual TimeRange available_range(ErrorStatus* error_status) const;

    TimeRange trimmed_range(ErrorStatus* error_status) const {
        return _source_range ? *_source_range : available_range(error_status);
    }

    TimeRange visible_range(ErrorStatus* error_status) const;

protected:
    virtual ~Item();

    virtual bool read_from(Reader&);
    virtual void write_to(Writer&) const;

private:
    optional<TimeRange> _source_range;
    std::vector<Retainer<Effect>> _effects;
    std::vector<Retainer<Marker>> _markers;
};

#endif
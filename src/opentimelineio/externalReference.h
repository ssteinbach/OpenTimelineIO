#ifndef OTIO_EXTERNAL_REFERENCE_H
#define OTIO_EXTERNAL_REFERENCE_H

#include "opentimelineio/mediaReference.h"

class ExternalReference final : public MediaReference {
public:
    struct Schema {
        static auto constexpr name = "ExternalReference";
        static int constexpr version = 1;
    };

    using Parent = MediaReference;

    ExternalReference(std::string target_url = std::string(),
                      optional<TimeRange> const& available_range = nullopt,
                      AnyDictionary const& metadata = AnyDictionary());
        
    std::string const& target_url() const {
        return _target_url;
    }
    
    void set_target_url(std::string const& target_url) {
        _target_url = target_url;
    }

protected:
    virtual ~ExternalReference();

    virtual bool read_from(Reader&);
    virtual void write_to(Writer&) const;

private:
    std::string _target_url;
};

#endif

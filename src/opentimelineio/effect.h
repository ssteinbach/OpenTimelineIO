#ifndef OTIO_EFFECT_H
#define OTIO_EFFECT_H

#include "opentimelineio/serializableObjectWithMetadata.h"

class Effect : public SerializableObjectWithMetadata {
public:
    struct Schema {
        static auto constexpr name = "Effect";
        static int constexpr version = 1;
    };

    using Parent = SerializableObjectWithMetadata;

    Effect(std::string const& name = std::string(),
           std::string const& effect_name = std::string(),
           AnyDictionary const& metadata = AnyDictionary());

    std::string const& effect_name() const {
        return _effect_name;
    }
    
    void set_effect_name(std::string const& effect_name) {
        _effect_name = effect_name;
    }

protected:
    virtual ~Effect();

    virtual bool read_from(Reader&);
    virtual void write_to(Writer&) const;

private:
    std::string _effect_name;
};

#endif
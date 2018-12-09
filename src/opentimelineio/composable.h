#ifndef OTIO_COMPOSABLE_H
#define OTIO_COMPOSABLE_H

#include "opentimelineio/serializableObjectWithMetadata.h"

class Composition;

class Composable : public SerializableObjectWithMetadata {
public:
    struct Schema {
        static auto constexpr name = "Composable";
        static int constexpr version = 1;
    };

    using Parent = SerializableObjectWithMetadata;

    Composable(std::string const& name = std::string(),
               AnyDictionary const& metadata = AnyDictionary());

    virtual bool visible() const;
    virtual bool overlapping() const;

    Composition* parent() const {
        return _parent;
    }
    

protected:
    bool _set_parent(Composition* parent);
    Composable* _highest_ancestor();

    Composable const* _highest_ancestor() const {
        return const_cast<Composable*>(this)->_highest_ancestor();
    }

    virtual ~Composable();

    virtual bool read_from(Reader&);
    virtual void write_to(Writer&) const;

private:
    Composition* _parent;
    friend class Composition;
};

#endif
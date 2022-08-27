// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

#include "errorStatus.h"
#include "nonstd/optional.hpp"
#include "opentimelineio/serializableObject.h"
#include "opentimelineio/serialization.h"
#include "opentimelineio/anyDictionary.h"
#include "opentimelineio/unknownSchema.h"
#include "stringUtils.h"
#include <string>

#define RAPIDJSON_NAMESPACE OTIO_rapidjson
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <fstream>

#if defined(_WINDOWS)
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif // WIN32_LEAN_AND_MEAN
#    ifndef NOMINMAX
#        define NOMINMAX
#    endif // NOMINMAX
#    include <windows.h>
#endif

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

// built in version labels (@TODO: move this into a generated .h/.cpp file)
static family_to_label_map FAMILY_LABEL_MAP {
    { 
        "OTIO_CORE", 
            { 
                { "test", 
                    { 
                        { "Clip", 1 } 
                    } 
                } 
            } 
    }
};

bool
add_family_label_version(
        const std::string& family, 
        const std::string& label,
        const schema_version_map& new_map,
        ErrorStatus* err
)
{
    if (family == "OTIO_CORE")
    {
        *err = ErrorStatus(
                ErrorStatus::SCHEMA_VERSION_UNSUPPORTED,
                (
                 "Not allowed to insert new version maps into the OTIO_CORE"
                 " version family."
                )
        );
        return false;
    }

    auto fam_map_it = FAMILY_LABEL_MAP.find(family);

    if (fam_map_it == FAMILY_LABEL_MAP.end())
    {
        FAMILY_LABEL_MAP[family] = label_to_schema_version_map {};
    }

    auto& fam_map = FAMILY_LABEL_MAP[family];

    // check to see if label already exists, not allowed to overwrite
    auto label_map_it = fam_map.find(label);
    if (label_map_it != fam_map.end())
    {
        *err = ErrorStatus(
                ErrorStatus::SCHEMA_VERSION_UNSUPPORTED,
                (
                 "version label: " + label 
                 + " already exists in version family: " + family 
                 + ", cannot add."
                )
        );
        return false;
    }

    fam_map[label] = new_map;

    return true;
}

const family_to_label_map
family_label_version_map()
{
    return FAMILY_LABEL_MAP;
}

/**
 * Base class for encoders.  Since rapidjson is templated (no virtual functions)
 * we need to do our dynamically classed hierarchy to abstract away which writer
 * we are using.  This also lets us create the CloningEncoder, which is what
 * we use not to serialize a class, but to copy it in memory, thereby cloning
 * an instance of a SerializableObject.
 *
 * This hierarchy is not visible outside this library, so we're not very concerned
 * about access control within this class.
 */
class Encoder
{
public:
    virtual ~Encoder() {}

    bool has_errored(ErrorStatus* error_status)
    {
        if (error_status)
        {
            *error_status = _error_status;
        }
        return is_error(_error_status);
    }

    bool has_errored() { return is_error(_error_status); }

    virtual bool encoding_to_anydict() { return false; }

    virtual void start_object() = 0;
    virtual void end_object()   = 0;

    virtual void start_array(size_t) = 0;
    virtual void end_array()         = 0;

    virtual void write_key(std::string const& key)                   = 0;
    virtual void write_null_value()                                  = 0;
    virtual void write_value(bool value)                             = 0;
    virtual void write_value(int value)                              = 0;
    virtual void write_value(int64_t value)                          = 0;
    virtual void write_value(uint64_t value)                         = 0;
    virtual void write_value(double value)                           = 0;
    virtual void write_value(std::string const& value)               = 0;
    virtual void write_value(class RationalTime const& value)        = 0;
    virtual void write_value(class TimeRange const& value)           = 0;
    virtual void write_value(class TimeTransform const& value)       = 0;
    virtual void write_value(struct SerializableObject::ReferenceId) = 0;
    virtual void write_value(Imath::Box2d const&)                    = 0;

protected:
    void _error(ErrorStatus const& error_status)
    {
        _error_status = error_status;
    }

private:
    friend class SerializableObject;
    ErrorStatus _error_status;
};

/**
 * This encoder builds up a dictionary as its method of "encoding".
 * The dictionary is than handed off to a CloningDecoder, to complete
 * copying of a SerializableObject instance.
 */
class CloningEncoder : public Encoder
{
public:
    enum class ResultObjectPolicy {
        CloneBackToSerializableObject = 0,
        MathTypesConcreteAnyDictionaryResult, 
        OnlyAnyDictionary,
    };

    CloningEncoder(
            CloningEncoder::ResultObjectPolicy result_object_policy,
            optional<const schema_version_map*> downgrade_version_manifest = {}
    ) : 
        _result_object_policy(result_object_policy),
        _downgrade_version_manifest(downgrade_version_manifest)
    {
        using namespace std::placeholders;
        _error_function = std::bind(&CloningEncoder::_error, this, _1);
    }

    virtual ~CloningEncoder() {}

    /**
     * 1.
     * e = JSONEncoder(_downgrade)
     * SerializableObject::Writer::write_root(e, _downgrade)
     * [recurse until downgrade]
     *      ce = CloningEncoder(_downgrade)
     *      SerializableObject::Writer::write_root(e), 
     */
    virtual bool 
    encoding_to_anydict() override 
    {
        return (_result_object_policy == ResultObjectPolicy::OnlyAnyDictionary);
    }

    void write_key(std::string const& key) override
    {
        if (has_errored())
        {
            return;
        }

        if (_stack.empty() || !_stack.back().is_dict)
        {
            _internal_error(
                "Encoder::write_key  called while not decoding an object");
            return;
        }

        _stack.back().cur_key = key;
    }

    void _store(any&& a)
    {
        if (has_errored())
        {
            return;
        }

        if (_stack.empty())
        {
            _root.swap(a);
        }
        else
        {
            auto& top = _stack.back();
            if (top.is_dict)
            {
                top.dict.emplace(_stack.back().cur_key, a);
            }
            else
            {
                top.array.emplace_back(a);
            }
        }
    }

    void write_null_value() override { _store(any()); }
    void write_value(bool value) override { _store(any(value)); }
    void write_value(int value) override { _store(any(value)); }
    void write_value(int64_t value) override { _store(any(value)); }
    void write_value(uint64_t value) override { _store(any(value)); }
    void write_value(std::string const& value) override { _store(any(value)); }
    void write_value(double value) override { _store(any(value)); }

    void 
    write_value(RationalTime const& value) override
    { 
        if (_result_object_policy == ResultObjectPolicy::OnlyAnyDictionary) 
        {
            AnyDictionary result = {
                {"OTIO_SCHEMA", "RationalTime.1"},
                {"value", value.value()},
                {"rate", value.rate()},
            };
            _store(any(std::move(result)));
        } 
        else 
        {
            _store(any(value)); 
        }
    }
    void 
    write_value(TimeRange const& value) override
    {
 
        if (_result_object_policy == ResultObjectPolicy::OnlyAnyDictionary) 
        {
            AnyDictionary result = {
                {"OTIO_SCHEMA", "TimeRange.1"},
                {"duration", value.duration()},
                {"start_time", value.start_time()},
            };
            _store(any(std::move(result)));
        } 
        else 
        { 
            _store(any(value)); 
        }

    }
    void 
    write_value(TimeTransform const& value) override
    {
        if (_result_object_policy == ResultObjectPolicy::OnlyAnyDictionary) 
        {
            AnyDictionary result {
                {"OTIO_SCHEMA", "TimeTransform.1"},
                {"offset", value.offset()},
                {"rate", value.rate()},
                {"scale", value.scale()},
            };
            _store(any(std::move(result)));
        } 
        else 
        {
            _store(any(value)); 
        }
    }
    void 
    write_value(SerializableObject::ReferenceId value) override
    {
        if (_result_object_policy == ResultObjectPolicy::OnlyAnyDictionary) 
        {
            AnyDictionary result {
                {"OTIO_SCHEMA", "SerializableObjectRef.1"},
                {"id", value.id.c_str()},
            };
            _store(any(std::move(result)));
        } 
        else 
        {
            _store(any(value)); 
        }
       _store(any(value));
    }
    
    void 
    write_value(Imath::V2d const& value) 
    {
 
        if (_result_object_policy == ResultObjectPolicy::OnlyAnyDictionary) 
        {
            AnyDictionary result {
                {"OTIO_SCHEMA", "V2d.1"},
                {"x", value.x},
                {"y", value.y},
            };
            _store(any(std::move(result)));
        }
        else 
        {
            _store(any(value)); 
        }

    }

    void 
    write_value(Imath::Box2d const& value) override 
    {
        if (_result_object_policy == ResultObjectPolicy::OnlyAnyDictionary) 
        {
            AnyDictionary result {
                {"OTIO_SCHEMA", "Box2d.1"},
                {"min", value.min},
                {"max", value.max},
            };
            _store(any(std::move(result)));
        } else { 
            _store(any(value)); 
        }
    }
    // @}

    void start_array(size_t /* n */) override
    {
        if (has_errored())
        {
            return;
        }

        _stack.emplace_back(_DictOrArray{ false /* is_dict*/ });
    }

    void start_object() override
    {
        if (has_errored())
        {
            return;
        }

        _stack.emplace_back(_DictOrArray{ true /* is_dict*/ });
    }

    void end_array() override
    {
        if (has_errored())
        {
            return;
        }

        if (_stack.empty())
        {
            _internal_error(
                "Encoder::end_array() called without matching start_array()");
        }
        else
        {
            auto& top = _stack.back();
            if (top.is_dict)
            {
                _internal_error(
                    "Encoder::end_array() called without matching start_array()");
                _stack.pop_back();
            }
            else
            {
                AnyVector va;
                va.swap(top.array);
                _stack.pop_back();
                _store(any(std::move(va)));
            }
        }
    }

    void end_object() override
    {
        if (has_errored())
        {
            return;
        }

        if (_stack.empty())
        {
            _internal_error(
                "Encoder::end_object() called without matching start_object()"
            );
            return;
        }

        auto& top = _stack.back();
        if (!top.is_dict)
        {
            _internal_error(
                "Encoder::end_object() called without matching start_object()");
            _stack.pop_back();

            return;
        }

        /*
         * Convert back to SerializableObject* right here.
         */
        if (
                _result_object_policy 
                == ResultObjectPolicy::CloneBackToSerializableObject
        )
        {
            SerializableObject::Reader reader(
                top.dict, 
                _error_function,
                nullptr
            );
            _stack.pop_back();
            _store(reader._decode(_resolver));

            return;
        }

        // otherwise, build out as an anydictionary & downgrade
        AnyDictionary m;
        m.swap(top.dict);

        if (_downgrade_version_manifest.has_value())
        {
            _downgrade_dictionary(m);
        }

        _stack.pop_back();
        _store(any(std::move(m)));
    }

    // @TODO: what kind of ownership policy here?
    AnyDictionary
    root() 
    {
        if (_root.type() == typeid(AnyDictionary))
        {
            return any_cast<AnyDictionary>(_root);
        }
        else 
        {
            return AnyDictionary();
        }
    }

private:
    any                                     _root;
    SerializableObject::Reader::_Resolver   _resolver;
    std::function<void(ErrorStatus const&)> _error_function;

    struct _DictOrArray
    {
        _DictOrArray(bool is_dict) { this->is_dict = is_dict; }

        bool          is_dict;
        AnyDictionary dict;
        AnyVector     array;
        std::string   cur_key;
    };


    void _internal_error(std::string const& err_msg)
    {
        _error(ErrorStatus(ErrorStatus::INTERNAL_ERROR, err_msg));
    }

    friend class SerializableObject;
    std::vector<_DictOrArray> _stack;
    ResultObjectPolicy        _result_object_policy;
    optional<const schema_version_map*> _downgrade_version_manifest = {};

    void
    _downgrade_dictionary(
            AnyDictionary& m
    )
    {
        const std::string& schema_string = m.get_default(
                "OTIO_SCHEMA",
                std::string("")
        );

        if (schema_string.empty())
        {
            return;
        }

        const int sep = schema_string.rfind('.');
        const std::string& schema_name = schema_string.substr(0, sep);

        const auto& dg_man = *(*_downgrade_version_manifest);
        const auto dg_version_it = dg_man.find(schema_name);

        if (dg_version_it == dg_man.end())
        {
            return;
        }

        const std::string& schema_vers = schema_string.substr(sep+1);
        int current_version = -1;

        if (!schema_vers.empty()) 
        {
            current_version = std::stoi(schema_vers);
        }

        if (current_version < 0)
        {
            _internal_error(
                    string_printf(
                        "Could not parse version number from Schema"
                        " string: %s",
                        schema_string.c_str()
                        )
                    );
            return;
        }

        const int target_version = (dg_version_it->second);

        const auto& type_rec = (
                TypeRegistry::instance()._find_type_record(schema_name)
                );

        while (current_version > target_version ) 
        {
            const auto& next_dg_fn = (
                    type_rec->downgrade_functions.find(
                        current_version
                    )
            );

            if (next_dg_fn == type_rec->downgrade_functions.end()) 
            {
                _internal_error(
                        string_printf(
                            "No downgrader function available for "
                            "going from version %d to version %d.",
                            current_version,
                            target_version
                            )
                        );
                return;
            }

            // apply it
            next_dg_fn->second(&m);

            current_version --;
        }

        m["OTIO_SCHEMA"] = schema_name + "." + std::to_string(current_version);
    }
};

template <typename RapidJSONWriterType>
class JSONEncoder : public Encoder
{
public:
    JSONEncoder(RapidJSONWriterType& writer)
        : _writer(writer)
    {}

    virtual ~JSONEncoder() {}

    void write_key(std::string const& key) { _writer.Key(key.c_str()); }

    void write_null_value() { _writer.Null(); }

    void write_value(bool value) { _writer.Bool(value); }

    void write_value(int value) { _writer.Int(value); }

    void write_value(int64_t value) { _writer.Int64(value); }

    void write_value(uint64_t value) { _writer.Uint64(value); }

    void write_value(std::string const& value)
    {
        _writer.String(value.c_str());
    }

    void write_value(double value) { _writer.Double(value); }

    void write_value(RationalTime const& value)
    {
        _writer.StartObject();

        _writer.Key("OTIO_SCHEMA");
        _writer.String("RationalTime.1");

        _writer.Key("rate");
        _writer.Double(value.rate());

        _writer.Key("value");
        _writer.Double(value.value());

        _writer.EndObject();
    }

    void write_value(TimeRange const& value)
    {
        _writer.StartObject();

        _writer.Key("OTIO_SCHEMA");
        _writer.String("TimeRange.1");

        _writer.Key("duration");
        write_value(value.duration());

        _writer.Key("start_time");
        write_value(value.start_time());

        _writer.EndObject();
    }

    void write_value(TimeTransform const& value)
    {
        _writer.StartObject();

        _writer.Key("OTIO_SCHEMA");
        _writer.String("TimeTransform.1");

        _writer.Key("offset");
        write_value(value.offset());

        _writer.Key("rate");
        _writer.Double(value.rate());

        _writer.Key("scale");
        _writer.Double(value.scale());

        _writer.EndObject();
    }

    void write_value(SerializableObject::ReferenceId value)
    {
        _writer.StartObject();
        _writer.Key("OTIO_SCHEMA");
        _writer.String("SerializableObjectRef.1");
        _writer.Key("id");
        _writer.String(value.id.c_str());
        _writer.EndObject();
    }

    void write_value(Imath::V2d const& value)
    {
        _writer.StartObject();

        _writer.Key("OTIO_SCHEMA");
        _writer.String("V2d.1");

        _writer.Key("x");
        _writer.Double(value.x);

        _writer.Key("y");
        _writer.Double(value.y);

        _writer.EndObject();
    }

    void write_value(Imath::Box2d const& value)
    {
        _writer.StartObject();

        _writer.Key("OTIO_SCHEMA");
        _writer.String("Box2d.1");

        _writer.Key("min");
        write_value(value.min);

        _writer.Key("max");
        write_value(value.max);

        _writer.EndObject();
    }

    void start_array(size_t) { _writer.StartArray(); }

    void start_object() { _writer.StartObject(); }

    void end_array() { _writer.EndArray(); }

    void end_object() { _writer.EndObject(); }

private:
    RapidJSONWriterType& _writer;
};

template <typename T>
bool
_simple_any_comparison(any const& lhs, any const& rhs)
{
    return lhs.type() == typeid(T) && rhs.type() == typeid(T) &&
           any_cast<T const&>(lhs) == any_cast<T const&>(rhs);
}

template <>
bool
_simple_any_comparison<void>(any const& lhs, any const& rhs)
{
    return lhs.type() == typeid(void) && rhs.type() == typeid(void);
}

template <>
bool
_simple_any_comparison<char const*>(any const& lhs, any const& rhs)
{
    return lhs.type() == typeid(char const*) &&
           rhs.type() == typeid(char const*) &&
           !strcmp(any_cast<char const*>(lhs), any_cast<char const*>(rhs));
}

void
SerializableObject::Writer::_build_dispatch_tables()
{
    /*
     * These are basically atomic writes to the encoder:
     */

    auto& wt          = _write_dispatch_table;
    wt[&typeid(void)] = [this](any const&) { _encoder.write_null_value(); };
    wt[&typeid(bool)] = [this](any const& value) {
        _encoder.write_value(any_cast<bool>(value));
    };
    wt[&typeid(int64_t)] = [this](any const& value) {
        _encoder.write_value(any_cast<int64_t>(value));
    };
    wt[&typeid(double)] = [this](any const& value) {
        _encoder.write_value(any_cast<double>(value));
    };
    wt[&typeid(std::string)] = [this](any const& value) {
        _encoder.write_value(any_cast<std::string const&>(value));
    };
    wt[&typeid(char const*)] = [this](any const& value) {
        _encoder.write_value(std::string(any_cast<char const*>(value)));
    };
    wt[&typeid(RationalTime)] = [this](any const& value) {
        _encoder.write_value(any_cast<RationalTime const&>(value));
    };
    wt[&typeid(TimeRange)] = [this](any const& value) {
        _encoder.write_value(any_cast<TimeRange const&>(value));
    };
    wt[&typeid(TimeTransform)] = [this](any const& value) {
        _encoder.write_value(any_cast<TimeTransform const&>(value));
    };
    wt[&typeid(Imath::V2d)] = [this](any const& value) {
        _encoder.write_value(any_cast<Imath::V2d const&>(value));
    };
    wt[&typeid(Imath::Box2d)] = [this](any const& value) {
        _encoder.write_value(any_cast<Imath::Box2d const&>(value));
    };

    /*
     * These next recurse back through the Writer itself:
     */
    wt[&typeid(SerializableObject::Retainer<>)] = [this](any const& value) {
        this->write(_no_key, any_cast<SerializableObject::Retainer<>>(value));
    };

    wt[&typeid(AnyDictionary)] = [this](any const& value) {
        this->write(_no_key, any_cast<AnyDictionary const&>(value));
    };

    wt[&typeid(AnyVector)] = [this](any const& value) {
        this->write(_no_key, any_cast<AnyVector const&>(value));
    };

    /*
     * Install a backup table, using the actual type name as a key.
     * This is to deal with type aliasing across compilation units.
     */
    for (const auto& e: wt)
    {
        _write_dispatch_table_by_name[e.first->name()] = e.second;
    }

    auto& et                   = _equality_dispatch_table;
    et[&typeid(void)]          = &_simple_any_comparison<void>;
    et[&typeid(bool)]          = &_simple_any_comparison<bool>;
    et[&typeid(int64_t)]       = &_simple_any_comparison<int64_t>;
    et[&typeid(double)]        = &_simple_any_comparison<double>;
    et[&typeid(std::string)]   = &_simple_any_comparison<std::string>;
    et[&typeid(char const*)]   = &_simple_any_comparison<char const*>;
    et[&typeid(RationalTime)]  = &_simple_any_comparison<RationalTime>;
    et[&typeid(TimeRange)]     = &_simple_any_comparison<TimeRange>;
    et[&typeid(TimeTransform)] = &_simple_any_comparison<TimeTransform>;
    et[&typeid(SerializableObject::ReferenceId)] =
        &_simple_any_comparison<SerializableObject::ReferenceId>;
    et[&typeid(Imath::V2d)]   = &_simple_any_comparison<Imath::V2d>;
    et[&typeid(Imath::Box2d)] = &_simple_any_comparison<Imath::Box2d>;

    /*
     * These next recurse back through the Writer itself:
     */
    et[&typeid(AnyDictionary)] = [this](any const& lhs, any const& rhs) {
        return _any_dict_equals(lhs, rhs);
    };
    et[&typeid(AnyVector)] = [this](any const& lhs, any const& rhs) {
        return _any_array_equals(lhs, rhs);
    };
}

bool
SerializableObject::Writer::_any_dict_equals(any const& lhs, any const& rhs)
{
    if (lhs.type() != typeid(AnyDictionary) ||
        rhs.type() != typeid(AnyDictionary))
    {
        return false;
    }

    AnyDictionary const& ld = any_cast<AnyDictionary const&>(lhs);
    AnyDictionary const& rd = any_cast<AnyDictionary const&>(rhs);

    auto r_it = rd.begin();

    for (const auto& l_it: ld)
    {
        if (r_it == rd.end())
        {
            return false;
        }

        if (l_it.first != r_it->first ||
            !_any_equals(l_it.second, r_it->second))
        {
            return false;
        }
        ++r_it;
    }
    return r_it == rd.end();
}

bool
SerializableObject::Writer::_any_array_equals(any const& lhs, any const& rhs)
{
    if (lhs.type() != typeid(AnyVector) || rhs.type() != typeid(AnyVector))
    {
        return false;
    }

    AnyVector const& lv = any_cast<AnyVector const&>(lhs);
    AnyVector const& rv = any_cast<AnyVector const&>(rhs);

    if (lv.size() != rv.size())
    {
        return false;
    }

    for (size_t i = 0; i < lv.size(); i++)
    {
        if (!_any_equals(lv[i], rv[i]))
        {
            return false;
        }
    }

    return true;
}

bool
SerializableObject::Writer::_any_equals(any const& lhs, any const& rhs)
{
    auto e = _equality_dispatch_table.find(&lhs.type());
    return (e != _equality_dispatch_table.end()) && e->second(lhs, rhs);
}

bool
SerializableObject::Writer::write_root(
    any const& value,
    Encoder& encoder,
    optional<const schema_version_map*> downgrade_version_manifest,
    ErrorStatus* error_status
)
{
    Writer w(encoder, downgrade_version_manifest);
    w.write(w._no_key, value);
    return !encoder.has_errored(error_status);
}

void
SerializableObject::Writer::_encoder_write_key(std::string const& key)
{
    if (&key != &_no_key)
    {
        _encoder.write_key(key);
    }
}

void
SerializableObject::Writer::write(std::string const& key, bool value)
{
    _encoder_write_key(key);
    _encoder.write_value(value);
}

void
SerializableObject::Writer::write(std::string const& key, int64_t value)
{
    _encoder_write_key(key);
    _encoder.write_value(value);
}

void
SerializableObject::Writer::write(std::string const& key, double value)
{
    _encoder_write_key(key);
    _encoder.write_value(value);
}

void
SerializableObject::Writer::write(
    std::string const& key, std::string const& value)
{
    _encoder_write_key(key);
    _encoder.write_value(value);
}

void
SerializableObject::Writer::write(std::string const& key, RationalTime value)
{
    _encoder_write_key(key);
    _encoder.write_value(value);
}

void
SerializableObject::Writer::write(std::string const& key, TimeRange value)
{
    _encoder_write_key(key);
    _encoder.write_value(value);
}

void
SerializableObject::Writer::write(
    std::string const& key, optional<RationalTime> value)
{
    _encoder_write_key(key);
    value ? _encoder.write_value(*value) : _encoder.write_null_value();
}

void
SerializableObject::Writer::write(
    std::string const& key, optional<TimeRange> value)
{
    _encoder_write_key(key);
    value ? _encoder.write_value(*value) : _encoder.write_null_value();
}

void
SerializableObject::Writer::write(
    std::string const& key, optional<Imath::Box2d> value)
{
    _encoder_write_key(key);
    value ? _encoder.write_value(*value) : _encoder.write_null_value();
}

void
SerializableObject::Writer::write(
        std::string const& key,
        TimeTransform value
)
{
    _encoder_write_key(key);
    _encoder.write_value(value);
}

void
SerializableObject::Writer::write(
    std::string const& key,
    SerializableObject const* value
)
{ 
    _encoder_write_key(key);
    if (!value)
    {
        _encoder.write_null_value();
        return;
    }

    auto e = _id_for_object.find(value);
    if (e != _id_for_object.end())
    {
#ifdef OTIO_INSTANCING_SUPPORT
        /*
         * We've already written this value.
         */
        _encoder.write_value(SerializableObject::ReferenceId{ e->second });
#else
        /*
         * We're encountering the same object while it is still
         * in the map, meaning we're in the middle of writing it out.
         * That's a cycle, as opposed to mere instancing, which we
         * allow so as not to break old allowed behavior.
         */
        std::string s = string_printf(
            "cyclically encountered object has schema %s",
            value->schema_name().c_str());
        _encoder._error(ErrorStatus(ErrorStatus::OBJECT_CYCLE, s));
#endif
        return;
    }

    std::string const& schema_type_name = value->_schema_name_for_reference();
    if (_next_id_for_type.find(schema_type_name) == _next_id_for_type.end())
    {
        _next_id_for_type[schema_type_name] = 0;
    }

    std::string next_id = schema_type_name + "-" +
                          std::to_string(++_next_id_for_type[schema_type_name]);
    _id_for_object[value] = next_id;

    std::string schema_str = "";

    // detect if downgrading needs to happen
    const std::string& schema_name = value->schema_name();
    int schema_version = value->schema_version();

    optional<AnyDictionary> downgraded = {};

    // if there is a manifest & the encoder is not converting to AnyDictionary
    if (
            _downgrade_version_manifest.has_value()    
            && !_encoder.encoding_to_anydict()
    ) 
    {
        const auto& dg_man = *(*_downgrade_version_manifest);
        const auto& target_version_it = dg_man.find(schema_name);

        // ...and if that downgrade manifest specifies a target version for
        // this schema
        if (target_version_it != dg_man.end())
        {
            const int target_version = target_version_it->second;

            // and the current_version is greater than the target version
            if (schema_version > target_version) 
            {
                CloningEncoder e(
                        CloningEncoder::ResultObjectPolicy::OnlyAnyDictionary,
                        _downgrade_version_manifest
                );

                Writer w(e, {});
                w.write(w._no_key, value);

                if (e.has_errored(&_encoder._error_status))
                {
                    return;
                }

                downgraded = { e.root() };
                schema_version = target_version;
            }
        }
    }

    // if its an unknown schema, the schema name is computed from the
    // _original_schema_name and _original_schema_version attributes
    if (UnknownSchema const* us = dynamic_cast<UnknownSchema const*>(value))
    {
        schema_str = (
                us->_original_schema_name 
                + "." 
                + std::to_string(us->_original_schema_version)
        );
    }
    else
    {
        // otherwise, use the schema_name and schema_version attributes
        schema_str = schema_name + "." + std::to_string(schema_version);
    }

    _encoder.start_object();

#ifdef OTIO_INSTANCING_SUPPORT
    _encoder.write_key("OTIO_REF_ID");
    _encoder.write_value(next_id);
#endif

    // write the contents of the object to the encoder, either the downgraded
    // anydictionary or the SerializableObject
    if (downgraded.has_value())
    {
        // the inner loop of write(
        for (const auto& kv : (*downgraded))
        {
            this->write(kv.first, kv.second);
        }
    }
    else
    {
        _encoder.write_key("OTIO_SCHEMA");
        _encoder.write_value(schema_str);
        value->write_to(*this);
    }

    _encoder.end_object();

#ifndef OTIO_INSTANCING_SUPPORT
    auto valueEntry = _id_for_object.find(value);
    if (valueEntry != _id_for_object.end())
    {
        _id_for_object.erase(valueEntry);
    }
#endif
}

void
SerializableObject::Writer::write(std::string const& key, Imath::V2d value)
{
    _encoder_write_key(key);
    _encoder.write_value(value);
}

void
SerializableObject::Writer::write(std::string const& key, Imath::Box2d value)
{
    _encoder_write_key(key);
    _encoder.write_value(value);
}

void
SerializableObject::Writer::write(
    std::string const& key, AnyDictionary const& value)
{
    _encoder_write_key(key);

    _encoder.start_object();

    for (const auto& e: value)
    {
        write(e.first, e.second);
    }

    _encoder.end_object();
}

void
SerializableObject::Writer::write(
    std::string const& key, AnyVector const& value)
{
    _encoder_write_key(key);

    _encoder.start_array(value.size());

    for (const auto& e: value)
    {
        write(_no_key, e);
    }

    _encoder.end_array();
}

void
SerializableObject::Writer::write(
        std::string const& key,
        any const& value)
{
    // if (value.empty()) 
    // {
    //     _encoder.write_key(key);
    //     _encoder.write_null_value();
    //     return;
    // }
    std::type_info const& type = value.type();

    _encoder_write_key(key);

    auto e = _write_dispatch_table.find(&type);
    if (e == _write_dispatch_table.end())
    {
        /*
         * Using the address of a type_info suffers from aliasing across compilation units.
         * If we fail on a lookup, we fallback on the by_name table, but that's slow because
         * we have to keep making a string each time.
         *
         * So when we fail, we insert the address of the type_info that failed to be found,
         * so that we'll catch it the next time.  This ensures we fail exactly once per alias
         * per type while using this writer.
         */
        auto backup_e = _write_dispatch_table_by_name.find(type.name());
        if (backup_e != _write_dispatch_table_by_name.end())
        {
            _write_dispatch_table[&type] = backup_e->second;
            e                            = _write_dispatch_table.find(&type);
        }
    }

    if (e != _write_dispatch_table.end())
    {
        e->second(value);
    }
    else
    {
        std::string s;
        std::string bad_type_name =
            (type == typeid(UnknownType))
                ? type_name_for_error_message(
                      any_cast<UnknownType>(value).type_name)
                : type_name_for_error_message(type);

        if (&key != &_no_key)
        {
            s = string_printf(
                "Encountered object of unknown type '%s' under key '%s'",
                bad_type_name.c_str(),
                key.c_str());
        }
        else
        {
            s = string_printf(
                "Encountered object of unknown type '%s'",
                bad_type_name.c_str());
        }

        _encoder._error(ErrorStatus(ErrorStatus::TYPE_MISMATCH, s));
        _encoder.write_null_value();
    }
}

bool
SerializableObject::is_equivalent_to(SerializableObject const& other) const
{
    if (_type_record() != other._type_record())
    {
        return false;
    }

    const auto policy = (
        CloningEncoder::ResultObjectPolicy::MathTypesConcreteAnyDictionaryResult
    );

    CloningEncoder             e1(policy), e2(policy);
    SerializableObject::Writer w1(e1, {});
    SerializableObject::Writer w2(e2, {});

    w1.write(w1._no_key, any(Retainer<>(this)));
    w2.write(w2._no_key, any(Retainer<>(&other)));

    return (
        !e1.has_errored() && !e2.has_errored() &&
        w1._any_equals(e1._root, e2._root));
}

SerializableObject*
SerializableObject::clone(ErrorStatus* error_status) const
{
    CloningEncoder e(
            CloningEncoder::ResultObjectPolicy::CloneBackToSerializableObject
    );
    SerializableObject::Writer w(e, {});

    w.write(w._no_key, any(Retainer<>(this)));
    if (e.has_errored(error_status))
    {
        return nullptr;
    }

    std::function<void(ErrorStatus const&)> error_function =
        [error_status](ErrorStatus const& status) {
            if (error_status)
            {
                *error_status = status;
            }
        };

    e._resolver.finalize(error_function);

    return e._root.type() == typeid(SerializableObject::Retainer<>)
               ? any_cast<SerializableObject::Retainer<>&>(e._root).take_value()
               : nullptr;
}

schema_version_map
_fetch_downgrade_manifest(
        family_label_spec target_family_label_spec
)
{
    const auto& family = target_family_label_spec.first;
    const auto& label = target_family_label_spec.second;

    // @TODO: error handling
    return FAMILY_LABEL_MAP[family][label];
}

// to json_string
std::string
serialize_json_to_string(
    const any& value,
    optional<family_label_spec> target_family_label_spec,
    ErrorStatus* error_status,
    int indent
)
{
    // @TODO: gross
    optional<const schema_version_map*> downgrade_version_manifest = {};
    schema_version_map mp;
    if (target_family_label_spec.has_value())
    {
        mp =  _fetch_downgrade_manifest(*target_family_label_spec);
        downgrade_version_manifest = { &mp };
    }

    OTIO_rapidjson::StringBuffer output_string_buffer;

    OTIO_rapidjson::PrettyWriter<
        decltype(output_string_buffer),
        OTIO_rapidjson::UTF8<>,
        OTIO_rapidjson::UTF8<>,
        OTIO_rapidjson::CrtAllocator,
        OTIO_rapidjson::kWriteNanAndInfFlag>
        json_writer(output_string_buffer);

    if (indent >= 0)
    {
        json_writer.SetIndent(' ', indent);
    }

    JSONEncoder<decltype(json_writer)> json_encoder(json_writer);

    if (
            !SerializableObject::Writer::write_root(
                value,
                json_encoder,
                downgrade_version_manifest,
                error_status
            )
    )
    {
        return std::string();
    }

    return std::string(output_string_buffer.GetString());

}

bool
serialize_json_to_file(
    any const&         value,
    std::string const& file_name,
    optional<family_label_spec> target_family_label_spec,
    ErrorStatus*       error_status,
    int                indent)
{
    // @TODO: gross
    optional<const schema_version_map*> downgrade_version_manifest = {};
    schema_version_map mp;
    if (target_family_label_spec.has_value())
    {
        mp =  _fetch_downgrade_manifest(*target_family_label_spec);
        downgrade_version_manifest = { &mp };
    }

#if defined(_WINDOWS)
    const int wlen =
        MultiByteToWideChar(CP_UTF8, 0, file_name.c_str(), -1, NULL, 0);
    std::vector<wchar_t> wchars(wlen);
    MultiByteToWideChar(CP_UTF8, 0, file_name.c_str(), -1, wchars.data(), wlen);
    std::ofstream os(wchars.data());
#else // _WINDOWS
    std::ofstream os(file_name);
#endif // _WINDOWS

    if (!os.is_open())
    {
        if (error_status)
        {
            *error_status =
                ErrorStatus(ErrorStatus::FILE_WRITE_FAILED, file_name);
        }
        return false;
    }

    OTIO_rapidjson::OStreamWrapper osw(os);
    bool                           status;

    OTIO_rapidjson::PrettyWriter<
        decltype(osw),
        OTIO_rapidjson::UTF8<>,
        OTIO_rapidjson::UTF8<>,
        OTIO_rapidjson::CrtAllocator,
        OTIO_rapidjson::kWriteNanAndInfFlag>
                                       json_writer(osw);
    JSONEncoder<decltype(json_writer)> json_encoder(json_writer);

    if (indent >= 0)
    {
        json_writer.SetIndent(' ', indent);
    }

    status = SerializableObject::Writer::write_root(
        value, 
        json_encoder,
        downgrade_version_manifest,
        error_status
    );

    return status;
}


}} // namespace opentimelineio::OPENTIMELINEIO_VERSION

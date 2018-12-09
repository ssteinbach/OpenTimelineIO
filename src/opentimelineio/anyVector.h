#ifndef OTIO_ANYVECTOR_H
#define OTIO_ANYVECTOR_H

#include "opentimelineio/any.h"
#include <vector>
#include <assert.h>

/*
 * A AnyVector has exactly the same API as
 *    std::vector<any>
 *
 * except that it records a "time-stamp" that
 * lets external observers know when the vector has been destroyed (which includes
 * the case of the vector being relocated in memory).
 *
 * This allows us to hand out iterators that can be aware of moves
 * and take steps to safe-guard themselves from causing a crash.
 */

class AnyVector : private std::vector<any> {
public:
    using vector::vector;

    AnyVector() {
        _mutation_stamp = nullptr;
    }

    ~AnyVector() {
        if (_mutation_stamp) {
            _mutation_stamp->any_vector = nullptr;
        }
    }
    
    AnyVector(const AnyVector& other)
        : vector(other),
          _mutation_stamp(nullptr) {
    }

    AnyVector& operator=(const AnyVector& other) {
        vector::operator= (other);
        return *this;
    }

    AnyVector& operator=(AnyVector&& other) {
        vector::operator= (other);
        return *this;
    }

    AnyVector& operator=(std::initializer_list<value_type> ilist) {
        vector::operator= (ilist);
        return *this;
    }

    using vector::assign;
    using vector::get_allocator;

    using vector::at;
    using vector::operator[];
    using vector::front;
    using vector::back;
    using vector::data;
        
    using vector::begin;
    using vector::cbegin;
    using vector::end;
    using vector::cend;
    using vector::rbegin;
    using vector::crbegin;
    using vector::rend;
    using vector::crend;

    using vector::empty;
    using vector::size;
    using vector::max_size;
    using vector::reserve;
    using vector::capacity;
    using vector::shrink_to_fit;

    using vector::swap;
    using vector::clear;
    using vector::insert;
    using vector::emplace;
    using vector::erase;
    using vector::push_back;
    using vector::emplace_back;
    using vector::pop_back;
    using vector::resize;

    using vector::value_type;
    using vector::allocator_type;
    using vector::size_type;
    using vector::difference_type;
    using vector::reference;
    using vector::const_reference;
    using vector::pointer;
    using vector::const_pointer;
    using vector::iterator;
    using vector::const_iterator;
    using vector::reverse_iterator;
    using vector::const_reverse_iterator;
    
    void swap(AnyVector& other) {
        vector::swap(other);
    }

    struct MutationStamp {
        MutationStamp(AnyVector* v)
            : any_vector(v),
              owning(false) {
            assert(v != nullptr);
        }
            
        MutationStamp(MutationStamp const&) = delete;
        MutationStamp& operator=(MutationStamp const&) = delete;
        
        ~MutationStamp() {
            if (any_vector) {
                any_vector->_mutation_stamp = nullptr;
                if (owning) {
                    delete any_vector;
                }
            }
        }

        AnyVector* any_vector;
        bool owning;

    protected:
        MutationStamp()
            : any_vector(new AnyVector),
              owning(true) {
            any_vector->_mutation_stamp = this;
        }
    };
    
    MutationStamp* get_or_create_mutation_stamp() {
        if (!_mutation_stamp) {
            _mutation_stamp = new MutationStamp(this);
        }
        return _mutation_stamp;
    }
    
    friend struct MutationStamp;
    
private:
    MutationStamp* _mutation_stamp = nullptr;
};

#endif
// Copyright (c) 2007, 2009 University of Twente
// Copyright (c) 2007, 2009 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef DENSE_SET_H_INCLUDED
#define DENSE_SET_H_INCLUDED

#include <memory>
#include <utility>

// N.B. this class is far from finished!

template< class Key, class Alloc = std::allocator<bool> >
class DenseSet
{
public:
    class Iterator
    {
        const DenseSet *set_;
        Key key_;

    public:
        typedef ssize_t                    difference_type;
        typedef std::forward_iterator_tag  iterator_category;
        typedef Key                        value_type;
        typedef Key*                       pointer;
        typedef Key&                       reference;

        Iterator(const DenseSet *set, Key key) : set_(set), key_(key) { };
        Iterator(const Iterator &it) : set_(it.set_), key_(it.key_) { };

        Iterator& operator=(const Iterator &it)
        {
            set_ = it.set_;
            key_ = it.key_;
            return *this;
        }

        bool operator== (const Iterator &other) { return key_ == other.key_; }
        bool operator!= (const Iterator &other) { return key_ != other.key_; }
        bool operator<  (const Iterator &other) { return key_ <  other.key_; }
        bool operator>  (const Iterator &other) { return key_ >  other.key_; }
        bool operator<= (const Iterator &other) { return key_ <= other.key_; }
        bool operator>= (const Iterator &other) { return key_ >= other.key_; }

        value_type operator*() { return key_; };

        Iterator& operator++()
        {
            do ++key_; while (!set_->used_[key_]);
            return *this;
        }

        Iterator& operator++(int)
        {
            Iterator copy = *this;
            ++*this;
            return copy;
        }
    };

    typedef size_t size_type;
    typedef Key value_type;
    typedef Iterator iterator;
    typedef Iterator const_iterator;

    DenseSet(Key begin, Key end, const Alloc &alloc = Alloc())
        : range_begin(begin), range_end(end < begin ? begin : end),
          range_size_(range_end - range_begin), alloc_(alloc),
          used_(alloc_.allocate(range_size_ + 1)), num_used_(0)
    {
        for (size_type i = 0; i < range_size_; ++i) used_[i] = false;
        used_[range_size_] = true;  // marks end of data
    }

    ~DenseSet()
    {
        alloc_.deallocate(used_, range_size_ + 1);
    }

    size_type size() const
    {
        return num_used_;
    }

    bool empty() const
    {
        return num_used_ == 0;
    }

    void clear()
    {
        if (num_used_ > 0)
        {
            for (size_type i = 0; i < range_size_; ++i) used_[i] = false;
            num_used_ = 0;
        }
    }

    iterator begin()
    {
        Key k = range_begin;
        while (!used_[k - range_begin]) ++k;
        return iterator(this, k);
    }

    iterator end()
    {
        return iterator(this, range_size_);
    }

    const_iterator begin() const
    {
        return iterator(const_cast<DenseSet*>(this)->begin());  // HACK
    }

    const_iterator end() const
    {
        return iterator(const_cast<DenseSet*>(this)->end());  // HACK
    }

    iterator find(const Key &k)
    {
        if ( /* k >= range_begin && k < range_end && */ used_[k - range_begin] )
        {
            return iterator(this, k);
        }
        else
        {
            return end();
        }
    }

    const_iterator find(const Key &k) const
    {
        return const_iterator(const_cast<DenseSet*>(this)->find(k));  // HACK
    }

    size_type count(const Key &k) const
    {
        return used_[k - range_begin];
    }

    std::pair<iterator, bool> insert(const Key &k)
    {
        bool &u = used_[k - range_begin];
        if (u)
        {
            return std::pair<iterator, bool>(iterator(this, k), false);
        }
        else
        {
            u = true;
            ++num_used_;
            return std::pair<iterator, bool>(iterator(this, k), true);
        }
    }

    size_t memory_use() { return sizeof(*used_)*range_size_ + sizeof(*this); }

public:
    const Key range_begin, range_end;

private:
    const size_type range_size_;
    Alloc           alloc_;
    bool            *used_;
    size_t          num_used_;

    friend class Iterator;
};

#endif /* ndef DENSE_SET_H_INCLUDED */
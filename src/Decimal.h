// Copyright (c) 2009-2013 University of Twente
// Copyright (c) 2009-2013 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2013 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2013 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef DECIMAL_H_INCLUDED
#define DECIMAL_H_INCLUDED

#include <string>
#include <sstream>

//! Arbitrary-precision natural numbers with an internal decimal representation.
class Decimal
{
    static std::string str(unsigned i)
    {
        std::ostringstream oss;
        oss << i;
        return oss.str();
    }

public:
    /*! Construct a Decimal from a string.
        \param t A string consisting of decimal digits without leading zeros. */
    Decimal(const std::string &t) : s(t) { }

    //! Construct a Decimal from an unsigned integer.
    Decimal(const unsigned i) : s(str(i)) { }

    //! Copy constructor.
    Decimal(const Decimal &d) : s(d.s) { }

    //! Assignment operator.
    Decimal &operator=(const Decimal &d) { s = d.s; return *this; }

    //! Returns the underlying decimal representation as a std::string.
    const std::string &str() const { return s; }

    //! Returns a character pointer to underlying decimal representation.
    const char *c_str() const { return s.c_str(); }

    //! Returns the length of the underlying decimal representation.
    size_t size() const { return s.size(); }

    //! Returns the result of adding this number to the given argument.
    Decimal operator+(const Decimal &d) const;

    //! Returns the result of multiplying this number with the given argument.
    Decimal operator*(const Decimal &d) const;

private:
    /*! Returns the digit at the `i`-th position, counted from the left.
        This is equivalent to `str()[i]` which means that `i` must be between
        `0` and `size()`, exclusive! */
    char operator[](size_t i) const { return s[i]; }

    std::string s;  //! internal representation as a decimal number
};

#endif /* ndef DECIMAL_H_INCLUDED */

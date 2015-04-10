/*
 * property.h
 *
 *  Created on: 25 жовт. 2014
 *      Author: oleg
 */

#ifndef PROPERTY_H_
#define PROPERTY_H_

#include <types.h>
#include <demangle.h>
#include <bd.h>
#include <exception.h>

#include <string>
#include <map>
#include <string.h>
#include <boost/endian/conversion.hpp>
#include <boost/any.hpp>
#include <boost/format.hpp>

typedef std::string (*bd_to_string)(bd_block *block);

struct bd_property
{
    boost::any value;

    bd_property()
    {
    }

    bd_property(const bd_property &other) : value(other.value)
    {
    }

    template<class T>
    bd_property(const T &val) : value(val)
    {
    }

    bool is_empty()
    {
        return value.empty();
    }

    const std::type_info &type() const
    {
        return value.type();
    }

    template<class T>
    bool is_type() const
    {
        return value.type() == typeid(T);
    }

    template<class T>
    T get()
    {
        try
        {
            return boost::any_cast<T>(value);
        }
        catch (const boost::bad_any_cast &ex)
        {
            throw BlockTemplException((boost::format("Wrong cast. Expected: %1%. Actual: %2%.")
                                       % get_type_name<T>() % get_type_name(type())).str());
        }
    }
};

typedef std::map<std::string, bd_property> bd_property_records;
extern bd_property_records default_property;

enum bd_endian
{
    BD_ENDIAN_NONE, // use system specific default endian
    BD_ENDIAN_LIT,  // little endian
    BD_ENDIAN_BIG,  // big endian
};

template<class T>
T correct_endian(T val, bd_endian endian)
{
    if (endian == BD_ENDIAN_LIT)
    {
        T res = boost::endian::little_endian_value(val);
        return res;
    }
    if (endian == BD_ENDIAN_BIG)
    {
        T res = boost::endian::big_endian_value(val);
        return res;
    }
    return val;
}

#endif /* PROPERTY_H_ */

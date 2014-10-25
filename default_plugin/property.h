/*
 * property.h
 *
 *  Created on: 25 жовт. 2014
 *      Author: oleg
 */

#ifndef PROPERTY_H_
#define PROPERTY_H_

#include <types.h>
#include <bd.h>

#include <string>
#include <map>
#include <string.h>
#include <boost/endian/conversion.hpp>

typedef bd_result (*bd_to_string)(bd_block *block, char *buf, bd_u32 size);

union bd_property_value
{
    bd_i32       i;
    bd_f64       d;
    bd_string    s;
    bd_to_string to_s;
};

struct bd_property
{
    bd_property_type  type;
    bd_property_value value;

    bd_property() : type(BD_PROP_UNDEF)
    {
        value.i = 0;
        memset(&value, 0, sizeof(value));
    }

    bd_property(const bd_property &other) : type(BD_PROP_UNDEF)
    {
        switch (other.type)
        {
        case BD_PROP_INTEGER:
            *this = bd_property(other.value.i);
            break;
        case BD_PROP_DOUBLE:
            *this = bd_property(other.value.d);
            break;
        case BD_PROP_STRING:
            *this = bd_property(other.value.s);
            break;
        case BD_PROP_TO_STRING:
            *this = bd_property(other.value.to_s);
            break;
        default:
            *this = bd_property();
            break;
        }
    }

    bd_property(bd_i32 val) : type(BD_PROP_INTEGER)
    {
        value.i = val;
    }

    bd_property(bd_f64 val) : type(BD_PROP_DOUBLE)
    {
        value.d = val;
    }

    bd_property(const bd_string val) : type(BD_PROP_STRING)
    {
        if (val != nullptr)
        {
            value.s = new bd_char[strlen(val) + 1];
            strncpy(value.s, val, strlen(val));
            value.s[strlen(val)] = 0;
        }
        else
        {
            value.s = nullptr;
        }
    }

    bd_property(bd_to_string val) : type(BD_PROP_TO_STRING)
    {
        value.to_s = val;
    }

    ~bd_property()
    {
        if (type == BD_PROP_STRING && value.s != nullptr)
            delete[] value.s;
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

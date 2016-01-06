/*
 * property.cpp
 *
 *  Created on: 25 жовт. 2014
 *      Author: oleg
 */

#include "property.h"

bd_property_records default_property = {
    {"endian",   (bd_i32) BD_ENDIAN_NONE},
    {"tostring", (bd_to_string) nullptr}, // default implementation
    {"format",   (bd_i32) BD_ENDIAN_NONE},
    {"size",     (bd_u64) BD_ENDIAN_NONE}, // fixed size of the template
};

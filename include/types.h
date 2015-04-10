/*
 * types.h
 *
 *  Created on: Jul 13, 2013
 *      Author: Oleg Khryptul
 */

#ifndef BD_TYPES_H_
#define BD_TYPES_H_

#include <stdint.h>

typedef int8_t   bd_i8;
typedef uint8_t  bd_u8;
typedef int16_t  bd_i16;
typedef uint16_t bd_u16;
typedef int32_t  bd_i32;
typedef uint32_t bd_u32;
typedef int64_t  bd_i64;
typedef uint64_t bd_u64;

typedef float     bd_f32;
typedef double    bd_f64;

typedef bd_i32    bd_bool;
#define BD_TRUE  1
#define BD_FALSE 0

typedef char    bd_char;
typedef wchar_t bd_wchar;

typedef bd_char*         bd_string;
typedef const bd_string  bd_cstring;
typedef bd_wchar*        bd_wstring;
typedef const bd_wstring bd_cwstring;

typedef void*     bd_pointer;
#define BD_NULL 0

typedef bd_i32 bd_result; /// 0 - success, < 0 - errors defined by plugin, > 0 - other system specific info

enum bd_base_result_code
{
    BD_SUCCESS = 0,
    BD_ERROR   = 1, // undefined error
    BD_EUSER   = 2, // user defined error
};
#define BD_SUCCEED(result) (result == BD_SUCCESS)

#define BD_BLOCK_TYPES \
    BD_BLOCK_TYPE_DECL(CHAR,   bd_i8)  \
    BD_BLOCK_TYPE_DECL(UCHAR,  bd_u8)  \
    BD_BLOCK_TYPE_DECL(WORD,   bd_i16) \
    BD_BLOCK_TYPE_DECL(DWORD,  bd_i32) \
    BD_BLOCK_TYPE_DECL(QWORD,  bd_i64) \
    BD_BLOCK_TYPE_DECL(DOUBLE, bd_f64)

typedef enum /*bd_block_type*/
{
    BD_TEMPL = 0, // user defined template
    BD_STRING,

    // simple types
#define BD_BLOCK_TYPE_DECL(name, type) BD_##name,
    BD_BLOCK_TYPES
#undef BD_BLOCK_TYPE_DECL

    BD_STRUCT, // user defined plain structure
} bd_block_type;

// Template types
typedef char* STRING_T;

#define BD_BLOCK_TYPE_DECL(name, type) typedef type name##_T;
    BD_BLOCK_TYPES
#undef BD_BLOCK_TYPE_DECL

//const char *get_block_type_name(bd_block_type type)
//{
//    switch (type)
//    {
//    case BD_TEMPL:
//        return "TEMPL";
//#define BD_BLOCK_TYPE_DECL(name, tp) \
//    case BD_##name: {                \
//        return #name; }
//    BD_BLOCK_TYPES
//#undef BD_BLOCK_TYPE_DECL
//    }
//    return "<UNDEF>";
//}

typedef enum /*bd_property_type*/
{
    BD_PROP_UNDEF,     // undefined
    BD_PROP_BYTE,      // bd_i8
    BD_PROP_SHORT,     // bd_i16
    BD_PROP_INTEGER,   // bd_i32
    BD_PROP_LONG,      // bd_i64
    BD_PROP_DOUBLE,    // bd_f64
    BD_PROP_STRING,    // bd_string
    BD_PROP_TO_STRING, // std::string (*bd_to_string)(bd_block *block)
} bd_property_type;

#endif // BD_TYPES_H_

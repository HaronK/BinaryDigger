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

typedef bd_i32 bd_result; /// 0 - success, < 0 - errors defined by plugin, > 0 - other system specific info

enum bd_base_result_code
{
    BD_SUCCESS = 0,
    BD_ERROR   = 1, // undefined error
    BD_EUSER   = 2, // user defined error
};
#define BD_SUCCEED(result) (result == BD_SUCCESS)


#endif // BD_TYPES_H_

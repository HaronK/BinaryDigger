/*
 * result_codes.cpp
 *
 *  Created on: Jul 16, 2013
 *      Author: okhryptul
 */

#define RESULT_CODES_DECL                                             \
    BD_DECL_ERROR(OUT_OF_BOUNDS,     "Index out of bounds: %u >= %u") \
    BD_DECL_ERROR(ELEMENT_NOT_FOUND, "Element")

enum bd_default_plugin_error_code
{
    BD_STUB = BD_SUCCESS, // start from correct number

#define BD_DECL_ERROR(id, msg) BD_E##id,
    RESULT_CODES_DECL
#undef BD_DECL_ERROR

    BD_DEFAULT_PLUGIN_ERROR_CODE_COUNT
};

char const *bd_default_plugin_error_message[] = {
    "Operation succeeded",

#define BD_DECL_ERROR(id, msg) msg,
    RESULT_CODES_DECL
#undef BD_DECL_ERROR
};

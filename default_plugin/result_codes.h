/*
 * result_codes.cpp
 *
 *  Created on: Jul 16, 2013
 *      Author: okhryptul
 */

#define BD_DECL_ERROR(id, msg) BD_E##id,
enum bd_default_plugin_error_code
{
    BD_STUB = BD_SUCCESS, // start from correct number
    #include <result_codes_decl.h>
    BD_DEFAULT_PLUGIN_ERROR_CODE_COUNT
};


#define BD_DECL_ERROR(id, msg) msg,
char const *bd_default_plugin_error_message[] = {
    "Operation succeeded",
    #include <result_codes_decl.h>
};

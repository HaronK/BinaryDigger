/*
 * FecTemplate.cpp
 *
 *  Created on: Oct 10, 2014
 *      Author: okhryptul
 */

#include <default_plugin.h>

const u_int SHA_DIGEST_LENGTH = 20;

TEMPL(Fec)
    VAR(DWORD, version);
    ARR(UCHAR, sourceHash, SHA_DIGEST_LENGTH);
    ARR(UCHAR, formulaHash, SHA_DIGEST_LENGTH);
    VAR(DWORD, size);
    ARR(UCHAR, formula, VAL(size));
TEMPL_END

PLUGIN(Fec)
    TEMPL_REGISTER(Fec);
PLUGIN_END

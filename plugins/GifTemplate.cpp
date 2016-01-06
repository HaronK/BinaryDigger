/*
 * GifTemplate.cpp
 *
 *  Created on: Feb 26, 2014
 *      Author: okhryptul
 */

#include <default_plugin.h>

TEMPL(GifHeader)
    ARR(CHAR, tag, 3);
    ARR(CHAR, version, 3);
TEMPL_END

TEMPL(Gif)
    VAR(GifHeader, header);
TEMPL_END

PLUGIN(Gif)
    TEMPL_REGISTER(Gif);
PLUGIN_END

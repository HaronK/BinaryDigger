/*
 * result_codes.h
 *
 *  Created on: Jul 16, 2013
 *      Author: okhryptul
 */

BD_DECL_ERROR(OUT_OF_BOUNDS,     "Index out of bounds: %u >= %u")
BD_DECL_ERROR(ELEMENT_NOT_FOUND, "Element")

#undef BD_DECL_ERROR

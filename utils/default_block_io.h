/*
 * default_block_io.h
 *
 *  Created on: 25 April 2014
 *      Author: oleg
 */

#ifndef DEFAULT_BLOCK_IO_H_
#define DEFAULT_BLOCK_IO_H_

#include <bd.h>
#include <fstream>

struct bd_default_block_io : bd_block_io
{
    bd_default_block_io(const std::string& path);

    std::fstream dataFile;
};

#endif /* DEFAULT_BLOCK_IO_H_ */

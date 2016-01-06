/*
 * default_block_io.cpp
 *
 *  Created on: 25 Aug 2014
 *      Author: Oleg Khryptul
 */

#include "default_block_io.h"

#include <functional>

bd_result process_block_io_operation(bd_block_io *self, std::function<void (bd_default_block_io*)> operation)
{
    bd_default_block_io* block_io = (bd_default_block_io *)self;
    try
    {
        operation(block_io);
    }
    catch (const std::ios_base::failure&)
    {
        return -1;
    }
    return BD_SUCCESS;

}

bd_result _get_pos(bd_block_io *self, bd_u64 *_pos)
{
    return process_block_io_operation(self, [=](bd_default_block_io* block_io)
    {
        *_pos = block_io->dataFile.tellg();
    });
}

bd_result _set_pos(bd_block_io *self, bd_u64 pos)
{
    return process_block_io_operation(self, [=](bd_default_block_io* block_io)
    {
        block_io->dataFile.seekg(pos);
    });
}

bd_result _shift_pos(bd_block_io *self, bd_u64 offset)
{
    return process_block_io_operation(self, [=](bd_default_block_io* block_io)
    {
        block_io->dataFile.seekg(offset, std::ios_base::cur);
    });
}

bd_result _get_data(bd_block_io *self, bd_u64 size, bd_pointer val)
{
    return process_block_io_operation(self, [=](bd_default_block_io* block_io)
    {
        block_io->dataFile.read((char*)val, size);
    });
}

bd_result _get_datap(bd_block_io *self, bd_u64 pos, bd_u64 size, bd_pointer val)
{
    return process_block_io_operation(self, [=](bd_default_block_io* block_io)
    {
        std::streampos cur_pos = block_io->dataFile.tellg();
        block_io->dataFile.seekg(pos);
        block_io->dataFile.read((char*)val, size);
        block_io->dataFile.seekg(cur_pos);
    });
}

bd_default_block_io::bd_default_block_io(const std::string& path)
{
    // TODO: set correct internal state flags for dataFile if needed

    dataFile.open(path, std::ios::in | std::ios::binary);

    get_pos   = _get_pos;
    set_pos   = _set_pos;
    shift_pos = _shift_pos;
    get_data  = _get_data;
    get_datap = _get_datap;
}

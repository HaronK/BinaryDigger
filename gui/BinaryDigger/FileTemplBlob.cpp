
#include "FileTemplBlob.h"

bd_result _get_pos(bd_templ_blob *self, bd_u64 *_pos)
{
    FileTemplBlob *blob = (FileTemplBlob *)self;
    *_pos = blob->dataFile->pos();
    return BD_SUCCESS;
}

bd_result _set_pos(bd_templ_blob *self, bd_u64 pos)
{
    FileTemplBlob *blob = (FileTemplBlob *)self;
    return blob->dataFile->seek(pos) ? BD_SUCCESS : -1;
}

bd_result _shift_pos(bd_templ_blob *self, bd_u64 offset)
{
    FileTemplBlob *blob = (FileTemplBlob *)self;
    qint64 _pos = blob->dataFile->pos();
    return blob->dataFile->seek(_pos + offset) ? BD_SUCCESS : -1;
}

bd_result _get_data(bd_templ_blob *self, bd_u64 size, bd_pointer val)
{
    FileTemplBlob *blob = (FileTemplBlob *)self;

    qint64 data_read = blob->dataFile->read((char*)val, size);

    return (bd_u64) data_read == size ? BD_SUCCESS : -1;
}

bd_result _get_datap(bd_templ_blob *self, bd_u64 pos, bd_u64 size, bd_pointer val)
{
    FileTemplBlob *blob = (FileTemplBlob *)self;

    qint64 cur_pos = blob->dataFile->pos();

    if (!blob->dataFile->seek(pos))
        return -1;

    qint64 data_read = blob->dataFile->read((char*)val, size);

    // restore original position
    if (!blob->dataFile->seek(cur_pos))
        return -1;

    return (bd_u64) data_read == size ? BD_SUCCESS : -1;
}

FileTemplBlob::FileTemplBlob()
{
    get_pos   = _get_pos;
    set_pos   = _set_pos;
    shift_pos = _shift_pos;
    get_data  = _get_data;
    get_datap = _get_datap;
}

FileTemplBlob::~FileTemplBlob()
{
//    dataFile->close();
}

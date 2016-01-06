#ifndef FILETEMPLBLOB_H
#define FILETEMPLBLOB_H

#include "../../include/bd.h"
#include <QFile>
#include <QSharedPointer>

struct FileTemplBlob : bd_block_io
{
    FileTemplBlob();
    ~FileTemplBlob();

    QSharedPointer<QFile> dataFile;
};

#endif // FILETEMPLBLOB_H

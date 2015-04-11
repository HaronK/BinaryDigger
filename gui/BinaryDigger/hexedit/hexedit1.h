#pragma once

#include <qhexedit2/qhexedit.h>

class HexEdit : public QHexEdit
{
public:
//    void setFile(QSharedPointer<QFile> const &dataFile)
//    {
//    }

    void setSelectionLength(qint64 begin, qint64 length)
    {
        setSelectionRange(begin, begin + length);
    }
};

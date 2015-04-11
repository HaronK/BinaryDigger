#pragma once

#include <QHexEdit_Dax89/qhexedit.h>

// https://github.com/Dax89/QHexEdit

class HexEdit : public QHexEdit
{
public:
    void setFile(QSharedPointer<QFile> const &dataFile)
    {
        QHexEditData* hexeditdata = QHexEditData::fromDevice(dataFile.data());
        setData(hexeditdata);
    }

//    void setSelectionLength(qint64 begin, qint64 length)
//    {
//    }

    int cursorPosition()
    {
        return 0;
    }

    int lastIndexOf(const QByteArray & ba, int from = 0) const
    {
        return 0;
    }

    int indexOf(const QByteArray & ba, int from = 0) const
    {
        return 0;
    }

    void replace( int pos, int len, const QByteArray & after)
    {
    }

    QString selectionToReadableString()
    {
        return "";
    }

    QString toReadableString()
    {
        return "";
    }

    bool overwriteMode()
    {
        return false;
    }

    void saveTo(QFile &file)
    {
    }

signals:

    /*! Contains the address, where the cursor is located. */
    void currentAddressChanged(int address);

    /*! Contains the size of the data to edit. */
    void currentSizeChanged(int size);

    /*! The signal is emited every time, the data is changed. */
    void dataChanged();

    /*! The signal is emited every time, the overwrite mode is changed. */
    void overwriteModeChanged(bool state);
};

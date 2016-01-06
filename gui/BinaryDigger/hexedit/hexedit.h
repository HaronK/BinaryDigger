#pragma once

#include <qhexedit.h>

// https://github.com/Dax89/QHexEdit
// or
// https://github.com/amaterasu-/QHexEdit

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

//    qint64 cursorPosition()
//    {
//        return 0;
//    }

    qint64 lastIndexOf(const QByteArray & /*ba*/, qint64 /*from = 0*/) const
    {
        return 0;
    }

//    qint64 indexOf(const QByteArray & /*ba*/, qint64 /*from = 0*/) const
//    {
//        return 0;
//    }

    void replace( int /*pos*/, int /*len*/, const QByteArray & /*after*/)
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

    void saveTo(QFile &/*file*/)
    {
    }

signals:

    /*! Contains the address, where the cursor is located. */
    void currentAddressChanged(int /*address*/);

    /*! Contains the size of the data to edit. */
    void currentSizeChanged(int /*size*/);

    /*! The signal is emited every time, the data is changed. */
    void dataChanged();

    /*! The signal is emited every time, the overwrite mode is changed. */
    void overwriteModeChanged(bool /*state*/);
};

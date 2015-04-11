#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QtCore>

#include "hexedit/hexedit.h"

namespace Ui {
    class SearchDialog;
}

class SearchDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SearchDialog(HexEdit *hexEdit, QWidget *parent = 0);
    ~SearchDialog();
    int findNext();
    Ui::SearchDialog *ui;

private slots:
    void on_pbFind_clicked();
    void on_pbReplace_clicked();
    void on_pbReplaceAll_clicked();

private:
    QByteArray getContent(int comboIndex, const QString &input);
    int replaceOccurrence(int idx, const QByteArray &replaceBa);

    HexEdit *_hexEdit;
};

#endif // SEARCHDIALOG_H

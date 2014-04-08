
#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include "../../include/bd.h"
#include "treeitem.h"

//! [0]
class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TreeModel(const QStringList &headers, bd_templ_blob *templ_blob, const bd_item *root_item,
              QObject *parent = 0);
    ~TreeModel();
//! [0]

//! [1]

    struct UserData
    {
        bd_u32 index;
        const bd_item* item;

        UserData() : index((bd_u32)-1), item(0) {}
    };

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
//! [1]

//! [2]
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole);
    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant &value, int role = Qt::EditRole);

    bool insertColumns(int position, int columns,
                       const QModelIndex &parent = QModelIndex());
    bool removeColumns(int position, int columns,
                       const QModelIndex &parent = QModelIndex());
    bool insertRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex());
    bool removeRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex());

    TreeItem<UserData> *getItem(const QModelIndex &index) const;

private:
    void setupModelData(const QStringList &lines, TreeItem<UserData> *parent);

    bd_templ_blob *templ_blob;
    TreeItem<UserData> *rootItem;

    void generateModelData(const bd_item *item, bd_u32 index, TreeItem<UserData> *parent);
    const bd_item* hdRootItem;
};
//! [2]

#endif // TREEMODEL_H

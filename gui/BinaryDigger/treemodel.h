
#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include "../../include/bd.h"
#include "treeitem.h"

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TreeModel(const QStringList &headers, bd_block_io *block_io, const bd_block *root_block,
              QObject *parent = 0);
    ~TreeModel();

    struct UserData
    {
        bd_u32 index;
        const bd_block* block;

        UserData() : index((bd_u32)-1), block(0) {}
    };

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

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

    TreeItem<UserData> *getBlock(const QModelIndex &index) const;

private:
    void setupModelData(const QStringList &lines, TreeItem<UserData> *parent);

    bd_block_io *block_io;
    TreeItem<UserData> *rootItem;

    void generateModelData(const bd_block *block, bd_u32 index, TreeItem<UserData> *parent);
    const bd_block* hdRootBlock;
};

#endif // TREEMODEL_H


#include <QtWidgets>
#include <QMessageBox>

#include "treeitem.h"
#include "treemodel.h"

#include <BDException.h>

TreeModel::TreeModel(const QStringList &headers, bd_plugin *plugin, bd_block_io *block_io, const bd_block *root_block, QObject *parent)
    : QAbstractItemModel(parent), plugin(plugin), block_io(block_io), hdRootBlock(root_block)
{
    QVector<QVariant> rootData;
    foreach (QString header, headers)
        rootData << header;

    rootItem = new TreeItem<UserData>(rootData);
    UserData userData;
    userData.index = (bd_u32)-1;
    userData.block = root_block;
    rootItem->setUserData(userData);
    generateModelData(hdRootBlock, (bd_u32)-1, rootItem);
}

TreeModel::~TreeModel()
{
    delete rootItem;
}

int TreeModel::columnCount(const QModelIndex & /* parent */) const
{
    return rootItem->columnCount();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    TreeItem<UserData> *item = getBlock(index);

    return item->data(index.column());
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

TreeItem<TreeModel::UserData> *TreeModel::getBlock(const QModelIndex &index) const
{
    if (index.isValid())
    {
        TreeItem<UserData> *item = static_cast<TreeItem<UserData>*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    TreeItem<UserData> *parentItem = getBlock(parent);

    TreeItem<UserData> *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

bool TreeModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
    bool success;

    beginInsertColumns(parent, position, position + columns - 1);
    success = rootItem->insertColumns(position, columns);
    endInsertColumns();

    return success;
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem<UserData> *parentItem = getBlock(parent);
    bool success;

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, rootItem->columnCount());
    endInsertRows();

    return success;
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem<UserData> *childItem = getBlock(index);
    TreeItem<UserData> *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool TreeModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
    bool success;

    beginRemoveColumns(parent, position, position + columns - 1);
    success = rootItem->removeColumns(position, columns);
    endRemoveColumns();

    if (rootItem->columnCount() == 0)
        removeRows(0, rowCount());

    return success;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem<UserData> *parentItem = getBlock(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem<UserData> *parentItem = getBlock(parent);

    return parentItem->childCount();
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    TreeItem<UserData> *item = getBlock(index);
    bool result = item->setData(index.column(), value);

    if (result)
        emit dataChanged(index, index);

    return result;
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    bool result = rootItem->setData(section, value);

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

void TreeModel::generateModelData(const bd_block *block, bd_u32 index, TreeItem<UserData> *parent)
{
    if (block == 0)
    {
        QMessageBox::warning(0, tr("Parsed tree"), tr("Parsed tree is empty."), QMessageBox::Ok);
        return;
    }

    QString str;

    int parentChildrenCount = parent->childCount();
    parent->insertChildren(parentChildrenCount, 1, rootItem->columnCount());

    bool is_array_elem = block->parent != NULL && block->parent->is_array == BD_TRUE;
    TreeItem<UserData> *child = parent->child(parentChildrenCount);
    UserData userData;
    userData.index = is_array_elem ? index : (bd_u32)-1;
    userData.block = block;
    child->setUserData(userData);

    if (block->is_array)
    {
        str.sprintf("%s[%d]", block->name, block->count);
    }
    else
    {
        str.sprintf("%s", block->name);
    }
    child->setData(0, str);                                // Name
    child->setData(2, block->type_name);                   // Type
    child->setData(3, str.sprintf("%lXh", block->offset)); // Start
    child->setData(4, str.sprintf("%lXh", block->size));   // Size

    bd_u32 buf_size = 100;
    if (is_string(block))
    {
        buf_size = block->size + 1;
    }

    bd_string buf = new bd_char[buf_size];

    bd_result res = plugin->get_string_value((bd_block *) block, buf, buf_size);
    if (!(res == BD_SUCCESS))
    {
        char buf[1024];
        plugin->result_message(res, (bd_string) buf, sizeof(buf));

        // TODO: substitute exception with message box
        throw BDException(tr("%1\n%2").arg(tr("Cannot get string representation of block value")).arg(buf));
    }

    str = buf;

    delete[] buf;

    child->setData(1, str); // Value

    for (bd_u32 i = 0; i < block->children.count; ++i)
    {
        generateModelData(block->children.child[i], i, child);
    }
}

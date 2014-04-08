
#include <QtWidgets>
#include <QMessageBox>

#include "treeitem.h"
#include "treemodel.h"

#include <BDException.h>

TreeModel::TreeModel(const QStringList &headers, bd_templ_blob *templ_blob, const bd_item *root_item, QObject *parent)
    : QAbstractItemModel(parent), templ_blob(templ_blob), hdRootItem(root_item)
{
    QVector<QVariant> rootData;
    foreach (QString header, headers)
        rootData << header;

    rootItem = new TreeItem<UserData>(rootData);
    UserData userData;
    userData.index = (bd_u32)-1;
    userData.item = root_item;
    rootItem->setUserData(userData);
    generateModelData(hdRootItem, (bd_u32)-1, rootItem);
}

//! [1]
TreeModel::~TreeModel()
{
    delete rootItem;
}
//! [1]

//! [2]
int TreeModel::columnCount(const QModelIndex & /* parent */) const
{
    return rootItem->columnCount();
}
//! [2]

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    TreeItem<UserData> *item = getItem(index);

    return item->data(index.column());
}

//! [3]
Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}
//! [3]

//! [4]
TreeItem<TreeModel::UserData> *TreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid())
    {
        TreeItem<UserData> *item = static_cast<TreeItem<UserData>*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}
//! [4]

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

//! [5]
QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();
//! [5]

//! [6]
    TreeItem<UserData> *parentItem = getItem(parent);

    TreeItem<UserData> *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}
//! [6]

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
    TreeItem<UserData> *parentItem = getItem(parent);
    bool success;

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, rootItem->columnCount());
    endInsertRows();

    return success;
}

//! [7]
QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem<UserData> *childItem = getItem(index);
    TreeItem<UserData> *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}
//! [7]

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
    TreeItem<UserData> *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

//! [8]
int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem<UserData> *parentItem = getItem(parent);

    return parentItem->childCount();
}
//! [8]

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    TreeItem<UserData> *item = getItem(index);
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

void TreeModel::generateModelData(const bd_item *item, bd_u32 index, TreeItem<UserData> *parent)
{
    if (item == 0)
    {
        QMessageBox::warning(0, tr("Parsed tree"), tr("Parsed tree is empty."), QMessageBox::Ok);
        return;
    }

    QString str;

    int parentChildrenCount = parent->childCount();
    parent->insertChildren(parentChildrenCount, 1, rootItem->columnCount());

    TreeItem<UserData> *child = parent->child(parentChildrenCount);
    UserData userData;
    userData.index = index;
    userData.item = item;
    child->setUserData(userData);

    if (item->is_array)
    {
        str.sprintf("%s %s[%d]", item->type_name, item->name, index == (bd_u32)-1 ? item->count : index);
    }
    else
    {
        str.sprintf("%s %s", item->type_name, item->name);
    }
    child->setData(0, str); // Name
    child->setData(2, str.sprintf("%lXh", item->offset));                  // Start
    child->setData(3, str.sprintf("%lXh", item->size));                    // Size

    if (item->is_array == BD_TRUE && index == (bd_u32)-1 && item->type != BD_IT_CHAR)
    {
        for (bd_u32 i = 0 ; i < item->count; ++i)
        {
            generateModelData(item, i, child);
        }
    }
    else // char array or array element
    {
        bd_u64 offset = item->offset + (index == (bd_u32)-1 ? 0 : index * item->elem_size);
        switch (item->type)
        {
        case BD_IT_CHAR:
        {
            char *data = new char[item->count + 1];
            if (!BD_SUCCEED(templ_blob->get_datap(templ_blob, item->offset, item->size, data)))
            {
                throw BDException(tr("Could not read single CHAR or CHAR array"));
            }
            data[item->count] = '\0';
            str = QString("\"%1\"").arg(data);
            break;
        }
        case BD_IT_UCHAR:
            {
                unsigned char data;
                if (!BD_SUCCEED(templ_blob->get_datap(templ_blob, offset, item->elem_size, &data)))
                {
                    throw BDException(tr("Could not read UCHAR"));
                }
    //            str.sprintf("%uc", data);
                str = QString("%1").arg(data);
            }
            break;
        case BD_IT_WORD:
            {
                WORD_T data;
                if (!BD_SUCCEED(templ_blob->get_datap(templ_blob, offset, item->elem_size, &data)))
                {
                    throw BDException(tr("Could not read WORD"));
                }
    //            str.sprintf("%i", data);
                str = QString("%1").arg(data);
            }
            break;
        case BD_IT_DWORD:
            {
                DWORD_T data;
                if (!BD_SUCCEED(templ_blob->get_datap(templ_blob, offset, item->elem_size, &data)))
                {
                    throw BDException(tr("Could not read DWORD"));
                }
    //            str.sprintf("%i", data);
                str = QString("%1").arg(data);
            }
            break;
        case BD_IT_QWORD:
            {
                QWORD_T data;
                if (!BD_SUCCEED(templ_blob->get_datap(templ_blob, offset, item->elem_size, &data)))
                {
                    throw BDException(tr("Could not read QWORD"));
                }
    //            str.sprintf("%ll", data);
                str = QString("%1").arg(data);
            }
            break;
        case BD_IT_DOUBLE:
            {
                DOUBLE_T data;
                if (!BD_SUCCEED(templ_blob->get_datap(templ_blob, offset, item->elem_size, &data)))
                {
                    throw BDException(tr("Could not read DOUBLE"));
                }
    //            str.sprintf("%ll", data);
                str = QString("%1").arg(data);
            }
            break;
        default:
    //        str = "<stub>";
            break;
        }
    }

    child->setData(1, str); // Value

    for (bd_u32 i = 0; i < item->children.count; ++i)
    {
        generateModelData(item->children.child[i], (bd_u32)-1, child);
    }
}

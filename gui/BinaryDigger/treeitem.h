
#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QVariant>
#include <QVector>

template <class T>
class TreeItem
{
public:
    explicit TreeItem(const QVector<QVariant> &data, TreeItem *parent = 0);
    ~TreeItem();

    TreeItem *child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    bool insertChildren(int position, int count, int columns);
    bool insertColumns(int position, int columns);
    TreeItem *parent();
    bool removeChildren(int position, int count);
    bool removeColumns(int position, int columns);
    int childNumber() const;
    bool setData(int column, const QVariant &value);

    void setUserData(const T& udata) { userData = udata; }

    const T& getUserData() { return userData; }

private:
    QList<TreeItem*> childItems;
    QVector<QVariant> itemData;
    TreeItem *parentItem;
    T userData;
};

template<class T>
TreeItem<T>::TreeItem(const QVector<QVariant> &data, TreeItem<T> *parent)
{
    parentItem = parent;
    itemData = data;
}

template<class T>
TreeItem<T>::~TreeItem()
{
    qDeleteAll(childItems);
}

template<class T>
TreeItem<T> *TreeItem<T>::child(int number)
{
    return childItems.value(number);
}

template<class T>
int TreeItem<T>::childCount() const
{
    return childItems.count();
}

template<class T>
int TreeItem<T>::childNumber() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<TreeItem<T>*>(this));

    return 0;
}

template<class T>
int TreeItem<T>::columnCount() const
{
    return itemData.count();
}

template<class T>
QVariant TreeItem<T>::data(int column) const
{
    return itemData.value(column);
}

template<class T>
bool TreeItem<T>::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > childItems.size())
        return false;

    for (int row = 0; row < count; ++row) {
        QVector<QVariant> data(columns);
        TreeItem<T> *item = new TreeItem<T>(data, this);
        childItems.insert(position, item);
    }

    return true;
}

template<class T>
bool TreeItem<T>::insertColumns(int position, int columns)
{
    if (position < 0 || position > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        itemData.insert(position, QVariant());

    foreach (TreeItem<T> *child, childItems)
        child->insertColumns(position, columns);

    return true;
}

template<class T>
TreeItem<T> *TreeItem<T>::parent()
{
    return parentItem;
}

template<class T>
bool TreeItem<T>::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete childItems.takeAt(position);

    return true;
}

template<class T>
bool TreeItem<T>::removeColumns(int position, int columns)
{
    if (position < 0 || position + columns > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        itemData.remove(position);

    foreach (TreeItem<T> *child, childItems)
        child->removeColumns(position, columns);

    return true;
}

template<class T>
bool TreeItem<T>::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= itemData.size())
        return false;

    itemData[column] = value;
    return true;
}

#endif // TREEITEM_H

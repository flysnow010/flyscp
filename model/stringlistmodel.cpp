#include "stringlistmodel.h"

StringListModel::StringListModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

StringListModel::StringListModel(const QStringList &strings, QObject *parent)
    : QAbstractListModel(parent), lst(strings)
{
}

int StringListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return lst.count();
}

QModelIndex StringListModel::sibling(int row, int column, const QModelIndex &idx) const
{
    if (!idx.isValid() || column != 0 || row >= lst.count() || row < 0)
        return QModelIndex();

    return createIndex(row, 0);
}

QVariant StringListModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= lst.size())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole)
        return lst.at(index.row());

    return QVariant();
}

Qt::ItemFlags StringListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractListModel::flags(index) | Qt::ItemIsDropEnabled;

    return QAbstractListModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

bool StringListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() >= 0 && index.row() < lst.size()
        && (role == Qt::EditRole || role == Qt::DisplayRole)) {
        const QString valueString = value.toString();
        if (lst.at(index.row()) == valueString)
            return true;
        lst.replace(index.row(), valueString);
        QVector<int> roles;
        roles.reserve(2);
        roles.append(Qt::DisplayRole);
        roles.append(Qt::EditRole);
        emit dataChanged(index, index, roles);
        // once Q_COMPILER_UNIFORM_INIT can be used, change to:
        // emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
        return true;
    }
    return false;
}

bool StringListModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (count < 1 || row < 0 || row > rowCount(parent))
        return false;

    beginInsertRows(QModelIndex(), row, row + count - 1);

    for (int r = 0; r < count; ++r)
        lst.insert(row, QString());

    endInsertRows();

    return true;
}

bool StringListModel::insertRow(int row, QString const& text)
{
    beginInsertRows(QModelIndex(), row, row);

    lst.insert(row, text);
    endInsertRows();

    return true;
}

bool StringListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
        return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);

    const auto it = lst.begin() + row;
    lst.erase(it, it + count);

    endRemoveRows();

    return true;
}

static bool ascendingLessThan(const QPair<QString, int> &s1, const QPair<QString, int> &s2)
{
    return s1.first < s2.first;
}

static bool decendingLessThan(const QPair<QString, int> &s1, const QPair<QString, int> &s2)
{
    return s1.first > s2.first;
}

void StringListModel::sort(int, Qt::SortOrder order)
{
    emit layoutAboutToBeChanged(QList<QPersistentModelIndex>(), VerticalSortHint);

    QVector<QPair<QString, int> > list;
    const int lstCount = lst.count();
    list.reserve(lstCount);
    for (int i = 0; i < lstCount; ++i)
        list.append(QPair<QString, int>(lst.at(i), i));

    if (order == Qt::AscendingOrder)
        std::sort(list.begin(), list.end(), ascendingLessThan);
    else
        std::sort(list.begin(), list.end(), decendingLessThan);

    lst.clear();
    QVector<int> forwarding(lstCount);
    for (int i = 0; i < lstCount; ++i) {
        lst.append(list.at(i).first);
        forwarding[list.at(i).second] = i;
    }

    QModelIndexList oldList = persistentIndexList();
    QModelIndexList newList;
    const int numOldIndexes = oldList.count();
    newList.reserve(numOldIndexes);
    for (int i = 0; i < numOldIndexes; ++i)
        newList.append(index(forwarding.at(oldList.at(i).row()), 0));
    changePersistentIndexList(oldList, newList);

    emit layoutChanged(QList<QPersistentModelIndex>(), VerticalSortHint);
}

QStringList StringListModel::stringList() const
{
    return lst;
}

void StringListModel::setStringList(const QStringList &strings)
{
    beginResetModel();
    lst = strings;
    endResetModel();
}

Qt::DropActions StringListModel::supportedDropActions() const
{
    return QAbstractItemModel::supportedDropActions() | Qt::MoveAction;
}


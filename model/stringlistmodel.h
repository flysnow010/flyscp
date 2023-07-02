#ifndef STRINGLISTMODEL_H
#define STRINGLISTMODEL_H

#include <QAbstractListModel>
#include <QStringList>

class StringListModel : public QAbstractListModel
{
public:
    explicit StringListModel(QObject *parent = nullptr);
    explicit StringListModel(const QStringList &strings,
                             QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex sibling(int row, int column,
                        const QModelIndex &idx) const override;

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index,
                 const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool insertRows(int row,
                    int count,
                    const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row,
                    int count,
                    const QModelIndex &parent = QModelIndex()) override;
    bool insertRow(int row,
                   QString const& text);
    bool appendRows(int row,
                    QStringList const& texts);

    void sort(int column,
              Qt::SortOrder order = Qt::AscendingOrder) override;

    QStringList stringList() const;
    void setStringList(const QStringList &strings);

    Qt::DropActions supportedDropActions() const override;
private:
    QStringList lst;
};

#endif // STRINGLISTMODEL_H

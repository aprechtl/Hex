#ifndef LINEEDITDELEGATE_H
#define LINEEDITDELEGATE_H
#include <QtWidgets/QStyledItemDelegate>


class LineEditDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    LineEditDelegate(QObject *parent = 0);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const;

signals:
    void editingFinished() const;
};

#endif // LINEEDITDELEGATE_H

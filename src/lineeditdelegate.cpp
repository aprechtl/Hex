#include "lineeditdelegate.h"
#include <QtWidgets/QLineEdit>

LineEditDelegate::LineEditDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{}

QWidget *LineEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
    return new QLineEdit(parent);
}

void LineEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    static_cast<QLineEdit*>(editor)->setText(index.model()->data(index, Qt::EditRole).toString());
}

void LineEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    model->setData(index, static_cast<QLineEdit*>(editor)->text(), Qt::EditRole);
    emit editingFinished();
}

void LineEditDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}

#ifndef COMPLETERVIEWDELEGATE_H
#define COMPLETERVIEWDELEGATE_H

#include "dfmplugin_titlebar_global.h"

#include <QStyledItemDelegate>

DPTITLEBAR_BEGIN_NAMESPACE

class CompleterViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CompleterViewDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

DPTITLEBAR_END_NAMESPACE

#endif   // COMPLETERVIEWDELEGATE_H

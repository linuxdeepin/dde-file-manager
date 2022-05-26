#include "completerviewdelegate.h"

#include <QPainter>

DPTITLEBAR_USE_NAMESPACE

CompleterViewDelegate::CompleterViewDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void CompleterViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // prepare
    QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled)
            ? QPalette::Normal
            : QPalette::Disabled;
    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active)) {
        cg = QPalette::Inactive;
    }

    // draw background
    if (option.showDecorationSelected && (option.state & (QStyle::State_Selected | QStyle::State_MouseOver))) {
        painter->fillRect(option.rect, option.palette.brush(cg, QPalette::Highlight));
    }

    // draw text
    if (option.state & (QStyle::State_Selected | QStyle::State_MouseOver)) {
        painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(cg, QPalette::Text));
    }

    painter->setFont(option.font);
    auto text = index.data(Qt::DisplayRole).toString();
    if (text.contains('\n'))
        text = text.replace('\n', ' ');
    painter->drawText(option.rect.adjusted(31, 0, 0, 0), Qt::AlignVCenter, text);
}

QSize CompleterViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize s = QStyledItemDelegate::sizeHint(option, index);
    s.setHeight(24);

    return s;
}

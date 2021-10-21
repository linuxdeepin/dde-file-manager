#ifndef ITEMDELEGATEHELPER_H
#define ITEMDELEGATEHELPER_H

#include "dfm-base/dfm_base_global.h"

#include <QStyledItemDelegate>
#include <QTextLayout>

// begin file view item icon delegate global define
static const int  ICON_MODE_TEXT_PADDING = 4;
static const int  ICON_MODE_ICON_SPACING = 5;
static const int  ICON_MODE_COLUMU_PADDING = 10;
static const int  ICON_MODE_RECT_RADIUS = ICON_MODE_TEXT_PADDING;
static const int  ICON_MODE_BACK_RADIUS = 18;
// end
// begin file view item list delegate global define
static const int  LIST_MODE_LEFT_MARGIN = 10;
static const int  LIST_MODE_RIGHT_MARGIN = 10;
static const int  LIST_MODE_RECT_RADIUS = 8;
static const int  LIST_MODE_LEFT_PADDING = 10;
static const int  LIST_MODE_ICON_SPACING = 6;
static const int  LIST_MODE_RIGHT_PADDING = 10;
static const int  LIST_MODE_COLUMU_PADDING = 10;
// end

DFMBASE_BEGIN_NAMESPACE
class ItemDelegateHelper
{
public:
    static inline Qt::Alignment visualAlignment(Qt::LayoutDirection direction, Qt::Alignment alignment)
    {
        if (!(alignment & Qt::AlignHorizontal_Mask))
            alignment |= Qt::AlignLeft;
        if (!(alignment & Qt::AlignAbsolute) && (alignment & (Qt::AlignLeft | Qt::AlignRight))) {
            if (direction == Qt::RightToLeft)
                alignment ^= (Qt::AlignLeft | Qt::AlignRight);
            alignment |= Qt::AlignAbsolute;
        }
        return alignment;
    }
    static QPixmap getIconPixmap(const QIcon &icon, const QSize &size, qreal pixelRatio,
                                 QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);
    static void paintIcon(QPainter *painter, const QIcon &icon, const QRectF &rect,
                          Qt::Alignment alignment = Qt::AlignCenter,
                          QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);
};
DFMBASE_END_NAMESPACE

#endif // ITEMDELEGATEHELPER_H

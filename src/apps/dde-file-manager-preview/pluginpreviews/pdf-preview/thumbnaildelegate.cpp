// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thumbnaildelegate.h"
#include "sidebarimageviewmodel.h"

#include <DGuiApplicationHelper>

#include <QApplication>
#include <QPainter>
#include <QItemSelectionModel>
#include <QAbstractItemView>
#include <QPainterPath>

DGUI_USE_NAMESPACE
using namespace plugin_filepreview;
ThumbnailDelegate::ThumbnailDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{
    itemViewParent = parent;
}

void ThumbnailDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid()) {
        qreal pixscale = itemViewParent->property("adaptScale").toDouble();

        int rotate = index.data(ImageinfoType_e::IMAGE_ROTATE).toInt();

        QTransform transform; // 使用QTransform替代QMatrix
        transform.rotate(rotate);

        const QPixmap &pixmap = index.data(ImageinfoType_e::IMAGE_PIXMAP).value<QPixmap>().transformed(transform); // 使用QTransform进行变换

        const int borderRadius = 6;

        QSize pageSize = index.data(ImageinfoType_e::IMAGE_PAGE_SIZE).toSize();

        if (rotate == 90 || rotate == 270)
            pageSize = QSize(pageSize.height(), pageSize.width());

        pageSize.scale(static_cast<int>(200 * pixscale * qApp->devicePixelRatio()), static_cast<int>(200 * pixscale * qApp->devicePixelRatio()), Qt::KeepAspectRatio);

        const QSize &scalePixSize = pageSize / qApp->devicePixelRatio();

        const QRect &rect = QRect(option.rect.center().x() - scalePixSize.width() / 2, option.rect.center().y() - scalePixSize.height() / 2, scalePixSize.width(), scalePixSize.height());

        if (!pixmap.isNull()) {
            //! clipPath pixmap
            painter->save();
            QPainterPath clipPath;
            clipPath.addRoundedRect(rect, borderRadius, borderRadius);
            painter->setClipPath(clipPath);
            painter->drawPixmap(rect.x(), rect.y(), rect.width(), rect.height(), pixmap);
            painter->restore();
        }

        //! drawText RoundRect
        painter->save();
        painter->setBrush(Qt::NoBrush);
        if (itemViewParent->selectionModel()->isRowSelected(index.row(), index.parent())) {
            painter->setPen(QPen(DTK_NAMESPACE::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color(), 2));
            painter->drawRoundedRect(rect, borderRadius, borderRadius);
        } else {
            painter->setPen(QPen(DTK_NAMESPACE::Gui::DGuiApplicationHelper::instance()->applicationPalette().frameShadowBorder().color(), 1));
            painter->drawRoundedRect(rect, borderRadius, borderRadius);
            painter->setPen(QPen(DTK_NAMESPACE::Gui::DGuiApplicationHelper::instance()->applicationPalette().windowText().color()));
        }
        painter->restore();
    }
}

QSize ThumbnailDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return DStyledItemDelegate::sizeHint(option, index);
}

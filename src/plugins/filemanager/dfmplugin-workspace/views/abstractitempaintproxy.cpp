// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractitempaintproxy.h"
#include "fileview.h"
#include "models/fileviewmodel.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/iconutils.h>

using namespace dfmplugin_workspace;
DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

AbstractItemPaintProxy::AbstractItemPaintProxy(QObject *parent)
    : QObject(parent)
{
}

void AbstractItemPaintProxy::drawIcon(QPainter *painter, QRectF *rect, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(painter)
    Q_UNUSED(rect)
    Q_UNUSED(option)
    Q_UNUSED(index)
}

void AbstractItemPaintProxy::drawBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(index)
}

void AbstractItemPaintProxy::drawText(QPainter *painter, QRectF *rect, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(painter)
    Q_UNUSED(rect)
    Q_UNUSED(option)
    Q_UNUSED(index)
}

QRectF AbstractItemPaintProxy::rectByType(RectOfItemType type, const QModelIndex &index)
{
    Q_UNUSED(type)
    Q_UNUSED(index)

    return QRectF();
}

QList<QRect> AbstractItemPaintProxy::allPaintRect(const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    return QList<QRect>();
}

int AbstractItemPaintProxy::iconRectIndex()
{
    return 0;
}

bool AbstractItemPaintProxy::supportContentPreview() const
{
    return false;
}

void AbstractItemPaintProxy::setStyleProxy(QStyle *style)
{
    this->style = style;
}

bool AbstractItemPaintProxy::isThumnailIconIndex(const QModelIndex &index) const
{
    auto parent = dynamic_cast<FileView *>(this->parent());
    if (!index.isValid() || !parent)
        return false;

    FileInfoPointer info { parent->model()->fileInfo(index) };
    if (info) {
        if (IconUtils::shouldSkipThumbnailFrame(info->nameOf(NameInfoType::kMimeTypeName)))
            return false;

        const auto &attribute { info->extendAttributes(ExtInfoType::kFileThumbnail) };
        if (attribute.isValid() && !attribute.value<QIcon>().isNull())
            return true;
    }
    return false;
}

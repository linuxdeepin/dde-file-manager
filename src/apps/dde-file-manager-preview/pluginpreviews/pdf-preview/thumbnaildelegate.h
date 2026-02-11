// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUMBNAILDELEGATE_H
#define THUMBNAILDELEGATE_H

#include "preview_plugin_global.h"

#include <DStyledItemDelegate>

namespace plugin_filepreview {
class ThumbnailDelegate : public DTK_WIDGET_NAMESPACE::DStyledItemDelegate
{
public:
    explicit ThumbnailDelegate(QAbstractItemView *parent = nullptr);

protected:
    /**
     * @brief paint
     * 绘制事件
     * @param painter
     * @param option
     * @param index
     */
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    /**
     * @brief sizeHint
     * 节点大小
     * @param option
     * @param index
     * @return
     */
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QAbstractItemView *itemViewParent { nullptr };
};
}
#endif   // THUMBNAILDELEGATE_H

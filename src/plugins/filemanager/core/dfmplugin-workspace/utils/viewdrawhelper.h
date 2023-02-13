// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIEWDRAWHELPER_H
#define VIEWDRAWHELPER_H

#include "dfmplugin_workspace_global.h"

#include "dfm-base/dfm_global_defines.h"

#include <QObject>
#include <QPixmap>
#include <QModelIndexList>
#include <QStyleOptionViewItem>

namespace GlobalPrivate {
inline constexpr int kDragIconMaxCount { 99 };
inline constexpr int kDragIconSize { 128 };
inline constexpr int kDragIconOutline { 30 };
inline constexpr int kDragIconMax { 4 };
inline constexpr int kListDragIconSize { 120 };
inline constexpr int kListDragTextWidth { 116 };
inline constexpr int kListDragTextHeight { 47 };
inline constexpr qreal kDragIconRotate { 10.0 };
inline constexpr qreal kDragIconOpacity { 0.1 };
}

namespace dfmplugin_workspace {

class FileView;
class FileViewModel;
class ViewDrawHelper : public QObject
{
    Q_OBJECT
public:
    explicit ViewDrawHelper(FileView *parent);

    QPixmap renderDragPixmap(DFMGLOBAL_NAMESPACE::ViewMode mode, QModelIndexList indexes) const;

private:
    void drawDragIcons(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QModelIndexList &indexes, const QModelIndex &topIndex) const;
    void drawDragCount(QPainter *painter, const QModelIndex &topIndex, const QStyleOptionViewItem &option, int count) const;
    void drawDragText(QPainter *painter, const QModelIndex &index) const;

    FileView *view { nullptr };
};

}

#endif   // VIEWDRAWHELPER_H

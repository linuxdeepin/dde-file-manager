// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STICKYGROUPHEADERHELPER_H
#define STICKYGROUPHEADERHELPER_H

#include "dfmplugin_workspace_global.h"

#include <QObject>
#include <QModelIndex>
#include <QPoint>

namespace dfmplugin_workspace {

class FileView;

// Sticky group header overlay: paints the current group header pinned at
// the top of the viewport when the real header scrolls out of view.
// State (currentStickyIndex/Rect/hovered/cachedNext) lives in FileViewPrivate
// (shared with FileView's event handlers); this helper accesses it via friend.
class StickyGroupHeaderHelper : public QObject
{
    Q_OBJECT
public:
    explicit StickyGroupHeaderHelper(FileView *parent = nullptr);

    int stickyHeaderHeight() const;
    QModelIndex findStickyGroupIndex(int headerHeight) const;
    int computeStickyY(int headerHeight) const;
    void paintStickyHeaderOverlay(const QModelIndex &index, int y, int headerHeight);
    bool isPosInStickyHeader(const QPoint &pos) const;
    void clearStickyHeaderState();
    void scrollStickyHeaderToTop(const QModelIndex &headerIndex);
    int groupHeaderContentTop(const QModelIndex &index) const;

private:
    FileView *view { nullptr };
};

}   // namespace dfmplugin_workspace

#endif   // STICKYGROUPHEADERHELPER_H

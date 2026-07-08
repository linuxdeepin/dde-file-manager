// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STICKYGROUPHEADERHELPER_H
#define STICKYGROUPHEADERHELPER_H

#include "dfmplugin_workspace_global.h"

#include <QObject>
#include <QModelIndex>
#include <QPoint>
#include <QRect>

namespace dfmplugin_workspace {

class FileView;

// Sticky group header overlay: paints the current group header pinned at
// the top of the viewport when the real header scrolls out of view.
// State ownership: currentStickyIndex/Rect/hovered/cachedNext live in this
// helper as private members; FileView reads/writes via public accessors
// instead of touching FileViewPrivate directly.
class StickyGroupHeaderHelper : public QObject
{
    Q_OBJECT
public:
    explicit StickyGroupHeaderHelper(FileView *parent = nullptr);

    int stickyHeaderHeight() const;
    QModelIndex findStickyGroupIndex(int headerHeight);
    int computeStickyY(int headerHeight) const;
    void paintStickyHeaderOverlay(const QModelIndex &index, int y, int headerHeight);
    bool isPosInStickyHeader(const QPoint &pos) const;
    void clearStickyHeaderState();
    void scrollStickyHeaderToTop(const QModelIndex &headerIndex);
    int groupHeaderContentTop(const QModelIndex &index) const;

    // State accessors — state is owned by this helper.
    QModelIndex currentStickyIndex() const { return m_currentStickyIndex; }
    void setCurrentStickyIndex(const QModelIndex &index) { m_currentStickyIndex = index; }

    QRect currentStickyRect() const { return m_currentStickyRect; }
    void setCurrentStickyRect(const QRect &rect) { m_currentStickyRect = rect; }

    bool isStickyHeaderHovered() const { return m_stickyHeaderHovered; }
    void setStickyHeaderHovered(bool hovered) { m_stickyHeaderHovered = hovered; }

    QModelIndex cachedNextStickyHeader() const { return m_cachedNextStickyHeader; }
    void setCachedNextStickyHeader(const QModelIndex &index) { m_cachedNextStickyHeader = index; }

private:
    FileView *m_view { nullptr };

    // Sticky overlay state — owned by this helper (was in FileViewPrivate).
    // cachedNextStickyHeader_ is mutable: findStickyGroupIndex() (const) updates it as a cache.
    QModelIndex m_currentStickyIndex;
    QRect m_currentStickyRect;
    bool m_stickyHeaderHovered { false };
    mutable QModelIndex m_cachedNextStickyHeader;
};

}   // namespace dfmplugin_workspace

#endif   // STICKYGROUPHEADERHELPER_H

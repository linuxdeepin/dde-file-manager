// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGEVENTCALLER_H
#define TAGEVENTCALLER_H

#include "dfmplugin_tag_global.h"

#include <dfm-base/dfm_global_defines.h>

class QAbstractItemView;

namespace dfmplugin_tag {

class TagEventCaller
{
    TagEventCaller() = delete;

public:
    static void sendOpenWindow(const QUrl &url);
    static void sendOpenTab(quint64 windowId, const QUrl &url);
    static void sendOpenFiles(const quint64 windowID, const QList<QUrl> &urls);
    static void sendFileUpdate(const QString &path);
    static bool sendCheckTabAddable(quint64 windowId);

    static QRectF getVisibleGeometry(const quint64 windowID);
    static QRectF getItemRect(const quint64 windowID, const QUrl &url, const DFMGLOBAL_NAMESPACE::ItemRoles role);

    static QAbstractItemView *getDesktopView(int viewIdx);
    static QAbstractItemView *getCollectionView(const QString &id);
    static int getDesktopViewIndex(const QString &url, QPoint *pos);
    static QString getCollectionViewId(const QUrl &url, QPoint *pos);
    static QRect getVisualRect(int viewIndex, const QUrl &url);
    static QRect getCollectionVisualRect(const QString &id, const QUrl &url);
    static QRect getIconRect(int viewIndex, QRect visualRect);
    static QRect getCollectionIconRect(const QString &id, QRect visualRect);
};

}

#endif   // TAGEVENTCALLER_H

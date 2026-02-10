// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOOKMARK_CALLBACK_H
#define BOOKMARK_CALLBACK_H

#include <dfm-mount/base/dmount_global.h>

#include <QObject>

namespace BookmarkCallBack {
void contextMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos);
void renameCallBack(quint64 windowId, const QUrl &url, const QString &name);
void cdBookMarkUrlCallBack(quint64 windowId, const QUrl &url);
void cdDefaultItemUrlCallBack(quint64 windowId, const QUrl &url);
}   // namespace BookmarkCallBack

#endif   // BOOKMARK_CALLBACK_H

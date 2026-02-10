// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAREEVENTHELPER_H
#define SHAREEVENTHELPER_H

#include "dfmplugin_myshares_global.h"

#include <QUrl>
#include <QList>
#include <QObject>

namespace dfmplugin_myshares {

class ShareEventHelper : public QObject
{
public:
    static ShareEventHelper *instance();
    bool blockPaste(quint64 winId, const QList<QUrl> &fromUrls, const QUrl &to);
    bool blockDelete(quint64 winId, const QList<QUrl> &urls, const QUrl &);
    bool blockMoveToTrash(quint64 winId, const QList<QUrl> &urls, const QUrl&);
    bool hookSendOpenWindow(const QList<QUrl> &urls);
    bool hookSendChangeCurrentUrl(quint64 winId, const QUrl &url);

protected:
    explicit ShareEventHelper(QObject *parent = nullptr);
    bool containsShareUrl(const QList<QUrl> &urls);
};

}

#endif   // SHAREEVENTHELPER_H

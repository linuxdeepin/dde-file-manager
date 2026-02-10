// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAREUTILS_H
#define SHAREUTILS_H

#include "dfmplugin_myshares_global.h"

#include <QString>
#include <QIcon>
#include <QUrl>

namespace dfmplugin_myshares {

class ShareUtils : public QObject
{
    Q_DISABLE_COPY(ShareUtils)
public:
    static ShareUtils *instance();

    static QString scheme();
    static QIcon icon();
    static QString displayName();
    static QUrl rootUrl();
    static QUrl makeShareUrl(const QString &path);
    static QUrl convertToLocalUrl(const QUrl &shareUrl);

private:
    explicit ShareUtils(QObject *parent = nullptr);
};

}

#endif   // SHAREUTILS_H

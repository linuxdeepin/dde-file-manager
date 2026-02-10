// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOOKMARKHELPER_H
#define BOOKMARKHELPER_H

#include "dfmplugin_bookmark_global.h"

#include <QObject>

namespace dfmplugin_bookmark {

class BookMarkHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(BookMarkHelper)

public:
    static BookMarkHelper *instance();
    QString scheme();
    QUrl rootUrl();
    QIcon icon();
    bool isValidQuickAccessConf(const QVariantList &list);

private:
    explicit BookMarkHelper(QObject *parent = nullptr);
};

}

#endif   // BOOKMARKHELPER_H

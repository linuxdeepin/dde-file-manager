// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMMANAGER_H
#define CUSTOMMANAGER_H

#include "dfmplugin_search_global.h"
#include <QVariantMap>

namespace dfmplugin_search {

class CustomManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CustomManager)
public:
    static CustomManager *instance();

    bool registerCustomInfo(const QString &scheme, const QVariantMap &properties);
    bool isRegisted(const QString &scheme) const;
    bool isDisableSearch(const QUrl &url);
    QString redirectedPath(const QUrl &url);
    bool isUseNormalMenu(const QString &scheme);

private:
    explicit CustomManager();

    using CustomInfos = QMap<QString, QVariantMap>;
    CustomInfos customInfos;
};

}

#endif   // CUSTOMMANAGER_H

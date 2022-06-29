/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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

private:
    explicit CustomManager();

    using CustomInfos = QMap<QString, QVariantMap>;
    CustomInfos customInfos;
};

}

#endif   // CUSTOMMANAGER_H

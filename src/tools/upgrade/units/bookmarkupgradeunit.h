/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhuangshu<zhuangshu@uniontech.com>
 *
 * Maintainer: zhuangshu<zhuangshu@uniontech.com>
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
#ifndef BOOKMARKUPGRADEUNIT_H
#define BOOKMARKUPGRADEUNIT_H

#include "core/upgradeunit.h"

#include <QElapsedTimer>
#include <QUrl>
#include <QDateTime>
#include <QVariantMap>
#include <QJsonObject>

namespace dfm_upgrade {
struct BookmarkData
{
    QDateTime created;
    QDateTime lastModified;
    QString locateUrl;
    QString deviceUrl;
    QString name;
    QString transName;
    QUrl url;
    bool isDefaultItem = false;
    int index = -1;

    QString udisksDBusPath;
    QString udisksMountPoint;
    void resetData(const QVariantMap &map);
    QVariantMap serialize();
};

class BookMarkUpgradeUnit : public UpgradeUnit
{
public:
    BookMarkUpgradeUnit();
    QString name() override;
    bool initialize(const QMap<QString, QString> &args) override;
    bool upgrade() override;
    void completed() override;

private:
    QVariantList initData() const;
    bool doUpgrade(const QVariantList &quickAccessDatas);

private:
    QMap<QUrl, BookmarkData> quickAccessDataMap {};
    QMap<QUrl, BookmarkData> bookmarkDataMap {};
    QJsonObject configObject;
};

}

#endif   // BOOKMARKUPGRADEUNIT_H

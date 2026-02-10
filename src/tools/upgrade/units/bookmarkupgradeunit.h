// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

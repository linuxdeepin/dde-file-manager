// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UPGRADEFACTORY_H
#define UPGRADEFACTORY_H

#include <QObject>

namespace dfm_upgrade {
class UpgradeUnit;
class UpgradeFactory
{
public:
    UpgradeFactory();
    void previous(const QMap<QString, QString> &args);
    void doUpgrade();
    void completed();
    bool isChanged() const;
private:
    QList<QSharedPointer<UpgradeUnit>> units;
};

}

#endif // UPGRADEFACTORY_H

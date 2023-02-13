// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFAULTITEMMANAGER_H
#define DEFAULTITEMMANAGER_H

#include "units/bookmarkupgradeunit.h"
#include <QObject>
namespace dfm_upgrade {

class DefaultItemManagerPrivate;
class DefaultItemManager : public QObject
{
    Q_OBJECT
    friend class DefaultItemManagerPrivate;
    DefaultItemManagerPrivate *const d = nullptr;

public:
    static DefaultItemManager *instance();
    void initDefaultItems();
    QList<BookmarkData> defaultItemInitOrder();

private:
    explicit DefaultItemManager(QObject *parent = nullptr);
};

}

#endif   // DEFAULTITEMMANAGER_H

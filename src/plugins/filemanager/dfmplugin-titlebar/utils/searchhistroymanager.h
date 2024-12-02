// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHHISTROYMANAGER_H
#define SEARCHHISTROYMANAGER_H

#include "dfmplugin_titlebar_global.h"

#include <QObject>

namespace dfmplugin_titlebar {

class SearchHistroyManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SearchHistroyManager)

public:
    static SearchHistroyManager *instance();

    QStringList getSearchHistroy();
    QList<IPHistroyData> getIPHistory();
    void writeIntoSearchHistory(QString keyword);
    void writeIntoIPHistory(const QString &ipAddr);
    bool removeSearchHistory(QString keyword);
    void clearHistory(const QStringList &schemeFilters = QStringList());

private:
    explicit SearchHistroyManager(QObject *parent = nullptr);
};

}

#endif   // SEARCHHISTROYMANAGER_H

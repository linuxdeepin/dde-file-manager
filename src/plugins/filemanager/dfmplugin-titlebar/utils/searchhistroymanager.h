// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHHISTROYMANAGER_H
#define SEARCHHISTROYMANAGER_H

#include "dfmplugin_titlebar_global.h"

#include <dfm-mount/base/dmount_global.h>

#include <QObject>
#include <QRegularExpression>

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
    void addIPHistoryCache(const QString &address);
    bool removeSearchHistory(QString keyword);
    void clearHistory(const QStringList &schemeFilters = QStringList());
    void clearIPHistory();

private:
    explicit SearchHistroyManager(QObject *parent = nullptr);
    void handleMountNetworkResult(const QString &address, bool ret, DFMMOUNT::DeviceError err, const QString &msg);
    bool isValidMount(const QString &address, bool ret, dfmmount::DeviceError err);
    void writeIntoIPHistory(const QString &ipAddr);

    QRegularExpression protocolIPRegExp;   // smb://ip, ftp://ip, sftp://ip
    QStringList ipAddressCache;
};

}

#endif   // SEARCHHISTROYMANAGER_H

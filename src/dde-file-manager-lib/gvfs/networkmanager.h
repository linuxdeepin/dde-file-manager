/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QSize>
#include <QDebug>
#include "debugobejct.h"
#include "dfmevent.h"

#undef signals
extern "C" {
    #include <gio/gio.h>
}
#define signals public

#include <QMutex>
#include <QWaitCondition>

class NetworkNode
{

public:
    explicit NetworkNode();
    ~NetworkNode();

    QString url() const;
    void setUrl(const QString &url);

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    QString iconType() const;
    void setIconType(const QString &iconType);

signals:

public slots:

private:
    QString m_url;
    QString m_displayName;
    QString m_iconType;


    friend QDebug operator<<(QDebug dbg, const NetworkNode &node);
};


QDebug operator<<(QDebug dbg, const NetworkNode &node);


typedef QList<NetworkNode> NetworkNodeList;

class NetworkManager : public QObject
{
    Q_OBJECT

public:


    explicit NetworkManager(QObject *parent = 0);
    ~NetworkManager();

    void initData();
    void initConnect();

    static QStringList SupportScheme;
    static QMap<DUrl, NetworkNodeList> NetworkNodes;
    static GCancellable * m_networks_fetching_cancellable;
    static bool fetch_networks(gchar* url, DFMEvent* e);
    static void network_enumeration_finished (GObject      *source_object,
                                  GAsyncResult *res,
                                  gpointer      user_data);

    static void network_enumeration_next_files_finished (GObject      *source_object,
                                             GAsyncResult *res,
                                             gpointer      user_data);

    static void populate_networks (GFileEnumerator *enumerator, GList *detected_networks, gpointer user_data);
    static void restartGVFSD();

signals:

public slots:
    void fetchNetworks(const DFMUrlBaseEvent &event);
    static void cancelFeatchNetworks();

private:
    static QPointer<QEventLoop> eventLoop;
};

#endif // NETWORKMANAGER_H
